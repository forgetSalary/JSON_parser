const char *false_keyword;
const char *true_keyword;
const char *null_keyword;

const char *first_keyword;
const char *last_keyword;
const char **keywords;



typedef enum TokenKind {
    TOKEN_EOF,
    TOKEN_INT = 128,
    TOKEN_FLOAT,
    TOKEN_STR,
    TOKEN_NAME,
    TOKEN_KEYWORD,
    // ...
} TokenKind;

typedef enum TokenMod {
    TOKENMOD_NONE,
    TOKENMOD_HEX,
    TOKENMOD_BIN,
    TOKENMOD_OCT,
    TOKENMOD_CHAR,
} TokenMod;

typedef struct Token {
    TokenKind kind;
    TokenMod mod;
    const char *start;
    const char *end;
    union {
        int32_t int_val;
        const char* str_val;
        double float_val;
        const char* name;
    };
} Token;



#define KEYWORD(name) name##_keyword = str_intern(#name); buf_push(keywords, name##_keyword)

void init_keywords() {
    static bool inited;
    if (inited) {
        return;
    }
    char *arena_end = str_arena.end;
    KEYWORD(false);
    KEYWORD(true);
    KEYWORD(null);
    first_keyword = false_keyword;
    last_keyword = null_keyword;
    inited = true;
}

#undef KEYWORD

bool is_keyword_str(const char *str) {
    return first_keyword <= str && str <= last_keyword;
}


Token token;
const char *stream;

void fatal(const char* fmt,...){
    va_list args;
    va_start(args,fmt);
    printf("FATAL: ");
    vprintf(fmt,args);
    va_end(args);
    exit(1);
}

const char* token_kind_names[] = {
        [TOKEN_INT] = "number",
        [TOKEN_FLOAT] = "number",
        [TOKEN_KEYWORD] = "keyword",
        [TOKEN_NAME] = "name"
};

const char* token_kind_name(TokenKind kind) {
    if (kind < sizeof(token_kind_names)/sizeof(*token_kind_names)){
        return token_kind_names[kind];
    }else{
        return NULL;
    }
}

uint8_t char_to_digit[256] = {
        ['0'] = 0,
        ['1'] = 1,
        ['2'] = 2,
        ['3'] = 3,
        ['4'] = 4,
        ['5'] = 5,
        ['6'] = 6,
        ['7'] = 7,
        ['8'] = 8,
        ['9'] = 9,
        ['a'] = 10, ['A'] = 10,
        ['b'] = 11, ['B'] = 11,
        ['c'] = 12, ['C'] = 12,
        ['d'] = 13, ['D'] = 13,
        ['e'] = 14, ['E'] = 14,
        ['f'] = 15, ['F'] = 15,
};

void scan_int() {
    uint64_t base = 10;
    if (*stream == '0') {
        stream++;
        if (tolower(*stream) == 'x') {
            stream++;
            token.mod = TOKENMOD_HEX;
            base = 16;
        } else if (tolower(*stream) == 'b') {
            stream++;
            token.mod = TOKENMOD_BIN;
            base = 2;
        } else if (isdigit(*stream)) {
            token.mod = TOKENMOD_OCT;
            base = 8;
        }
    }
    uint64_t val = 0;
    for (;;) {
        uint64_t digit = char_to_digit[*stream];
        if (digit == 0 && *stream != '0') {
            break;
        }
        if (digit >= base) {
            fatal("Digit '%c' out of range for base %", *stream, base);
            digit = 0;
        }
        if (val > (UINT64_MAX - digit)/base) {
            fatal("Integer literal overflow");
            while (isdigit(*stream)) {
                stream++;
            }
            val = 0;
            break;
        }
        val = val*base + digit;
        stream++;
    }
    token.kind = TOKEN_INT;
    token.int_val = val;
}

void scan_float() {
    const char *start = stream;
    while (isdigit(*stream)) {
        stream++;
    }
    if (*stream == '.') {
        stream++;
    }
    while (isdigit(*stream)) {
        stream++;
    }
    if (tolower(*stream) == 'e') {
        stream++;
        if (*stream == '+' || *stream == '-') {
            stream++;
        }
        if (!isdigit(*stream)) {
            fatal("Expected digit after float literal exponent, found '%c'.", *stream);
        }
        while (isdigit(*stream)) {
            stream++;
        }
    }
    double val = strtod(start, NULL);
    if (val == HUGE_VAL || val == -HUGE_VAL) {
        fatal("Expected digit after float literal exponent, found '%c'.", *stream);
    }
    token.kind = TOKEN_FLOAT;
    token.float_val = val;
}

