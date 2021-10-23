#include"10cc.h"

void clear_semicolon()
{
    while (consume(TK_RESERVED, ";"))
        ;
}