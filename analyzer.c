#include <stdio.h>

#include "vector.h"
#include "lexer.h"
#include "parser.h"

int main(int argc, char **argv)
{
    Buffer b;
    v_init(b);
    FILE *f = fopen("code.l", "r");
    get_content(f, &b);

    return 0;
}
