#include <stdlib.h>
#include <string.h>
#include "compiler_engine.h"
#include "tokenizer.h"
#include "output_writer.h"
#include "err_handler.h"

/*****************************************************************************/
/* PRIVATE FUNCTIONS */
/*****************************************************************************/
int consume_token_helper(compEng *eng, bool condition, TokenType tokType)
{
    Tokenizer *t = eng->tknzr;

    if (condition && t->currTok.type == tokType) {
        if (tknzr_has_more_tokens(t)) {
            tknzr_advance(t);
        }
    }
    else {
        return -EINVAL;
    }

    return 0;
}

int consume_keyword(compEng *eng, Keyword keyword)
{
    Tokenizer *t         = eng->tknzr;
    bool       condition = t->currTok.keyword == keyword;

    return consume_token_helper(eng, condition, TOK_TYPE_KEYWORD);
}

int consume_identifier(compEng *eng)
{
    Tokenizer *t         = eng->tknzr;
    bool       condition = true; // Always true, with identifiers we don't
                                 // need to check for a verbatim match against
                                 // a known string

    return consume_token_helper(eng, condition, TOK_TYPE_IDENTIFIER);
}

int consume_symbol(compEng *eng, char sym)
{
    Tokenizer *t = eng->tknzr;
    char tmpChar[2]; // 1 for char, plus extra for null terminator
    bool condition;

    tknzr_get_string_val(t, tmpChar, sizeof(tmpChar));
    condition = (sym == tmpChar[0]);

    return consume_token_helper(eng, condition, TOK_TYPE_SYMBOL);
}

int consume_str_literal(compEng *eng, const char* s)
{
    Tokenizer *t = eng->tknzr;
    char strBuf[MAX_IDENTIFIER_STR_LEN];
    bool condition;

    tknzr_get_string_val(t, strBuf, sizeof(strBuf));

    condition = (strncmp(strBuf, s, sizeof(strBuf)) == 0);

    return consume_token_helper(eng, condition, TOK_TYPE_STRING_CONST);
}

Keyword consume_keyword_if_found(compEng *eng, Keyword *arr, uint8_t arrSize)
{
    for (uint8_t i = 0; i < arrSize; i++) {
        if (consume_keyword(eng, arr[i]) == 0) {
            return arr[i];
        }
    }
    return KW_INVALID;
}

bool is_statement_keyword(Keyword kw)
{
    const Keyword statementKeywords[] = {
        KW_LET, KW_DO, KW_IF, KW_WHILE, KW_RETURN
    };

    for (uint8_t i = 0; i < ARR_SIZE(statementKeywords); i++) {
        if (kw == statementKeywords[i]) {
            return true;
        }
    }

    return false;
}

/*****************************************************************************/
/* PUBLIC FUNCTIONS */
/*****************************************************************************/
int compEng_new(compEng *eng, Tokenizer *t)
{
    eng->tknzr = t;
    eng->recurseLevel = 0;
    return 0;
}

void compEng_close(compEng *eng)
{
}

// Rule:
// 'class' className '{' classVarDec* subroutineDec* '}'
int compEng_compileClass(compEng *eng)
{
    int ret;
    Tokenizer* t = eng->tknzr;

    // Open tag ...........................................
    eng->recurseLevel++;
    write_output(eng, "<class>\n");

    // Compile according to rule ..........................
    EXIT_ON_ERR(consume_keyword(eng, KW_CLASS));
    write_keyword(eng, KW_CLASS);
    
    EXIT_ON_ERR(consume_identifier(eng));
    write_identifier(eng, &eng->tknzr->prevTok);

    EXIT_ON_ERR(consume_symbol(eng, '{'));
    write_symbol(eng, '{');

    while (t->currTok.keyword == KW_STATIC || t->currTok.keyword == KW_FIELD) {
        EXIT_ON_ERR(compEng_compileClassVarDec(eng));
    }

    while (t->currTok.keyword == KW_FUNCTION
            || t->currTok.keyword == KW_METHOD
            || t->currTok.keyword == KW_CONSTRUCTOR)
    {
        EXIT_ON_ERR(compEng_compileSubroutineDec(eng));
    }

    EXIT_ON_ERR(consume_symbol(eng, '}'));
    write_symbol(eng, '}');

    // Close tag ..........................................
    write_output(eng, "</class>\n");
    eng->recurseLevel--;

    return 0;
}

