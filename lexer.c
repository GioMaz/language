#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "vector.h"
#include "lexer.h"

static char *keywords[] = {
    "let",
    "if",
    "else",
    "for",
    "while",
    "and",
    "or",
    "true",
    "false",
    "fn",
    "return",
};

static const size_t keywords_size = (sizeof(keywords) / sizeof(char *));

void get_content(FILE *f, Buffer *buffer)
{
    char c;
    do {
        c = fgetc(f);
        v_append(*buffer, c);
    } while (c != EOF);
}

void token_free(Token t)
{
    if (t.data) {
        free(t.data);
    }
}

void lexer_free(Lexer *l)
{
    for (size_t i = 0; i < l->size; i++) {
        token_free(l->items[i]);
    }
    free(l->items);
}

void lexer_init(Lexer *l, char *content)
{
    v_init(*l);
    l->content = content;
    l->pos = 0;
    l->line = 1;
}

char peek(Lexer *l)
{
    return l->content[l->pos+1];
}

char get_cur(Lexer *l)
{
    return l->content[l->pos];
}

bool is_next(Lexer *l, char c)
{
    if (peek(l) == c) {
        l->pos++;
        return true;
    } else {
        return false;
    }
}

bool is_digit(Lexer *l)
{
    char c = get_cur(l);
    return c >= '0' && c <= '9';
}

bool is_alpha(Lexer *l)
{
    char c = get_cur(l);
    return ((c >= 'a' && c <= 'z')
            || (c >= 'A' && c <= 'Z')
            || c == '_');
}

bool is_alphanumeric(Lexer *l)
{
    return is_digit(l) || is_alpha(l);
}

bool is_keyword(char *name)
{
    for (size_t i = 0; i < keywords_size; i++) {
        if (strcmp(keywords[i], name) == 0) {
            return true;
        }
    }
    return false;
}

TokenType get_keyword(char *name)
{
    if (strcmp(name, "let") == 0) {
        return T_LET;
    } else if (strcmp(name, "if") == 0) {
        return T_IF;
    } else if (strcmp(name, "else") == 0) {
        return T_ELSE;
    } else if (strcmp(name, "for") == 0) {
        return T_FOR;
    } else if (strcmp(name, "while") == 0) {
        return T_WHILE;
    } else if (strcmp(name, "or") == 0) {
        return T_OR;
    } else if (strcmp(name, "and") == 0) {
        return T_AND;
    } else if (strcmp(name, "true") == 0) {
        return T_TRUE;
    } else if (strcmp(name, "false") == 0) {
        return T_FALSE;
    } else if (strcmp(name, "fn") == 0) {
        return T_FN;
    } else if (strcmp(name, "return") == 0) {
        return T_RETURN;
    } else { // Unexpected
        printf("No keyword enum matches string\n");
        exit(1);
    }
}

Token make_token(TokenType tt)
{
    return (Token) {
        .type = tt,
    };
}

char *get_string(Lexer *l)
{
    size_t start = ++l->pos;
    while (get_cur(l) != '"' && get_cur(l) != EOF) {
        if (get_cur(l) == '\\') {
            l->pos += 2;
        } else {
            l->pos += 1;
        }
    }

    if (peek(l) == EOF) {
        return NULL;
    } else {
        size_t size = l->pos - start;
        char *string = malloc(size + 1);
        memcpy(string, l->content + start, size);
        string[size] = 0;
        return string;
    }
}

Token make_string(char *string)
{
    return (Token) {
        .type = T_STRING,
        .data = string
    };
}

double *get_double(Lexer *l)
{
    size_t start = l->pos;
    while ((is_digit(l) || get_cur(l) == '.')
            && get_cur(l) != EOF) {
        l->pos++;
    }

    double *data = malloc(sizeof(double));
    char *ptr;
    double number = strtod(l->content + start, &ptr);
    memcpy(data, &number, sizeof(number));
    l->pos = ptr - l->content - 1;
    return data;
}

Token make_double(double *number)
{
    return (Token) {
        .type = T_DOUBLE,
        .data = number,
    };
}

char *get_name(Lexer *l)
{
    size_t start = l->pos;
    while (is_alphanumeric(l)) {
        l->pos++;
    }

    size_t size = l->pos - start;
    char *name = malloc(l->pos - start + 1);
    memcpy(name, l->content + start, size);
    name[size] = 0;
    l->pos--;
    return name;
}

Token make_name(char *name)
{
    return (Token) {
        .type = T_NAME,
        .data = name,
    };
}

void skip_comment(Lexer *l)
{
    while (get_cur(l) != '\n') {
        l->pos++;
    }
    l->line++;
}

