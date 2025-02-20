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

    write_output(eng, "<identifier>\n\t");
    write_output_n(eng, &t->content[tok->start], tok->end - tok->start);
    write_output(eng, "\n");
    write_output(eng, "</identifier>\n");

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
