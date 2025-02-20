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

    return consume_token_helper(eng, condition, TOK_TYPE_KEYWORD);
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
    ret = consume_keyword(eng, KW_CLASS);
    if (ret < 0) {
        return ret;
    }
    write_keyword(eng, KW_CLASS);
    
    ret = consume_identifier(eng);
    if (ret < 0) {
        return ret;
    }
    write_identifier(eng, &eng->tknzr->prevTok);

    ret = consume_symbol(eng, '{');
    if (ret < 0) {
        return ret;
    }
    write_symbol(eng, '{');

    while (t->currTok.keyword == KW_STATIC || t->currTok.keyword == KW_FIELD) {
        ret = compEng_compileClassVarDec(eng);
        if (ret < 0) {
            return ret;
        }
    }

    while (t->currTok.keyword == KW_FUNCTION
            || t->currTok.keyword == KW_METHOD
            || t->currTok.keyword == KW_CONSTRUCTOR)
    {
        ret = compEng_compileSubroutineDec(eng);
        if (ret < 0) {
            return ret;
        }
    }

    ret = consume_symbol(eng, '}');
    if (ret < 0) {
        return ret;
    }
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
    Tokenizer *t = eng->tknzr;
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

    ret = compEng_compileType(eng);
    if (ret < 0) {
        return ret;
    }

    ret = consume_identifier(eng);
    if (ret < 0) {
        return ret;
    }
    write_identifier(eng, &t->prevTok);

    while (true) {
        ret = consume_symbol(eng, ';');
        if (ret == 0) {
            // Success, we found symbol closing statement
            write_symbol(eng, ';');
            break;
        }

        ret = consume_symbol(eng, ',');
        if (ret < 0) {
            return ret;
        }
        write_symbol(eng, ',');

        ret = consume_identifier(eng);
        if (ret < 0) {
            return ret;
        }
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

    ret = consume_identifier(eng);
    if (ret < 0) {
        return ret;
    }
    write_identifier(eng, &t->prevTok);

    return 0;
}

// parameterList rule:
// ((type varName) (',' type varName)*)?
//
// subroutineBody rule:
// '{' varDec* statements '}'
//
// varDec rule:
// 'var' type varName (',' varName)* ';'
