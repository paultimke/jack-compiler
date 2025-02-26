#include <stdio.h>
#include "tokenizer.h"
#include "err_handler.h"
#include "compiler_engine.h"

static const char* tokType_enum2str[TOK_TYPE_COUNT] = {
    "keyword", "symbol", "identifier", "int-const", "string-const"
};

/*****************************************************************************/
/* FUNCTION PROTOTYPES */
/*****************************************************************************/
int processKeyword(Tokenizer* t, compEng* eng);

/*****************************************************************************/
/* ENTRY POINT */
/*****************************************************************************/

int main(int argc, char **argv)
{
    int ret;
    Tokenizer tokenizer;
    compEng compEng;
    char tokStr[100];
    const char* inputPath;

    if (argc != 2) {
        LOG_ERR("Please provide input file or directory\n");
    }

    // Create objects
    ret = tknzr_new(&tokenizer, argv[1]);
    if (ret < 0) {
        return ret;
    }

    ret = compEng_new(&compEng, &tokenizer);
    if (ret < 0) {
        return ret;
    }

    // Start compilation process
    while (tknzr_has_more_tokens(&tokenizer)) {
        tknzr_advance(&tokenizer);

        switch(tokenizer.currTok.type) {
            case TOK_TYPE_KEYWORD:
                ret = processKeyword(&tokenizer, &compEng);
                break;
            case TOK_TYPE_SYMBOL:
                break;
            case TOK_TYPE_IDENTIFIER:
                break;
            case TOK_TYPE_INT_CONST:
                break;
            case TOK_TYPE_STRING_CONST:
                break;
            default:
                break;
        }
    }

    if (ret < 0) {
        switch (ret) {
            case -EINVAL:
                LOG_ERR("Parse Error: Did not get expected token");
            default:
                break;
        }
    }

    tknzr_close(&tokenizer);
    compEng_close(&compEng);

    return ret;
}

/*****************************************************************************/
/* PRIVATE FUNCTIONS */
/*****************************************************************************/
int processKeyword(Tokenizer* t, compEng* eng)
{
    int ret;

    switch (t->currTok.keyword) {
        case KW_CLASS:
            ret = compEng_compileClass(eng);
            break;
        case KW_CONSTRUCTOR:
            break;
        case KW_FUNCTION:
            break;
        case KW_METHOD:
            break;
        case KW_FIELD:
            break;
        case KW_STATIC:
            break;
        case KW_VAR:
            break;
        case KW_INT:
            break;
        case KW_CHAR:
            break;
        case KW_BOOLEAN:
            break;
        case KW_VOID:
            break;
        case KW_TRUE:
            break;
        case KW_FALSE:
            break;
        case KW_NULL:
            break;
        case KW_THIS:
            break;
        case KW_LET:
            break;
        case KW_DO:
            break;
        case KW_IF:
            break;
        case KW_ELSE:
            break;
        case KW_WHILE:
            break;
        case KW_RETURN:
            break;
        default:
            break;
    }

    return ret;
}
