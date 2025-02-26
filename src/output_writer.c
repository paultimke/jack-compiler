#include "output_writer.h"

int write_output(compEng* eng, const char* s)
{
    for (uint8_t i = 0; i < eng->recurseLevel; i++) {
        printf("\t");
    }

    printf("%s", s);
    return 0;
}

int write_output_n(compEng* eng, const char* s, int n)
{
    for (uint8_t i = 0; i < eng->recurseLevel; i++) {
        printf("\t");
    }

    char buffer[100];
    snprintf(buffer, n + 1, "%s", s);
    printf("%s", buffer);
    return 0;
}

int write_identifier(compEng* eng, Token* tok)
{
    Tokenizer *t = eng->tknzr;
    eng->recurseLevel++;

    char identifier[MAX_IDENTIFIER_STR_LEN + 1];
    char tmpBuf[60];

    snprintf(identifier, (tok->end - tok->start) + 1, "%s", &t->content[tok->start]);
    snprintf(tmpBuf, sizeof(tmpBuf), "<identifier> %s </identifier>\n", identifier);
    write_output(eng, tmpBuf);

    eng->recurseLevel--;
    return 0;
}

int write_keyword(compEng* eng, Keyword keyword)
{
    Tokenizer *t = eng->tknzr;
    eng->recurseLevel++;

    char tmpBuf[60];
    snprintf(tmpBuf, sizeof(tmpBuf), "<keyword> %s </keyword>\n", keywords[keyword]);
    write_output(eng, tmpBuf);

    eng->recurseLevel--;
    return 0;
}

int write_symbol(compEng* eng, char symbol)
{
    Tokenizer *t = eng->tknzr;
    eng->recurseLevel++;

    char tmpBuf[60];
    snprintf(tmpBuf, sizeof(tmpBuf), "<symbol> %c </symbol>\n", symbol);
    write_output(eng, tmpBuf);

    eng->recurseLevel--;
    return 0;
}
