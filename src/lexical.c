#include "token.c"
#include "scanner.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 词法分析器。
typedef struct lex {
  // 文件浏览器
  ScannerType* scanner;
  // 当前输入字符
  char c;
  // 缓冲区：用于存储字面量
  char buf[256];
  // 缓冲区指针
  char* p;
  // 字面量字符串
  char* literal;
  // 记录起始位置
  int start;
} LexType;



// 初始化词法分析器。
LexType* lexInit(FILE* file) {
  LexType* lex = (LexType*)malloc(sizeof(LexType));
  lex->scanner = scannerInit(file);
  memset(lex->buf, 0, 256);
  lex->p = lex->buf;
  lex->start = 1;
  return lex;
}
// 关闭词法分析器。
void stopLex(LexType* lex) {
  free(lex->scanner);
  free(lex);
}
// 打印词法分析错误。
void lex_error(LexType* lex, char* err) {
  fprintf(stderr, "Line %d, Col %d: %s\n", lex->scanner->row, lex->scanner->column, err);
  exit(1);
}
// 获得下一个 token 。
TokenType nextToken(LexType* lex) {
  int real = 0;
  while ((lex->c = nextChar(lex->scanner)) != EOF) {
    lex->start = lex->scanner->column;
    switch (lex->c) {
      // 忽略空格和换行符。
      case ' ':
      case '\t':
        break;
      // 分析下一行。
      case '\n':
        nextLine(lex->scanner);
        break;
      // 分析 != 。
      case '!':
        (lex->c = nextChar(lex->scanner));
        if (lex->c == '=') {
          return NE;
        }
        lex_error(lex, "Undefined symbol.");
      // 分析 == 。
      case '=':
        (lex->c = nextChar(lex->scanner));
        if (lex->c == '=') {
          return EQ;
        }
        lex_error(lex, "Undefined symbol.");
      // 分析 := 。
      case ':':
        (lex->c = nextChar(lex->scanner));
        if (lex->c == '=') {
          return ASSIGN;
        }
        lex_error(lex, "Undefined symbol.");
      // 分析操作符。
      case '(':
      case ')':
      case ',':
      case ';':
      case '+':
      case '-':
      case '*':
        return lex->c;
      // 分析 / 是除法操作符还是注释。
      case '/':
        (lex->c = nextChar(lex->scanner));
        // 如果后面不为*，则直接返回
        if (lex->c != '*') {
          lastChar(lex->scanner, lex->c);
          return '/';
        }
        // 判断注释格式是否正确
        while ((lex->c = nextChar(lex->scanner)) != EOF) {
        comment:
          if (lex->c == EOF) {
            lex_error(lex, "Comment symbols don't match.");
          }
          if (lex->c == '\n') {
            nextLine(lex->scanner);
            continue;
          }
          if (lex->c == '*') {
            (lex->c = nextChar(lex->scanner));
            if (lex->c == '/') {
              break;
            }
            goto comment;
          }
        }
        break;
      // 分析字符串字面量
      case '"':
        *(lex->p++) = lex->c;
        while ((lex->c = nextChar(lex->scanner)) != EOF) {
          *(lex->p++) = lex->c;
          if (lex->c == EOF || lex->c == '\n') {
            lex_error(lex, "String symbols don't match.");
          }
          if (lex->c == '"') {
            *lex->p = '\0';
            lex->literal = (lex->p = lex->buf);
            break;
          }
        }
        return STRING_LITERAL;
      // 分析数值字面量
      case '0' ... '9':
        real = 0;
        *(lex->p++) = lex->c;
        while ((lex->c = nextChar(lex->scanner)) != EOF) {
          switch (lex->c) {
            case '.':
              real = 1;
            case '0' ... '9':
              *(lex->p++) = lex->c;
              break;
            default:
              *lex->p = '\0';
              // 存放字面量。
              lex->literal = (lex->p = lex->buf);
              lastChar(lex->scanner, lex->c);
              if (real) {
                return REAL_LITERAL;
              }
              return INT_LITERAL;
          }
        }
        break;
      // 分析标识符
      case 'a' ... 'z':
      case 'A' ... 'Z':
      case '_':
        *(lex->p++) = lex->c;
        while ((lex->c = nextChar(lex->scanner)) != EOF) {
          switch (lex->c) {
            case '0' ... '9':
            case 'a' ... 'z':
            case 'A' ... 'Z':
            case '_':
              *(lex->p++) = lex->c;
              break;
            default:
              *lex->p = '\0';
              lex->literal = (lex->p = lex->buf);
              lastChar(lex->scanner, lex->c);
              // int flag = 1;
              // for (int i = 0; i < 11; i++) {
              //   if (strcmp(lex->buf, keywords[i]) == 0) {
              //     printf("{ %s: %s }, ", lex->buf, lex->buf);
              //     flag = 0;
              //     break;
              //   }
              // }
              // if (flag) printf("{ INDENTIFIER: %s }, ", lex->buf);
              return matchKeyword(lex->buf);
          }
        }
        break;
      default:
        lex_error(lex, "Undefined symbol.");
    }
  }
  return -1;
}