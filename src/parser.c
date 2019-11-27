#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "lexical.c"

// 语法分析器
typedef struct parser {
  // 输出文件
  FILE* output;
  // 词法分析器
  LexType* lex;
  // 当前分析字符
  TokenType token;
  // 缩进的数目
  int indentCounts;
}ParserType;

/* 根据 EBNF Grammar 对程序进行语法分析，构造语法树 */
void parse_error(ParserType* parser, int n, ...);
void Program(ParserType* parser); //Tiny程序
void MethodDecl(ParserType* parser); //方法声明
void GlobalVarDecl(ParserType* parser); //全局变量声明
void Type(ParserType* parser); //类型
void Id(ParserType* parser); //名称
void FormalParams(ParserType* parser); //形参
void FormalParam(ParserType* parser); //具体形参
void Block(ParserType* parser); //函数块
void Statement(ParserType* parser); //语句
void LocalVarDecl(ParserType* parser); //局部变量
void AssignStmt(ParserType* parser); //赋值语句
void ReturnStmt(ParserType* parser); //Return语句
void IfStmt(ParserType* parser); //IF语句
void WriteStmt(ParserType* parser); //Write语句
void ReadStmt(ParserType* parser); //Read语句
void Expression(ParserType* parser); //表达式
void MultiplicativeExpr(ParserType* parser); //乘法表达式
void PrimaryExpr(ParserType* parser); //基本表达式
void ActualParams(ParserType* parser); //实际参数
void BoolExpression(ParserType* parser); //bool表达式

#define TOKEN parser->token
#define EQUAL(TokenType) (TOKEN == TokenType)
#define OUT parser->output
#define INDENT parser->indentCounts
#define LEX parser->lex
#define SCANNER parser->lex->scanner

/* 回退至上一token */
#define LAST_TOKEN(tokens)               \
  do {                                   \
    int len = strlen(tokens);            \
    for (int i = len - 1; i >= 0; --i) { \
      lastChar(SCANNER, tokens[i]);     \
    }                                    \
  } while (0)

/* 打印缩进以及"-> */
#define PRINT_INDENT                           \
  do {                                         \
    for (int i = 0; i < INDENT; ++i) { \
      fprintf(OUT, "\t");                      \
    } \
    if (INDENT > 0)                                          \
      fprintf(OUT, "->"); \
  } while (0)

