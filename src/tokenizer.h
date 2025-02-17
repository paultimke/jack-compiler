#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>

typedef enum TokenType {
    TOK_TYPE_KEYWORD,
    TOK_TYPE_SYMBOL,
    TOK_TYPE_IDENTIFIER,
    TOK_TYPE_INT_CONST,
    TOK_TYPE_STRING_CONST,

    TOK_TYPE_COUNT,
    TOK_TYPE_INVALID = TOK_TYPE_COUNT
} TokenType;

typedef enum Keyword {
    KW_CLASS,
    KW_CONSTRUCTOR,
    KW_FUNCTION,
    KW_METHOD,
    KW_FIELD,
    KW_STATIC,
    KW_VAR,
    KW_INT,
    KW_CHAR,
    KW_BOOLEAN,
    KW_VOID,
    KW_TRUE,
    KW_FALSE,
    KW_NULL,
    KW_THIS,
    KW_LET,
    KW_DO,
    KW_IF,
    KW_ELSE,
    KW_WHILE,
    KW_RETURN,

    KW_COUNT,
    KW_INVALID = KW_COUNT
} Keyword;

typedef struct Tokenizer {
    const char* content;
    uint64_t contentLen;
    uint64_t cursor;

    uint64_t tokStart;
    uint64_t tokEnd;
    TokenType tokType;
    Keyword tokKeyword;
} Tokenizer;

int tknzr_new(Tokenizer *t, const char* path);

void tknzr_close(Tokenizer *t);

bool tknzr_has_more_tokens(Tokenizer *t);

void tknzr_advance(Tokenizer *t);

void tknzr_get_string_val(Tokenizer *t, char* dst, uint16_t dstSize);

#endif /* TOKENIZER_H */