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

// Program structure
int compEng_compileClass(compEng* eng);
int compEng_compileClassVarDec(compEng* eng);
int compEng_compileSubroutineDec(compEng* eng);
int compEng_compileParameterList(compEng* eng);
int compEng_compileSubroutineBody(compEng *eng);
int compEng_compileVarDec(compEng* eng);
int compEng_compileType(compEng* eng);
int compEng_compileTypeVarName(compEng* eng);

// Statements
int compEng_compileStatement(compEng* eng);
int compEng_compileLetStatement(compEng* eng);
int compEng_compileDoStatement(compEng* eng);
int compEng_compileIfStatement(compEng* eng);
int compEng_compileWhileStatement(compEng* eng);
int compEng_compileReturnStatement(compEng* eng);

// Expressions
int compEng_compileExpression(compEng* eng);
int compEng_compileTerm(compEng* eng);
int compEng_compileSubroutineCall(compEng* eng);
int compEng_compileExpressionList(compEng* eng);

#endif // COMPILER_ENGINE_H
