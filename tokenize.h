#include "10cc.h"

bool is_keyword(char *str);
void new_token(TokenKind kind, int len);
int is_alnum(char c);

bool commentout();

extern char *user_input_pos;
