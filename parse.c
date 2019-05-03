#include "9cc.h"

int pos = 0;

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
    Node *node = equality();
    if (!consume(')'))
      error("開きカッコに対応する閉じカッコがありません: %s",
            tokens[pos].input);
    return node;
  }

  // そうでなければ数値のはず
  if (tokens[pos].ty == TK_NUM)
    return new_node_num(tokens[pos++].val);

  error("数値でも開きカッコでもないトークンです: %s",
        tokens[pos].input);
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

// pが指している文字列をトークンに分割してtokensに保存する
void tokenize(char *p) {
  int i = 0;
  while (*p) {
    // 空白文字をスキップ
    if (isspace(*p)) {
      p++;
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

    if (strchr("+-*/<>()", *p)) {
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