// Rule:
// ('static'|'field') type varName (','varName)* ';'
int compEng_compileClassVarDec(compEng* eng)
{
    int        ret;
    Tokenizer* t = eng->tknzr;
    Keyword    found_Kw;
    Keyword    kw_options[] = {KW_STATIC, KW_FIELD};

    // Open tag ...........................................
    eng->recurseLevel++;
    write_output(eng, "<classVarDec>\n");

    // Compile according to rule
    found_Kw = consume_keyword_if_found(eng, kw_options, ARR_SIZE(kw_options));
    if (found_Kw == KW_INVALID) {
        return -EINVAL;
    }
    write_keyword(eng, found_Kw);

    EXIT_ON_ERR(compEng_compileTypeVarName(eng));

    while (true) {
        ret = consume_symbol(eng, ';');
        if (ret == 0) {
            // Success, we found symbol closing statement
            write_symbol(eng, ';');
            break;
        }

        EXIT_ON_ERR(consume_symbol(eng, ','));
        write_symbol(eng, ',');

        EXIT_ON_ERR(consume_identifier(eng));
        write_identifier(eng, &t->prevTok);
    }

    // Close tag ..........................................
    write_output(eng, "</classVarDec>\n");
    eng->recurseLevel--;

    return 0;
}

// Rule:
// ('constructor'|'function'|'method')('void'|type)subroutineName
// '(' parameterList ')' subroutineBody
int compEng_compileSubroutineDec(compEng* eng)
{
    int        ret;
    Tokenizer* t = eng->tknzr;
    Keyword    found_Kw;
    Keyword    kw_options[] = {KW_METHOD, KW_FUNCTION, KW_CONSTRUCTOR};

    // Open tag ...........................................
    eng->recurseLevel++;
    write_output(eng, "<subroutineDec>\n");

    // Compile according to rule ..........................
    found_Kw = consume_keyword_if_found(eng, kw_options, ARR_SIZE(kw_options));
    if (found_Kw == KW_INVALID) {
        return -EINVAL;
    }
    write_keyword(eng, found_Kw);

    if (t->currTok.keyword == KW_VOID) {
        EXIT_ON_ERR(consume_keyword(eng, KW_VOID));
        write_keyword(eng, KW_VOID);
    }
    else {
        EXIT_ON_ERR(compEng_compileType(eng));
    }

    EXIT_ON_ERR(consume_identifier(eng));
    write_identifier(eng, &t->prevTok);

    EXIT_ON_ERR(consume_symbol(eng, '('));
    write_symbol(eng, '(');

    EXIT_ON_ERR(compEng_compileParameterList(eng));

    EXIT_ON_ERR(consume_symbol(eng, ')'));
    write_symbol(eng, ')');

    EXIT_ON_ERR(compEng_compileSubroutineBody(eng));

    // Close tag ..........................................
    write_output(eng, "</subroutineDec>\n");
    eng->recurseLevel--;

    return 0;
}

// Rule:
// 'int'|'char'|'boolean'|className
int compEng_compileType(compEng* eng)
{
    int        ret;
    Tokenizer *t = eng->tknzr;
    Keyword    found_Kw = KW_INVALID;
    Keyword    kw_options[] = {KW_INT, KW_CHAR, KW_BOOLEAN};

    found_Kw = consume_keyword_if_found(eng, kw_options, ARR_SIZE(kw_options));
    if (found_Kw != KW_INVALID) {
        write_keyword(eng, found_Kw);
        return 0;
    }

    EXIT_ON_ERR(consume_identifier(eng));
    write_identifier(eng, &t->prevTok);

    return 0;
}

// Rule:
// ((type varName) (',' type varName)*)?
int compEng_compileParameterList(compEng* eng)
{
    int ret;
    Tokenizer* t = eng->tknzr;

    // Open tag ...........................................
    eng->recurseLevel++;
    write_output(eng, "<parameterList>\n");

    // Compile according to rule ..........................
    EXIT_ON_ERR(compEng_compileTypeVarName(eng));

    while (t->content[t->currTok.start] == ',') {
        EXIT_ON_ERR(consume_symbol(eng, ','));
        write_symbol(eng, ',');

        EXIT_ON_ERR(compEng_compileTypeVarName(eng));
    }

    // Close tag ..........................................
    write_output(eng, "</parameterList>\n");
    eng->recurseLevel--;

    return 0;
}