bool get_token(Lexer *l)
{
    Token t = {
        .line = l->line,
    };
    switch (get_cur(l)) {

    // Handle signle character tokens
    case '(': t.type = T_LPAREN; break;
    case ')': t.type = T_RPAREN; break;
    case ']': t.type = T_LSBRACE; break;
    case '[': t.type = T_RSBRACE; break;
    case '{': t.type = T_LBRACE; break;
    case '}': t.type = T_RBRACE; break;
    case ',': t.type = T_COMMA; break;
    case ';': t.type = T_SEMICOLON; break;
    case '.': t.type = T_DOT; break;
    case ':': t.type = T_COLON; break;
    case '+': t.type = T_PLUS; break;
    case '-': t.type = T_MINUS; break;
    case '*': t.type = T_STAR; break;

    // Handle double character operators
    case '/':
        if (is_next(l, '/')) {
            skip_comment(l);
            return true;
        } else {
            t.type = T_SLASH;
        }
        break;
    case '=':
        t.type = is_next(l, '=')
                 ? T_2EQUAL
                 : T_EQUAL;
        break;
    case '!':
        t.type = is_next(l, '=')
                 ? T_BANG_EQUAL
                 : T_BANG;
        break;
    case '<':
        t.type = is_next(l, '<')
                 ? T_2LESS
                 : T_LESS;
        break;
    case '>':
        t.type = is_next(l, '<')
                 ? T_2GREATER
                 : T_GREATER;
        break;

    // Handle white spaces
    case ' ':
    case '\r':
    case '\t':
        return true;
    case '\n':
        l->line++;
        return true;

    // Handle string literals
    case '"': {
        char *string = get_string(l);
        if (string == NULL) {
            printf("Unterminated string at line %zu\n", l->line);
            exit(1);
        } else {
            t.type = T_STRING;
            t.data = string;
        }
        break;
    }

    // Handle end of file
    case EOF:
        t.type = T_EOF;
        v_append(*l, t);
        return false;

    // Handle unexpected token
    default:
        if (is_digit(l)) {
            double *number = get_double(l);
            t.type = T_DOUBLE;
            t.data = number;
        } else if (is_alpha(l)) {
            char *name = get_name(l);
            t.type = is_keyword(name)
                ? get_keyword(name)
                : T_NAME;
            t.data = name;
        } else {
            printf("Unexpected token `%c` at line %zu\n",
                    get_cur(l), l->line + 1);
            return false;
        }
        break;
    }

    v_append(*l, t);
    return true;
}

void get_tokens(Lexer *l)
{
    while (get_token(l)) {
        l->pos++;
    }
}

double get_ddata(Token t)
{
    return ((double *)t.data)[0];
}

void print_token(Token t)
{
    switch (t.type) {
        case T_LPAREN:printf("("); break;
        case T_RPAREN:printf(")"); break;
        case T_LSBRACE:printf("["); break;
        case T_RSBRACE:printf("]"); break;
        case T_LBRACE:printf("{"); break;
        case T_RBRACE:printf("}"); break;
        case T_COMMA:printf(","); break;
        case T_SEMICOLON:printf(";"); break;
        case T_DOT:printf("."); break;
        case T_COLON:printf(":"); break;
        case T_PLUS:printf("+"); break;
        case T_MINUS:printf("-"); break;
        case T_STAR:printf("*"); break;
        case T_SLASH:printf("/"); break;
        case T_EQUAL:printf("="); break;
        case T_2EQUAL:printf("=="); break;
        case T_BANG:printf("!"); break;
        case T_BANG_EQUAL:printf("!="); break;
        case T_LESS:printf("<"); break;
        case T_2LESS:printf("<<"); break;
        case T_GREATER:printf(">"); break;
        case T_2GREATER:printf(">>"); break;
        case T_EOF:printf("EOF"); break;
        case T_STRING:
            printf("\"%s\"", (char *)t.data);
            break;
        case T_DOUBLE:
            printf("$%f$", *((double *)t.data));
            break;
        case T_NAME:
            printf("%%%s%%", (char *)t.data);
            break;
        case T_LET:printf("let");break;
        case T_IF:printf("if");break;
        case T_ELSE:printf("else");break;
        case T_FOR:printf("for");break;
        case T_WHILE:printf("while");break;
        case T_OR:printf("or");break;
        case T_AND:printf("and");break;
        case T_TRUE:printf("true");break;
        case T_FALSE:printf("false");break;
        case T_FN:printf("fn");break;
        case T_RETURN:printf("return");break;
        default:
            printf("unk");
            break;
    }
}

void print_tokens(Lexer *l)
{
    for (size_t i = 0; i < l->size; i++) {
        print_token(l->items[i]);
    }
}
