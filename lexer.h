#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>
#include <stdio.h>

typedef struct {
    size_t size;
    size_t capacity;
    char *items;
} Buffer;

typedef enum {
    T_LPAREN,       // (
    T_RPAREN,       // )
    T_LSBRACE,      // [
    T_RSBRACE,      // ]
    T_LBRACE,       // {
    T_RBRACE,       // }
    T_COMMA,        // ,
    T_SEMICOLON,    // ;
    T_DOT,          // .
    T_COLON,        // :
    T_PLUS,         // +
    T_MINUS,        // -
    T_STAR,         // *
    T_SLASH,        // /
    T_EQUAL,        // =
    T_2EQUAL,       // ==
    T_BANG,         // !
    T_BANG_EQUAL,   // !=
    T_LESS,         // <
    T_2LESS,        // <<
    T_GREATER,      // >
    T_2GREATER,     // >>
    T_EOF,          // EOF
    T_STRING,       // "..."
    T_DOUBLE,       // 1.2
    T_NAME,         // counter
    T_LET,          // let
    T_IF,           // if
    T_ELSE,         // else
    T_FOR,          // for
    T_WHILE,        // while
    T_OR,           // or
    T_AND,          // and
    T_TRUE,         // true
    T_FALSE,        // false
    T_FN,           // fn
    T_RETURN,       // return
} TokenType;

typedef struct {
    TokenType type;
    void *data;
    size_t line;
} Token;

typedef struct {
    size_t size;
    size_t capacity;
    Token *items;
    char *content;
    size_t pos;
    size_t line;
} Lexer;

void get_content(FILE *f, Buffer *buffer);
void lexer_init(Lexer *l, char *content);
void token_free(Token t);
void lexer_free(Lexer *l);
char peek(Lexer *l);
char get_cur(Lexer *l);
bool is_next(Lexer *l, char c);
bool is_digit(Lexer *l);
bool is_alpha(Lexer *l);
bool is_alphanumeric(Lexer *l);
bool is_keyword(char *name);
Token make_token(TokenType tt);
char *get_string(Lexer *l);
Token make_string(char *string);
double *get_double(Lexer *l);
Token make_double(double *number);
char *get_name(Lexer *l);
Token make_name(char *string);
bool get_token(Lexer *l);
void get_tokens(Lexer *l);
double get_ddata(Token t);
void print_token(Token t);
void print_tokens(Lexer *l);

#endif
