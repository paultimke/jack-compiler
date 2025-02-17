#include <stdio.h>
#include "tokenizer.h"
#include "err_handler.h"

static const char* tokType_enum2str[TOK_TYPE_COUNT] = {
    "keyword", "symbol", "identifier", "int-const", "string-const"
};

int main(int argc, char **argv)
{
    int ret;
    Tokenizer tokenizer;
    char tokStr[100];
    const char* inputPath;

    if (argc != 1) {
        LOG_ERR("Please provide input file or directory\n");
    }

    ret = tknzr_new(&tokenizer, argv[1]);
    if (ret < 0) {
        return ret;
    }

    while (tknzr_has_more_tokens(&tokenizer)) {
        tknzr_advance(&tokenizer);

        tknzr_get_string_val(&tokenizer, tokStr, sizeof(tokStr));

        printf("%s --> %s", tokType_enum2str[tokenizer.tokType], tokStr);
        if (tokenizer.tokType == TOK_TYPE_KEYWORD) {
            printf("\tkeyword: %d", tokenizer.tokKeyword);
        }
        printf("\n");
    }

    tknzr_close(&tokenizer);

    return ret;
}