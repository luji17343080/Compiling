#include <stdio.h>
#include "parser.c"
int main(int argc, char** argv) {
  FILE* testFile = fopen(argv[1], "r");
  FILE* resultFile = fopen(argv[2], "w");
  ParserType* parser = parserInit(testFile, resultFile);
  parse(parser);
  fclose(testFile);
  fclose(resultFile);
  stopParser(parser);
  return 0;
}