#include "10cc.h"

void new_token(TokenKind kind, char *str, int len);
int is_alnum(char c);
bool is_keyword(char *p, char *str, int num);

extern char *user_input_pos;
