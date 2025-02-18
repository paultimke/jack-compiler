#include <string.h>
#include "compiler_engine.h"
#include "tokenizer.h"
#include "err_handler.h"

/*****************************************************************************/
/* PRIVATE FUNCTIONS */
/*****************************************************************************/
static inline int write_output(compEng *eng, const char* s)
{
    (void)eng;
    printf("%s\n", s);
    return 0;
}

static inline int write_output_n(compEng *eng, const char* s, int n)
{
    (void)eng;
    char buffer[100];
    snprintf(buffer, n + 1, "%s", s);
    printf("%s\n", buffer);
    return 0;
}

static inline int write_identifier(compEng *eng)
{
    Tokenizer *t = eng->tknzr;
    int identifierLen = t->tokEnd - t->tokStart;

    write_output(eng, "<identifier>");
    write_output_n(eng, &t->content[t->tokStart], identifierLen);
    write_output(eng, "</identifier>");
    return 0;
}

static inline int write_keyword(compEng *eng, Keyword keyword)
{
    Tokenizer *t = eng->tknzr;

    write_output(eng, "<keyword>");
    write_output(eng, keywords[keyword]);
    write_output(eng, "</keyword>");
    return 0;
}

static inline int write_symbol(compEng *eng, const char* symbol)
{
    Tokenizer *t = eng->tknzr;

    write_output(eng, "<symbol>");
    write_output(eng, symbol);
    write_output(eng, "</symbol>");
    return 0;
}

int eat_type(compEng *eng, TokenType tokType)
{
    Tokenizer *t = eng->tknzr;

    if (t->tokType == tokType) {
        if (tknzr_has_more_tokens(t)) {
            tknzr_advance(t);
        }
        else {
            return -2;
        }
    }
    else {
        LOG_ERR("Did not get expected token");
        return -1;
    }
    return 0;
}

int eat_keyword(compEng *eng, Keyword keyword)
{
    Tokenizer *t = eng->tknzr;

    if (t->tokKeyword == keyword) {
        if (tknzr_has_more_tokens(t)) {
            tknzr_advance(t);
        }
        else {
            return -2;
        }
    }
    else {
        LOG_ERR("Did not get expected token");
        return -1;
    }
    return 0;
}

int eat_identifier(compEng *eng)
{
    Tokenizer *t = eng->tknzr;

#if 0
    if (t->tokKeyword == ) {
        if (tknzr_has_more_tokens(t)) {
            tknzr_advance(t);
        }
        else {
            return -2;
        }
    }
    else {
        LOG_ERR("Did not get expected token");
        return -1;
    }
#endif
    return 0;
}

int eat_str_literal(compEng *eng, const char* s)
{
    char strBuf[MAX_IDENTIFIER_STR_LEN];
    Tokenizer *t = eng->tknzr;

    tknzr_get_string_val(t, strBuf, sizeof(strBuf));

    if (strncmp(strBuf, s, sizeof(strBuf)) == 0) {
        if (tknzr_has_more_tokens(t)) {
            tknzr_advance(t);
        }
        else {
            return -2;
        }
    }
    else {
        LOG_ERR("Did not get expected token");
        return -1;
    }

    return 0;
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

int compEng_compileClass(compEng *eng)
{
    // Rule:
    // 'class' className '{' classVarDec* subroutineDec* '}'
    
    int ret;

    // Open class tag
    write_output(eng, "<class>");

    // Compile according to rule
    ret = eat_keyword(eng, KW_CLASS);
    if (ret < 0) {
        return ret;
    }
    write_keyword(eng, KW_CLASS);
    
    ret = eat_type(eng, TOK_TYPE_IDENTIFIER);
    if (ret < 0) {
        return ret;
    }
    write_identifier(eng);

    ret = eat_str_literal(eng, "{");
    if (ret < 0) {
        return ret;
    }
    write_symbol(eng, "{");

    ret = compEng_compileClassVarDec(eng);
    if (ret < 0) {
        return ret;
    }

    ret = compEng_compileSubroutineDec(eng);
    if (ret < 0) {
        return ret;
    }

    ret = eat_str_literal(eng, "}");
    if (ret < 0) {
        return ret;
    }
    write_symbol(eng, "}");

    // Close class tag
    write_output(eng, "</class>");

    return 0;
}

int compEng_compileClassVarDec(compEng* eng)
{
    // Rule:
    // ('static'|'field') type varName (','varName)* ';'
    return 0;
}

int compEng_compileSubroutineDec(compEng* eng)
{
    // Rule:
    // ('constructor'|'function'|'method')('void'|type)subroutineName
    // '(' parameterList ')' subroutineBody

    return 0;
}

// type rule:
// 'int'|'char'|'boolean'|className
//
// parameterList rule:
// ((type varName) (',' type varName)*)?
//
// subroutineBody rule:
// '{' varDec* statements '}'
//
// varDec rule:
// 'var' type varName (',' varName)* ';'
