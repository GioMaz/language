#ifndef PARSER_H
#define PARSER_H

#include "vector.h"
#include "lexer.h"

typedef enum {
    UNARY,
    BINARY,
    GROUPING,
    TERMINAL,
} ExprType;

typedef union anyexpr AnyExpr;

// Expr is a wrapper around AnyExpr that
// also specifies the expression type in
// the ExprType parameter
typedef struct expr {
    ExprType type;
    AnyExpr *as;
} Expr;

typedef struct {
    Token op;
    Expr expr;
} UnExpr;

typedef struct {
    Expr lexpr;
    Token op;
    Expr rexpr;
} BinExpr;

typedef struct {
    Expr expr;
} GroupExpr;

typedef struct {
    Token term;
} TermExpr;

// Single dinamically allocated struct
typedef union anyexpr {
    UnExpr unexpr;
    BinExpr binexpr;
    GroupExpr groupexpr;
    TermExpr termexpr;
} AnyExpr;

typedef struct {
    size_t pos;
    size_t size;
    Token *tokens;
} Parser;

void parser_init(Parser *p, Lexer *l);
void parser_free(Parser *p);
void print_expr(Expr expr);
Expr make_unexpr(Token op, Expr expr);
Expr make_binexpr(Expr lexpr, Token op, Expr rexpr);
Expr make_groupexpr(Expr expr);
Expr make_termexpr(Token term);
bool is_token(Parser *p, TokenType tt);
void sync(Parser *p);
Expr parse_terminal(Parser *p);
Expr parse_unary(Parser *p);
Expr parse_factor(Parser *p);
Expr parse_term(Parser *p);
Expr parse_comparison(Parser *p);
Expr parse_equality(Parser *p);
Expr parse_logicaland(Parser *p);
Expr parse_logicalor(Parser *p);
Expr parse_assignment(Parser *p);
Expr parse_expr(Parser *p);
void expr_free(Expr expr);

typedef enum {
    S_LET,
    S_IF,
    S_FOR,
    S_WHILE,
    S_BLOCK,
    S_EXPR,
    S_FUNC,
    S_RET,
} StmtType;

typedef union anystmt AnyStmt;

typedef struct {
    StmtType type;
    AnyStmt *as;
} Stmt;

typedef struct {
    Token name;
    Expr value;
} LetStmt;

typedef struct {
    Expr cond;
    Stmt thenb;
    Stmt elseb;
} IfStmt;

typedef struct {
    Expr init;
    Expr cond;
    Expr step;
    Stmt thenb;
} ForStmt;

typedef struct {
    Expr cond;
    Stmt thenb;
} WhileStmt;

typedef struct {
    size_t size;
    size_t capacity;
    Stmt *items;
} Block;

typedef struct {
    Block block;
} BlockStmt;

typedef struct {
    size_t size;
    size_t capacity;
    Token *items;
} Args;

typedef struct {
    Expr expr;
} ExprStmt;

typedef struct {
    Token name;
    Args args;
    Block block;
} FuncStmt;

typedef struct {
    Expr expr;
} RetStmt;

// Single dinamically allocated struct
typedef union anystmt {
    LetStmt letstmt;
    IfStmt ifstmt;
    ForStmt forstmt;
    WhileStmt whilestmt;
    BlockStmt blockstmt;
    ExprStmt exprstmt;
    FuncStmt funcstmt;
    RetStmt retstmt;
} AnyStmt;

Stmt make_letstmt(Token name, Expr value);
Stmt make_ifstmt(Expr cond, Stmt thenb, Stmt elseb);
Stmt make_forstmt(Expr init, Expr step, Expr cond, Stmt thenb);
Stmt make_whilestmt(Expr cond, Stmt thenb);
Stmt make_blockstmt(Block block);
Stmt make_exprstmt(Expr expr);
Stmt make_retstmt(Expr expr);
Stmt parse_retstmt(Parser *p);
Stmt parse_exprstmt(Parser *p);
Stmt parse_blockstmt(Parser *p);
Stmt parse_whilestmt(Parser *p);
Stmt parse_forstmt(Parser *p);
Stmt parse_ifstmt(Parser *p);
Stmt parse_declstmt(Parser *p);
Stmt parse_stmt(Parser *p);

typedef struct {
    size_t size;
    size_t capacity;
    Stmt *items;
} Program;

Program parse_program(Parser *p);
void print_stmt(Stmt stmt);
void stmt_free(Stmt stmt);
void print_program(Program *p);
void program_free(Program *p);

#endif
