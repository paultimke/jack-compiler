#ifndef OUTPUT_WRITER_H
#define OUTPUT_WRITER_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "compiler_engine.h"

int write_output(compEng *eng, const char* s);
int write_output_n(compEng *eng, const char* s, int n);
int write_identifier(compEng *eng, Token* tok);
int write_keyword(compEng *eng, Keyword keyword);
int write_symbol(compEng *eng, char symbol);

#endif // OUTPUT_WRITER_H
