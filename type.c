#include "parse.h"

Type *new_type(TypeKind kind)
{
    Type *type = calloc(1, sizeof(type));
    type->kind = kind;
    return type;
}