/* 对某个函数块进行语法分析 */
#define PARSE(func)              \
  do {                             \
    INDENT++;                    \
    PRINT_INDENT;                  \
    fprintf(OUT, "%s\n", #func); \
    func(parser);                \
    INDENT--;                    \
  } while (0)
#define MATCH(func, block)           \
  do {                                \
    if (EQUAL(func)) {                 \
      block;                           \
    } else {                          \
      parse_error(parser, 1, func); \
    }                                 \
  } while (0)
#define TERMINAL(func, data, block)  \
  do {                                \
    if (EQUAL(func)) {                 \
      INDENT++;                     \
      PRINT_INDENT;                   \
      fprintf(OUT, "%s\n", data);     \
      block;                           \
      TOKEN = nextToken(LEX);                     \
      INDENT--;                     \
    } else {                          \
      parse_error(parser, 1, func); \
    }                                 \
  } while (0)

// 初始化语法分析器。
ParserType* parserInit(FILE* testFile, FILE* output) {
  ParserType* parser = (ParserType*)malloc(sizeof(ParserType));
  OUT = output;
  LEX = lexInit(testFile);
  INDENT = -1;
  return parser;
}
// 关闭语法分析器
void stopParser(ParserType* parser) {
  stopLex(LEX);
  free(parser);
}

// 打印语法分析错误
void parse_error(ParserType* parser, int n, ...) {
  fprintf(stderr, "Line %d, Pos %d: Need ", SCANNER->row, LEX->start);
  va_list ex;
  va_start(ex, n);
  for (int i = 0; i < n; ++i) {
    if (i) {
      fprintf(stderr, " or ");
    }
    fprintf(stderr, "\"%s\"", getToken(va_arg(ex, int)));
  }
  va_end(ex);
  fprintf(stderr, ", but got \"%s\".\n", getToken(TOKEN));
  exit(1);
}

// 对输入文件进行语法分析，并输出语法树。
void parse(ParserType* parser) { 
  PARSE(Program);
}

void Program(ParserType* parser) {
  TOKEN = nextToken(LEX);
  // 分析方法声明或全局变量声明
  while (1) {
    if (EQUAL(-1)) {
      break;
    }
    char* tokens[2] = {};
    if (EQUAL(INT) || EQUAL(REAL) || EQUAL(STRING)) {
      tokens[0] = strdup(LEX->literal);
      TOKEN = nextToken(LEX);
      // 分析 MAIN 方法。
      if (EQUAL(MAIN)) {
        // 回退至上一token
        LAST_TOKEN("MAIN");
        LAST_TOKEN(" ");
        LAST_TOKEN(tokens[0]);
        TOKEN = nextToken(LEX);
        PARSE(MethodDecl);
        continue;
      }
      if (EQUAL(IDENTIFIER)) {
        tokens[1] = strdup(LEX->literal);
        TOKEN = nextToken(LEX);
        // 分析全局变量声明。
        if (EQUAL(';')) {
          // 回退至上一token
          LAST_TOKEN(";");
          LAST_TOKEN(tokens[1]);
          LAST_TOKEN(" ");
          LAST_TOKEN(tokens[0]);
          TOKEN = nextToken(LEX);
          PARSE(GlobalVarDecl);
          continue;
        }
        // 分析全局变量声明。
        if (EQUAL(ASSIGN)) {
          // 回退至上一token
          LAST_TOKEN(":=");
          LAST_TOKEN(" ");
          LAST_TOKEN(tokens[1]);
          LAST_TOKEN(" ");
          LAST_TOKEN(tokens[0]);
          TOKEN = nextToken(LEX);
          PARSE(GlobalVarDecl);
          continue;
        }
        // 分析方法声明。
        if (EQUAL('(')) {
          // 回退至上一token
          LAST_TOKEN("(");
          LAST_TOKEN(tokens[1]);
          LAST_TOKEN(" ");
          LAST_TOKEN(tokens[0]);
          TOKEN = nextToken(LEX);
          PARSE(MethodDecl);
          continue;
        }
      }
      parse_error(parser, 1, IDENTIFIER);
    }
    parse_error(parser, 2, INT, REAL);
  }
}
// 分析方法声明。

void MethodDecl(ParserType* parser) {
  // 分析方法声明的返回类型。
  PARSE(Type);
  // 判断是否为 MAIN 方法。
  if (EQUAL(MAIN)) {
    TERMINAL(MAIN, getToken(TOKEN), {});
  }
  // 分析方法的标识符。
  PARSE(Id);
  // 匹配 ( 。
  MATCH('(', TOKEN = nextToken(LEX););
  // 分析形式参数。
  PARSE(formal_params);
  // 匹配 ) 。
  MATCH(')', TOKEN = nextToken(LEX););
  // 分析方法块。
  PARSE(Block);
}

// 分析全局变量声明。
void GlobalVarDecl(ParserType* parser) { 
  LocalVarDecl(parser);  
}

// 分析方法返回类型。

void Type(ParserType* parser) {
  switch (TOKEN) {
    case INT:
      TERMINAL(INT, getToken(TOKEN), {});
      break;
    case REAL:
      TERMINAL(REAL, LEX->literal, {});
      break;
    case STRING:
      TERMINAL(STRING, LEX->literal, {});
      break;
    default:
      parse_error(parser, 3, INT, REAL, STRING);
  }
}
// 分析标识符。

void Id(ParserType* parser) {
  // 打印标识符名称。
  TERMINAL(IDENTIFIER, LEX->literal, {});
}
// 分析方法的形式参数组。

void formal_params(ParserType* parser) {
  switch (TOKEN) {
    case INT:
    case REAL:
    case STRING:
      PARSE(FormalParams);
      while (EQUAL(',')) {
        MATCH(',', TOKEN = nextToken(LEX););
        PARSE(FormalParams);
      }
      break;
    case ')':
      break;
    default:
      parse_error(parser, 3, INT, REAL, ')');
  }
}
// 分析形式参数。

void FormalParams(ParserType* parser) {
  PARSE(Type);
  PARSE(Id);
}
// 分析方法块。

void Block(ParserType* parser) {
  MATCH(BEGIN, TOKEN = nextToken(LEX););
  // 分析语句。
  while (!EQUAL(END)) {
    PARSE(Statement);
  }
  MATCH(END, TOKEN = nextToken(LEX););
}
// 分析语句。

void Statement(ParserType* parser) {
  switch (TOKEN) {
    // 分析局部变量声明。
    case INT:
    case REAL:
    case STRING:
      PARSE(LocalVarDecl);
      break;
    // 分析赋值语句。
    case IDENTIFIER:
      PARSE(AssignStmt);
      break;
    // 分析块。
    case BEGIN:
      PARSE(Block);
      break;
    // 分析返回语句。
    case RETURN:
      PARSE(ReturnStmt);
      break;
    // 分析 IF 条件语句。
    case IF:
      PARSE(IfStmt);
      break;
    // 分析 READ 语句。
    case READ:
      PARSE(ReadStmt);
      break;
    // 分析 WRITE 语句。
    case WRITE:
      PARSE(WriteStmt);
      break;
    default:
      parse_error(parser, 8, INT, REAL, IDENTIFIER, BEGIN, RETURN, IF, READ,
                  WRITE);
  }
}
// 分析局部变量声明。

void LocalVarDecl(ParserType* parser) {
  PARSE(Type);
  char* token = strdup(LEX->literal);
  if (EQUAL(IDENTIFIER)) {
    TOKEN = nextToken(LEX);
    if (EQUAL(';')) {
      // 回退至上一token
      LAST_TOKEN(";");
      LAST_TOKEN(" ");
      LAST_TOKEN(token);
      TOKEN = nextToken(LEX);
      PARSE(Id);
      MATCH(';', TOKEN = nextToken(LEX););
      return;
    }
    if (EQUAL(ASSIGN)) {
      // 回退至上一token
      LAST_TOKEN(":=");
      LAST_TOKEN(" ");
      LAST_TOKEN(token);
      TOKEN = nextToken(LEX);
      PARSE(AssignStmt);
      return;
    }
  }
  parse_error(parser, 2, ';', ASSIGN);
}
// 分析赋值语句。

void AssignStmt(ParserType* parser) {
  PARSE(Id);
  TERMINAL(ASSIGN, getToken(TOKEN), {});
  if (EQUAL(STRING_LITERAL)) {
    // 分析字符串字面量。
    TERMINAL(STRING_LITERAL, getToken(TOKEN), {
      INDENT++;
      PRINT_INDENT;
      fprintf(OUT, "%s\n", LEX->literal);
      INDENT--;
    });
  } else {
    // 分析表达式。
    PARSE(Expression);
  }
  MATCH(';', TOKEN = nextToken(LEX););
}
// 分析表达式。

void Expression(ParserType* parser) {
  PARSE(MultiplicativeExpr);
  while (1) {
    if (EQUAL('+')) {
      TERMINAL('+', getToken(TOKEN), {});
      PARSE(MultiplicativeExpr);
      continue;
    }
    if (EQUAL('-')) {
      TERMINAL('-', getToken(TOKEN), {});
      PARSE(MultiplicativeExpr);
      continue;
    }
    break;
  }
}
// 分析 Multiplicative 表达式。

void MultiplicativeExpr(ParserType* parser) {
  PARSE(PrimaryExpr);
  while (1) {
    if (EQUAL('*')) {
      TERMINAL('*', getToken(TOKEN), {});
      PARSE(PrimaryExpr);
      continue;
    }
    if (EQUAL('/')) {
      TERMINAL('/', getToken(TOKEN), {});
      PARSE(PrimaryExpr);
      continue;
    }
    break;
  }
}
// 分析 Primary 表达式。

void PrimaryExpr(ParserType* parser) {
  switch (TOKEN) {
    // 分析 INT 字面量。
    case INT_LITERAL:
      TERMINAL(INT_LITERAL, getToken(TOKEN), {
        INDENT++;
        PRINT_INDENT;
        fprintf(OUT, "%s\n", LEX->literal);
        INDENT--;
      });
      break;
    // 分析 REAL 字面量。
    case REAL_LITERAL:
      TERMINAL(REAL_LITERAL, getToken(TOKEN), {
        INDENT++;
        PRINT_INDENT;
        fprintf(OUT, "%s\n", LEX->literal);
        INDENT--;
      });
      break;
    case '(':
      MATCH('(', TOKEN = nextToken(LEX););
      PARSE(Expression);
      MATCH(')', TOKEN = nextToken(LEX));
      break;
    // 分析实际参数。
    case IDENTIFIER:
      PARSE(Id);
      if (EQUAL('(')) {
        MATCH('(', TOKEN = nextToken(LEX););
        PARSE(ActualParams);
        MATCH(')', TOKEN = nextToken(LEX););
      }
    default:
      break;
  }
}
// 分析实际参数。

void ActualParams(ParserType* parser) {
#define SAME_CODE              \
  do {                         \
    PARSE(Expression);         \
    while (EQUAL(',')) {          \
      MATCH(',', TOKEN = nextToken(LEX);); \
      PARSE(Expression);       \
    }                          \
  } while (0)
  switch (TOKEN) {
    case INT_LITERAL:
      TERMINAL(INT_LITERAL, getToken(TOKEN), {
        INDENT++;
        PRINT_INDENT;
        fprintf(OUT, "%s\n", LEX->literal);
        INDENT--;
      });
      SAME_CODE;
      break;
    case REAL_LITERAL:
      TERMINAL(REAL_LITERAL, getToken(TOKEN), {
        INDENT++;
        PRINT_INDENT;
        fprintf(OUT, "%s\n", LEX->literal);
        INDENT--;
      });
      SAME_CODE;
      break;
    case '(':
      MATCH('(', TOKEN = nextToken(LEX););
      SAME_CODE;
      break;
    case IDENTIFIER:
      PARSE(Id);
      SAME_CODE;
      break;
    default:
      break;
  }
#undef SAME_CODE
}
// 分析返回语句。

void ReturnStmt(ParserType* parser) {
  MATCH(RETURN, TOKEN = nextToken(LEX););
  PARSE(Expression);
  MATCH(';', TOKEN = nextToken(LEX););
}
// 分析 READ 语句。

void ReadStmt(ParserType* parser) {
  MATCH(READ, TOKEN = nextToken(LEX););
  MATCH('(', TOKEN = nextToken(LEX););
  PARSE(Id);
  MATCH(',', TOKEN = nextToken(LEX););
  TERMINAL(STRING_LITERAL, getToken(TOKEN), {
    INDENT++;
    PRINT_INDENT;
    fprintf(OUT, "%s\n", LEX->literal);
    INDENT--;
  });
  MATCH(')', TOKEN = nextToken(LEX););
  MATCH(';', TOKEN = nextToken(LEX););
}
// 分析 WRITE 语句。

void WriteStmt(ParserType* parser) {
  MATCH(WRITE, TOKEN = nextToken(LEX););
  MATCH('(', TOKEN = nextToken(LEX););
  PARSE(Expression);
  MATCH(',', TOKEN = nextToken(LEX););
  TERMINAL(STRING_LITERAL, getToken(TOKEN), {
    INDENT++;
    PRINT_INDENT;
    fprintf(OUT, "%s\n", LEX->literal);
    INDENT--;
  });
  MATCH(')', TOKEN = nextToken(LEX););
  MATCH(';', TOKEN = nextToken(LEX););
}
// 分析 IF 语句。

void IfStmt(ParserType* parser) {
  TERMINAL(IF, getToken(TOKEN), {});
  MATCH('(', TOKEN = nextToken(LEX););
  INDENT++;
  PARSE(BoolExpression);
  MATCH(')', TOKEN = nextToken(LEX););
  PARSE(Statement);
  INDENT--;
  if (EQUAL(ELSE)) {
    TERMINAL(ELSE, getToken(TOKEN), {});
    INDENT++;
    PARSE(Statement);
    INDENT--;
  }
}

// 分析 Bool 表达式。
void BoolExpression(ParserType* parser) {
  PARSE(Expression);
  if (EQUAL(EQ)) {
    TERMINAL(EQ, getToken(TOKEN), {});
  } else if (EQUAL(NE)) {
    MATCH(NE, {});
  } else {
    parse_error(parser, 2, EQ, NE);
  }
  PARSE(Expression);
}

