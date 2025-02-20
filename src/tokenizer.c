#include <stdlib.h>
#include <string.h>
#include "tokenizer.h"
#include "err_handler.h"

#define NUMBER_OF_SYMBOLS  sizeof(symbols)
#define NUMBER_OF_KEYWORDS KW_COUNT

/*****************************************************************************/
/* PRIVATE VARIABLES */
/*****************************************************************************/
static Token defaultToken = {
    .start = TOKEN_CURSOR_INVALID_VALUE,
    .end = TOKEN_CURSOR_INVALID_VALUE,
    .type = TOK_TYPE_INVALID,
    .keyword = KW_INVALID,
};

/*****************************************************************************/
/* PRIVATE FUNCTIONS */
/*****************************************************************************/

bool is_EOF(Tokenizer* t)
{
    return t->cursor >= t->contentLen;
}

bool is_digit(char c)
{
    return (c >= '0' && c <= '9');
}

bool is_letter(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

bool is_symbol(char c)
{
    for (uint8_t i = 0; i < NUMBER_OF_SYMBOLS; i++) {
        if (c == symbols[i]) {
            return true;
        }
    }

    return false;
}

bool is_whitespace(char c)
{
    return (c == ' ' || c == '\t' || c == '\n');
}

bool is_comment_start(Tokenizer* t)
{
    if ((t->cursor + 1) >= t->contentLen) {
        return false;
    }
    
    return (t->content[t->cursor] == '/') && (t->content[t->cursor + 1] == '/');
}

void strip_whitespace(Tokenizer* t)
{
    while (is_whitespace(t->content[t->cursor]) && !is_EOF(t)) {
        t->cursor++;
    }
}

void remove_whitespace_and_comments(Tokenizer* t)
{
    while (!is_EOF(t))
    {
        if (is_whitespace(t->content[t->cursor])) {
            strip_whitespace(t);
        }
        else if (is_comment_start(t)) {
            while (t->content[t->cursor] != '\n') {
                t->cursor++;
            }
        }
        else {
            break;
        }
    }
}

Keyword get_keyword_type(const char* s, uint8_t s_len)
{
    for (uint8_t i = 0; i < NUMBER_OF_KEYWORDS; i++) {
        if (strncmp(s, keywords[i], s_len) == 0) {
            return (Keyword)i;
        }
    }
    return KW_INVALID;
}

/*****************************************************************************/
/* PUBLIC FUNCTIONS */
/*****************************************************************************/

int tknzr_new(Tokenizer* t, const char* path)
{
    FILE* file;
    uint64_t fileSize;
    char* buffer = NULL;

    file = fopen(path, "rb");
    if (NULL == file) {
        fclose(file);
        LOG_ERR("No such file %s", path);
        return -ENOENT;
    }

    // Figure out size of file as number of characters
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate buffer to hold entire file contents
    buffer = malloc(fileSize + 1);
    if (buffer == NULL) {
        fclose(file);
        LOG_ERR("Failed allocating memory\n");
        return -ENOMEM;
    }
    fread(buffer, fileSize, 1, file);
    fclose(file);
    buffer[fileSize] = '\0';

    // Initialize members
    t->content = buffer;
    t->contentLen = fileSize;
    t->cursor = 0;
    t->currTok = defaultToken;
    t->prevTok = defaultToken;

    // Remove the first encountered whitespace and comments. This has to be done
    // once at start and will be continued to be done at the end of each token
    // advance
    remove_whitespace_and_comments(t);

    return 0;
}

void tknzr_close(Tokenizer* t)
{
    if (t->content != NULL) {
        free((char*)t->content);
        t->content = NULL;
    }

    return;
}

bool tknzr_has_more_tokens(Tokenizer *t)
{
    return t->cursor < t->contentLen;
}

void tknzr_advance(Tokenizer *t)
{
    // Copy currTok to be the previous before advancing
    t->prevTok = t->currTok;

    // Reset keyword type
    t->currTok.keyword = KW_INVALID;

    if (t->content[t->cursor] == '"') {
        // String literal
        t->currTok.type = TOK_TYPE_STRING_CONST;
        t->cursor++;
        t->currTok.start = t->cursor;

        while (!is_EOF(t) && t->content[t->cursor] != '"') {
            t->cursor++;
        }

        t->currTok.end = t->cursor;
        t->cursor++; // Advance one more to get rid of closing '""
    } 
    else if (is_digit(t->content[t->cursor])) {
        // integer constant
        t->currTok.type = TOK_TYPE_INT_CONST;
        t->currTok.start = t->cursor;

        while (!is_EOF(t) && is_digit(t->content[t->cursor])) {
            t->cursor++;
        }

        t->currTok.end = t->cursor;
    } 
    else if (is_symbol(t->content[t->cursor])) {
        // Symbol (nothing else to do, since symbols are one character)
        t->currTok.type = TOK_TYPE_SYMBOL;
        t->currTok.start = t->cursor;
        t->cursor++;
        t->currTok.end = t->cursor;
    }
    else if (is_letter(t->content[t->cursor]) || t->content[t->cursor] == '_') {
        // Could be either identifier or keyword. We need to get the complete
        // token and decide its type at the end
        t->currTok.start = t->cursor;

        while (is_letter(t->content[t->cursor])
                || is_digit(t->content[t->cursor])
                || t->content[t->cursor] == '_') 
        {
            t->cursor++;
        }

        // Now that we have the token, check if it's keyword or identifier
        Keyword kw = get_keyword_type(&t->content[t->currTok.start], t->cursor - t->currTok.start);
        if (kw != KW_INVALID) {
            t->currTok.type = TOK_TYPE_KEYWORD;
            t->currTok.keyword = kw;
        }
        else {
            t->currTok.type = TOK_TYPE_IDENTIFIER;
        }

        t->currTok.end = t->cursor;
    }
    
    // Remove all whitespace and comments between the end of this token
    // and the next one
    remove_whitespace_and_comments(t);
}

void tknzr_get_string_val(Tokenizer *t, char* dst, uint16_t dstSize)
{
    uint16_t strLen = t->currTok.end - t->currTok.start;

    if (dstSize < (strLen + 1)) {
        return;
    }

    // Copy to the buffer and add null terminator
    strncpy(dst, &t->content[t->currTok.start], strLen);
    dst[strLen] = '\0';
}
