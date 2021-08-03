#include "includes.h"
#include "common.h"
#include "common.c"
#include "lex.c"
#include "JSON.c"
#include "JSON_print.c"
#include "JSON_parse.c"

void main_test(){
    //lex_test();
    json_print_test();
    //json_parse_test();
}

int main() {
    main_test();
    return 0;
}