// Rule:
// '{' varDec* statements '}'
// Note: statements = statement*
int compEng_compileSubroutineBody(compEng* eng)
{
    int ret;
    Tokenizer* t = eng->tknzr;

    // Open tag ...........................................
    eng->recurseLevel++;
    write_output(eng, "<subroutineBody>\n");

    // Compile according to rule ..........................
    EXIT_ON_ERR(consume_symbol(eng, '{'));
    write_symbol(eng, '{');

    while (t->currTok.keyword == KW_VAR) {
        EXIT_ON_ERR(compEng_compileVarDec(eng));
    }

    while (is_statement_keyword(t->currTok.keyword)) {
        EXIT_ON_ERR(compEng_compileStatement(eng));
    }

    EXIT_ON_ERR(consume_symbol(eng, '}'));
    write_symbol(eng, '}');

    // Close tag ..........................................
    write_output(eng, "</subroutineBody>\n");
    eng->recurseLevel--;

    return 0;
}

// Rule:
// 'var' type varName (',' varName)* ';'
int compEng_compileVarDec(compEng* eng)
{
    return 0;
}

// Helper to compile frequently appearing rule:
// type varName
int compEng_compileTypeVarName(compEng* eng)
{
    int ret;
    Tokenizer* t = eng->tknzr;

    EXIT_ON_ERR(compEng_compileType(eng));

    EXIT_ON_ERR(consume_identifier(eng));
    write_identifier(eng, &t->prevTok);

    return 0;
}

// ********************************************************
// Statements
// ********************************************************

// Rule:
// letStatement | doStatement | ifStatement | whileStatement | returnStatement
int compEng_compileStatement(compEng* eng)
{
    int ret;
    Tokenizer* t = eng->tknzr;

    // Open tag ...........................................
    eng->recurseLevel++;
    write_output(eng, "<statement>\n");

    // Compile according to rule ..........................
    switch (t->currTok.keyword) {
        case KW_LET:
            EXIT_ON_ERR(compEng_compileLetStatement(eng));
            break;
        case KW_DO:
            EXIT_ON_ERR(compEng_compileDoStatement(eng));
            break;
        case KW_IF:
            EXIT_ON_ERR(compEng_compileIfStatement(eng));
            break;
        case KW_WHILE:
            EXIT_ON_ERR(compEng_compileWhileStatement(eng));
            break;
        case KW_RETURN:
            EXIT_ON_ERR(compEng_compileReturnStatement(eng));
            break;
        default:
            return -EINVAL;
    }

    // Close tag ..........................................
    write_output(eng, "</statement>\n");
    eng->recurseLevel--;

    return 0;
}

// Rule:
// 'let' varName ('[' expression ']')? '=' expression ';'
int compEng_compileLetStatement(compEng* eng)
{
    int ret;
    Tokenizer* t = eng->tknzr;

    EXIT_ON_ERR(consume_keyword(eng, KW_LET));
    write_keyword(eng, KW_LET);

    EXIT_ON_ERR(consume_identifier(eng));
    write_identifier(eng, &t->prevTok);

    if (t->content[t->currTok.start] == '[') {
        EXIT_ON_ERR(consume_symbol(eng, '['));
        write_symbol(eng, '[');

        EXIT_ON_ERR(compEng_compileExpression(eng));

        EXIT_ON_ERR(consume_symbol(eng, ']'));
        write_symbol(eng, ']');
    }

    EXIT_ON_ERR(consume_symbol(eng, '='));
    write_symbol(eng, '=');

    EXIT_ON_ERR(compEng_compileExpression(eng));

    EXIT_ON_ERR(consume_symbol(eng, ';'));
    write_symbol(eng, ';');

    return 0;
}

// Rule:
// 'do' subroutineCall ';'
int compEng_compileDoStatement(compEng* eng)
{
    int ret;
    Tokenizer* t = eng->tknzr;

    EXIT_ON_ERR(consume_keyword(eng, KW_DO));
    write_keyword(eng, KW_DO);

    EXIT_ON_ERR(compEng_compileSubroutineCall(eng));

    EXIT_ON_ERR(consume_symbol(eng, ';'));
    write_symbol(eng, ';');

    return 0;
}

