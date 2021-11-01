#include "10cc.h"

bool is_keyword(char *str);
void new_token(TokenKind kind, char *str, int len);
int is_alnum(char c);

extern char *user_input_pos;