char escape_to_char[256] = {
        ['n'] = '\n',
        ['r'] = '\r',
        ['t'] = '\t',
        ['v'] = '\v',
        ['b'] = '\b',
        ['a'] = '\a',
        ['0'] = 0,
};

void scan_str(){
    assert(*stream == '"');
    stream++;
    char* str = NULL;
    while(*stream && *stream !='"'){
        char val = *stream;
        if (val == '\n'){
            fatal("String literal cannot contain newline");
        }else if (val == '\\'){
            stream++;
            val = escape_to_char[*stream];
            if (val == 0 && *stream != '0'){
                fatal("Invalid string literal escape '\\%c'",*stream);
            }
        }
        buf_push(str,val);
        stream++;
    }
    if (*stream){
        assert(*stream == '"');
        stream++;
    }else{
        fatal("Unexpected end of file within string literal");
    }
    buf_push(str,0);
    token.kind = TOKEN_STR;
    token.str_val = str;
}

void next_token() {
    begin:
    switch (*stream) {
        case '-':
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': {
            bool negative;
            if (*stream == '-'){
                negative = true;
                stream++;
            }
            else negative = false;

            char* num_start = stream;
            while (isdigit(*stream)) {
                stream++;
            }
            char c = *stream;
            stream = num_start;
            if (c == '.' || tolower(c) == 'e') {
                scan_float();
                token.float_val = negative ? (-token.float_val) : token.float_val;
            } else {
                scan_int();
                token.int_val = negative ? (-token.int_val) : token.int_val;
            }

            break;
        }
        case 'a': case 'b': case 'c': case 'd':case 'e': case 'f':case 'g': case 'h':case 'i':case 'j':
        case 'k': case 'l': case 'm': case 'n':case 'o': case 'p':case 'q': case 'r':case 's':case 't':
        case 'u': case 'v': case 'w': case 'x':case 'y': case 'z':
        case 'A': case 'B': case 'C': case 'D':case 'E': case 'F':case 'G': case 'H':case 'I':case 'J':
        case 'K': case 'L': case 'M': case 'N':case 'O': case 'P':case 'Q': case 'R': case 'S': case 'T':
        case 'U': case 'V': case 'W': case 'X':case 'Y': case 'Z':
        case '_': {
            token.start = stream;
            while (isalnum(*stream) || *stream == '_') {
                stream++;
            }
            token.kind = TOKEN_NAME;
            token.name = str_intern_range(token.start,stream);
            break;
        }
        case ' ':case '\r':case '\v':case '\b':case '\a':case '\n':case '\t':{
            stream++;
            while (*stream == ' ' || *stream == '\n' || *stream == '\t') {
                stream++;
            }
            goto begin;
        }
        case '"':
            scan_str();
            break;
        case '\0':
            token.kind = TOKEN_EOF;
        default:
            token.kind = *stream++;
    }
    token.end = stream;
}

void init_stream(const char *str) {
    stream = str;
    next_token();
}

void print_token(Token token) {
    switch (token.kind) {
        case TOKEN_INT:
            printf("TOKEN NUMBER: %d ", token.int_val);
            break;
        case TOKEN_FLOAT:
            printf("TOKEN NUMBER: %f ", token.float_val);
            break;
        case TOKEN_NAME:
            if (is_keyword_str(token.name)){
                printf("TOKEN KEYWORD: %s ", token.name);
            }else{
                printf("TOKEN NAME: %s ", token.name);
            }
            break;
        case TOKEN_STR:
            printf("TOKEN STR: \"%s\" ", token.str_val);
            break;
        default:
            printf("TOKEN '%c' ", token.kind);
            break;
    }
    if(token.kind == ',' || token.kind == '{' || token.kind == '}') printf("\n");
}

inline bool is_token(TokenKind kind){
    return token.kind == kind;
}

inline bool is_token_name(const char* name){
    return token.kind = TOKEN_NAME && token.name == name;
}

inline bool match_token(TokenKind kind){
    if(is_token(kind)){
        next_token();
        return true;
    } else{
        return false;
    }
}

inline bool expect_token(TokenKind kind){
    if(is_token(kind)){
        next_token();
        return true;
    } else{
        fatal("expected token %s, got %s",token_kind_name(token.kind)),token_kind_name(token.kind);
        return false;
    }
}