// Rule:
// 'if' '(' expression ')' '{' statements '}'
// ('else' '{' statements '}')?
int compEng_compileIfStatement(compEng* eng)
{
    int ret;
    Tokenizer* t = eng->tknzr;

    EXIT_ON_ERR(consume_keyword(eng, KW_IF));
    write_keyword(eng, KW_IF);

    EXIT_ON_ERR(consume_symbol(eng, '('));
    write_symbol(eng, '(');

    EXIT_ON_ERR(compEng_compileExpression(eng));

    EXIT_ON_ERR(consume_symbol(eng, ')'));
    write_symbol(eng, ')');

    EXIT_ON_ERR(consume_symbol(eng, '{'));
    write_symbol(eng, '{');

    while (is_statement_keyword(t->currTok.keyword)) {
        EXIT_ON_ERR(compEng_compileStatement(eng));
    }

    EXIT_ON_ERR(consume_symbol(eng, '}'));
    write_symbol(eng, '}');

    if (t->currTok.keyword == KW_ELSE) {
        EXIT_ON_ERR(consume_symbol(eng, '{'));
        write_symbol(eng, '{');

        while (is_statement_keyword(t->currTok.keyword)) {
            EXIT_ON_ERR(compEng_compileStatement(eng));
        }

        EXIT_ON_ERR(consume_symbol(eng, '}'));
        write_symbol(eng, '}');
    }

    return 0;
}

// Rule:
// 'while' '(' expression ')' '{' statements '}'
int compEng_compileWhileStatement(compEng *eng)
{
    int ret;
    Tokenizer* t = eng->tknzr;

    EXIT_ON_ERR(consume_keyword(eng, KW_WHILE));
    write_keyword(eng, KW_WHILE);

    EXIT_ON_ERR(consume_symbol(eng, '('));
    write_symbol(eng, '(');

    EXIT_ON_ERR(compEng_compileExpression(eng));

    EXIT_ON_ERR(consume_symbol(eng, ')'));
    write_symbol(eng, ')');

    EXIT_ON_ERR(consume_symbol(eng, '{'));
    write_symbol(eng, '{');

    while (is_statement_keyword(t->currTok.keyword)) {
        EXIT_ON_ERR(compEng_compileStatement(eng));
    }

    EXIT_ON_ERR(consume_symbol(eng, '}'));
    write_symbol(eng, '}');

    return 0;
}

// Rule:
// 'return' expression? ';'
int compEng_compileReturnStatement(compEng* eng)
{
    int ret;
    Tokenizer* t = eng->tknzr;

    EXIT_ON_ERR(consume_keyword(eng, KW_RETURN));
    write_keyword(eng, KW_RETURN);

    // We must either get ';' or a valid expression
    if (!(t->content[t->currTok.start] == ';')) {
        EXIT_ON_ERR(compEng_compileExpression(eng));
    }

    EXIT_ON_ERR(consume_symbol(eng, ';'));
    write_symbol(eng, ';');

    return 0;
}

// ********************************************************
// Expressions
// ********************************************************

int compEng_compileExpression(compEng* eng)
{
    int ret;
    Tokenizer* t = eng->tknzr;

    EXIT_ON_ERR(consume_identifier(eng));
    write_identifier(eng, &t->prevTok);

    return 0;
}

// Rule:
// integerConstant | stringConstant | keywordConstant | varName |
// varName '[' expression ']' | subroutineCall | '(' expression ')' |
// unaryOp term
int compEng_compileTerm(compEng* eng)
{
    return 0;
}

// Rule:
// subroutineName '(' expressionList ')' |
// (className | varName) '.' subroutineName '(' expressionList ')'
int compEng_compileSubroutineCall(compEng* eng)
{
    int ret;
    Tokenizer* t = eng->tknzr;

    // Open tag ...........................................
    eng->recurseLevel++;
    write_output(eng, "<subroutineCall>\n");

    // Compile according to rule ..........................

    EXIT_ON_ERR(consume_identifier(eng));
    write_identifier(eng, &t->prevTok);

    if (t->content[t->currTok.start] == '.') {
        EXIT_ON_ERR(consume_symbol(eng, '.'));
        write_symbol(eng, '.');

        EXIT_ON_ERR(consume_identifier(eng));
        write_identifier(eng, &t->prevTok);
    }

    EXIT_ON_ERR(consume_symbol(eng, '('));
    write_symbol(eng, '(');

    EXIT_ON_ERR(compEng_compileExpressionList(eng));

    EXIT_ON_ERR(consume_symbol(eng, ')'));
    write_symbol(eng, ')');

    // Close tag ...........................................
    write_output(eng, "</subroutineCall>\n");
    eng->recurseLevel--;

    return 0;
}

int compEng_compileExpressionList(compEng* eng)
{
    int ret;
    Tokenizer* t = eng->tknzr;

    // Open tag ...........................................
    eng->recurseLevel++;
    write_output(eng, "<expressionList>\n");

    // Compile according to rule ..........................

    // Close tag ..........................................
    write_output(eng, "<expressionList>\n");
    eng->recurseLevel--;

    return 0;
}