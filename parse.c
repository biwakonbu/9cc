#include "9cc.h"

int pos = 0;
int ident_num = 0;

Map *ident_map;

int is_alnum(char c) {
  return ('a' <= c && c <= 'z') ||
    ('A' <= c && c <= 'Z') ||
    ('0' <= c && c <= '9') ||
    (c == '_');
}

Node *new_node(int ty, Node *lhs, Node *rhs) {
  Node *node = malloc(sizeof(Node));
  node->ty = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

Node *new_node_ident(char *name) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_IDENT;
  node->name = name;
  return node;
}

int consume(int ty) {
  if (tokens[pos].ty != ty)
    return 0;
  pos++;
  return 1;
}

Node *equality() {
  Node *node = relational();
  if (consume(TK_EQ))
    node = new_node(TK_EQ, node, relational());
  else if (consume(TK_NE))
    node = new_node(TK_NE, node, relational());
  return node;
}

Node *relational() {
  Node *node = add();
  if (consume(TK_LE))
    node = new_node(TK_LE, node, relational());
  else if (consume('<'))
    node = new_node('<', node, relational());
  else if (consume(TK_GE))
    node = new_node(TK_LE, relational(), node);
  else if (consume('>'))
    node = new_node('<', relational(), node);
  return node;
}

Node *term() {
  // 次のトークンが'('なら、"(" equality ")"のはず
  if (consume('(')) {
    Node *node = assign();
    if (!consume(')'))
      error("開きカッコに対応する閉じカッコがありません: %s",
            tokens[pos].input);
    return node;
  }

  // そうでなければ数値のはず
  if (tokens[pos].ty == TK_NUM)
    return new_node_num(tokens[pos++].val);

  if (tokens[pos].ty == TK_IDENT)
    return new_node_ident(tokens[pos++].name);

  error("数値でも開きカッコでもないトークンです: %s",
        tokens[pos].input);
  return NULL;
}

Node *unary() {
  if (consume('+'))
    return term();
  if (consume('-'))
    return new_node('-', new_node_num(0), term());
  return term();
}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume('*'))
      node = new_node('*', node, unary());
    else if (consume('/'))
      node = new_node('/', node, unary());
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume('+'))
      node = new_node('+', node, mul());
    else if (consume('-'))
      node = new_node('-', node, mul());
    else
      return node;
  }
}

Node *assign() {
  Node *node = equality();

  while(consume('='))
    node = new_node('=', node, assign());
  return node;
}

Node *stmt() {
  Node *node;

  if (consume(TK_RETURN)) {
    node = malloc(sizeof(Node));
    node->ty = ND_RETURN;
    node->lhs = assign();
  } else {
    node = assign();
  }

  if (!consume(';'))
    error("';'ではないトークンです: %s", tokens[pos].input);
  return node;
}

void program() {
  int i = 0;
  while (tokens[pos].ty != TK_EOF)
    code[i++] = stmt();
  code[i] = NULL;
}

// pが指している文字列をトークンに分割してtokensに保存する
void tokenize(char *p) {
  int i = 0;
  ident_map = new_map();
  while (*p) {
    // 空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
      tokens[i].ty = TK_RETURN;
      tokens[i].input = p;
      i++;
      p += 6;
      continue;
    }

    if ('a' <= *p && *p <= 'z') {
      int char_i = 0;
      tokens[i].ty = TK_IDENT;
      while (is_alnum(*(p + char_i)))
        char_i++;
      tokens[i].name = strndup(p, char_i);
      tokens[i].input = p;
      map_put(ident_map, tokens[i].name, (void *)ident_num);
      i++;
      p += char_i;
      ident_num++;
      continue;
    }

    if (strncmp(p, "==", 2) == 0) {
      tokens[i].ty = TK_EQ;
      tokens[i].input = p;
      i++;
      p += 2;
      continue;
    } else if (strncmp(p, "!=", 2) == 0) {
      tokens[i].ty = TK_NE;
      tokens[i].input = p;
      i++;
      p += 2;
      continue;
    } else if (strncmp(p, "<=", 2) == 0) {
      tokens[i].ty = TK_LE;
      tokens[i].input = p;
      i++;
      p += 2;
      continue;
    } else if (strncmp(p, ">=", 2) == 0) {
      tokens[i].ty = TK_GE;
      tokens[i].input = p;
      i++;
      p += 2;
      continue;
    }

    if (strchr("+-*/<>()=;", *p)) {
      tokens[i].ty = *p;
      tokens[i].input = p;
      i++;
      p++;
      continue;
    }

    if (isdigit(*p)) {
      tokens[i].ty = TK_NUM;
      tokens[i].input = p;
      tokens[i].val = strtol(p, &p, 10);
      i++;
      continue;
    }

    error("トークナイズできません: %s", p);
    exit(1);
  }

  tokens[i].ty = TK_EOF;
  tokens[i].input = p;
}

