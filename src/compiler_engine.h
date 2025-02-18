#ifndef COMPILER_ENGINE_H
#define COMPILER_ENGINE_H

#include <stdint.h>
#include <stdio.h>
#include "tokenizer.h"

typedef struct compEng {
    FILE* outputFile;
    Tokenizer* tknzr;
    uint8_t recurseLevel;
} compEng;

int compEng_new(compEng* eng, Tokenizer* t);

void compEng_close(compEng* eng);

int compEng_compileClass(compEng* eng);

int compEng_compileClassVarDec(compEng* eng);

int compEng_compileSubroutineDec(compEng* eng);

#endif // COMPILER_ENGINE_H