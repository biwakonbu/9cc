#include <stdarg.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  void **data;
  int capacity;
  int len;
} Vector;

Vector *new_vector();
void vec_push(Vector *vec, void *elem);

// test
void expect(int line, int expected, int actual);
void runtest();
void test_map();

// トークンの型を表す値
enum {
  TK_NUM = 256, // 整数トークン
  TK_IDENT, // 識別子
  TK_RETURN, // return
  TK_EQ, // ==
  TK_NE, // !=
  TK_LE, // <=
  TK_GE, // >=
  TK_EOF, // 入力の終わりを表すトークン
};

// トークンの型
typedef struct {
  int ty; // トークンの型
  int val; // tyがTK_NUMの場合、その数値
  char name; // tyがTK_IDENTの場合、その名前
  char *input; // トークン文字列 (エラーメッセージ用)
} Token;

// トークナイズした結果のトークン列はこの配列に保存する
// 100個以上のトークンは来ないものとする
Token tokens[100];

enum {
  ND_NUM = 256, // 整数のノードの型
  ND_IDENT, // 識別子のノードの型
  ND_RETURN, // return
};

typedef struct Node {
  int ty; // 演算子かND_NUM
  struct Node *lhs; // 左辺
  struct Node *rhs; // 右辺
  int val; // tyがND_NUMの場合のみ使う
  char name; // tyがND_IDENTの場合のみ使う
} Node;

Node *code[100];

Node *add();
Node *mul();
Node *term();
Node *relational();
Node *equality();
Node *assign();
Node *stmt();
void program();
void tokenize(char *p);

void error(char *fmt, ...);

void gen(Node *node);

typedef struct {
  Vector *keys;
  Vector *vals;
} Map;

Map *new_map();
void map_put(Map *map, char *key, void *val);
void *map_get(Map *map, char *key);
