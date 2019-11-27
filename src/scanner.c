#include "stdio.h"
#include <stdlib.h>

typedef struct scanner {
  FILE* file; //文件
  int row; //行数
  int column; //列数
} ScannerType;

// 初始化文件读取器。
ScannerType* scannerInit(FILE* file) {
  ScannerType* scanner = (ScannerType*)malloc(sizeof(ScannerType));
  scanner->file = file;
  scanner->row = 1;
  scanner->column = 1;
  return scanner;
}

// 读取下一行。
void nextLine(ScannerType* scanner) {
  scanner->row++;
  scanner->column = 0;
}

// 读取下一字符。
int nextChar(ScannerType* scanner) {
  scanner->column++;
  return fgetc(scanner->file);
}

// 回退至上一字符。
void lastChar(ScannerType* scanner, char ch) {
  scanner->column--;
  ungetc(ch, scanner->file);
}