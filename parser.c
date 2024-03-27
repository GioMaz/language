#include <stdio.h>
#include <string.h>

#include "vector.h"
#include "parser.h"
#include "lexer.h"

void parser_init(Parser *p, Lexer *l)
{
    p->pos = 0;
    p->size = l->size;
    p->tokens = l->items;
}

void parser_free(Parser *p)
{
    for (size_t i = 0; i < p->size; i++) {
        token_free(p->tokens[i]);
    }
}

void print_expr(Expr expr)
{
    switch (expr.type) {
    case UNARY: {
        print_token(expr.as->unexpr.op);
        print_expr(expr.as->unexpr.expr);
        break;
    }
    case BINARY: {
        print_expr(expr.as->binexpr.lexpr);
        print_token(expr.as->binexpr.op);
        print_expr(expr.as->binexpr.rexpr);
        break;
    }
    case GROUPING: {
        printf("(");
        print_expr(expr.as->groupexpr.expr);
        printf(")");
        break;
    }
    case TERMINAL:
        print_token(expr.as->termexpr.term);
        break;
    default:
        printf("unk");
        break;
    }
}

Expr make_unexpr(Token op, Expr expr)
{
    UnExpr unexpr = {
        .op = op,
        .expr = expr,
    };

    AnyExpr *as = malloc(sizeof(AnyExpr));
    as->unexpr = unexpr;

    Expr expr_new = {
        .type = UNARY,
        .as = as,
    };

    return expr_new;
}

Expr make_binexpr(Expr lexpr, Token op, Expr rexpr)
{
    BinExpr binexpr = {
        .lexpr = lexpr,
        .op = op,
        .rexpr= rexpr,
    };

    AnyExpr *as = malloc(sizeof(AnyExpr));
    as->binexpr = binexpr;

    Expr expr = {
        .type = BINARY,
        .as = as,
    };

    return expr;
}

Expr make_groupexpr(Expr expr)
{
    GroupExpr groupexpr = {
        .expr = expr,
    };

    AnyExpr *as = malloc(sizeof(AnyExpr));
    as->groupexpr = groupexpr;

    Expr expr_new = {
        .type = GROUPING,
        .as = as,
    };

    return expr_new;
}

Expr make_termexpr(Token term)
{
    TermExpr termexpr = {
        .term = term,
    };

    AnyExpr *as = malloc(sizeof(AnyExpr));
    as->termexpr = termexpr;

    Expr expr = {
        .type = TERMINAL,
        .as = as,
    };

    return expr;
}

bool is_token(Parser *p, TokenType tt)
{
    return p->tokens[p->pos].type == tt;
}

void sync(Parser *p)
{
    while (p->pos < p->size) {
        switch (p->tokens[p->pos].type) {
        case T_IF:
        case T_ELSE:
        case T_FOR:
        case T_WHILE:
        case T_SEMICOLON:
            return;
        default:
            p->pos++;
        }
    }
}

Expr parse_expr(Parser *p);

Expr parse_terminal(Parser *p)
{
    Token terminal = p->tokens[p->pos];
    if (is_token(p, T_DOUBLE)
            || is_token(p, T_STRING)
            || is_token(p, T_NAME)
            || is_token(p, T_TRUE) || is_token(p, T_FALSE)) {
        Expr expr = make_termexpr(terminal);
        p->pos++;
        return expr;
    } else if (is_token(p, T_LPAREN)) {
        p->pos++;
        Expr expr = parse_expr(p);
        if (is_token(p, T_RPAREN)) {
            p->pos++;
            return make_groupexpr(expr);
        } else {
            printf("Expected ')' at line %zu\n", terminal.line);
            exit(1);
        }
    } else {
        printf("Unexpected token '");
        print_token(terminal);
        printf("' at line %zu\n", terminal.line);
        exit(1);
    }
}

Expr parse_unary(Parser *p)
{
    if (is_token(p, T_BANG)
            || is_token(p, T_MINUS)) {
        Token op = p->tokens[p->pos];
        p->pos++;

        Expr right = parse_terminal(p);

        Expr expr = make_unexpr(op, right);
        return expr;
    } else {
        Expr expr = parse_terminal(p);
        return expr;
    }
}

Expr parse_factor(Parser *p)
{
    Expr expr = parse_unary(p);

    while (is_token(p, T_STAR)
            || is_token(p, T_SLASH)) {
        Token op = p->tokens[p->pos];
        p->pos++;

        Expr right = parse_unary(p);

        expr = make_binexpr(expr, op, right);
    }

    return expr;
}

Expr parse_term(Parser *p)
{
    Expr expr = parse_factor(p);

    while (is_token(p, T_PLUS)
            || is_token(p, T_MINUS)) {
        Token op = p->tokens[p->pos];
        p->pos++;

        Expr right = parse_factor(p);

        expr = make_binexpr(expr, op, right);
    }

    return expr;
}

Expr parse_comparison(Parser *p)
{
    Expr expr = parse_term(p);

    while (is_token(p, T_LESS)
            || is_token(p, T_GREATER)) {
        Token op = p->tokens[p->pos];
        p->pos++;

        Expr right = parse_term(p);

        expr = make_binexpr(expr, op, right);
    }

    return expr;
}

Expr parse_equality(Parser *p)
{
    Expr expr = parse_comparison(p);

    while (is_token(p, T_2EQUAL)
            || is_token(p, T_BANG_EQUAL)) {
        Token op = p->tokens[p->pos];

        p->pos++;

        Expr right = parse_comparison(p);

        expr = make_binexpr(expr, op, right);
    }

    return expr;
}

Expr parse_logicaland(Parser *p)
{
    Expr expr = parse_equality(p);

    while (is_token(p, T_AND)) {
        Token op = p->tokens[p->pos];
        p->pos++;

        Expr right = parse_equality(p);

        expr = make_binexpr(expr, op, right);
    }

    return expr;
}

Expr parse_logicalor(Parser *p)
{
    Expr expr = parse_logicaland(p);

    while (is_token(p, T_OR)) {
        Token op = p->tokens[p->pos];
        p->pos++;

        Expr right = parse_logicaland(p);

        expr = make_binexpr(expr, op, right);
    }

    return expr;
}

Expr parse_assignment(Parser *p)
{
    Expr expr = parse_logicalor(p);

    while (is_token(p, T_EQUAL)) {
        Token op = p->tokens[p->pos];

        // Incrementing p->pos only when consuming operators
        // since parse_terminal(Parser *p) already consumes
        // p->pos after reaching the end of the recursive descent
        p->pos++;

        Expr right = parse_logicalor(p);

        expr = make_binexpr(expr, op, right);
    }

    return expr;
}

// The algorithm used for the parsing of expression
// is recursive descent and it's used to parse an
// expression specifying precedence rules (* before +)
// and associativity rules (left before right).
//
// The pseudocode for a recursive descent function is
// the following:
//
// Expr parse_expr1(Parser *p)
//   Expr expr = parse_expr2(p);
//   while (curtok(p) == OP_EXPR1) {
//     Token op = curtok(p);
//     p->pos++;
//     Expr right = parse_expr2(p);
//     expr = make_binexpr(expr, op, right);
//   }
//   return expr;
Expr parse_expr(Parser *p)
{
    return parse_assignment(p);
}

void expr_free(Expr expr)
{
    switch (expr.type) {
    case UNARY:
        expr_free(expr.as->unexpr.expr);
        break;
    case BINARY:
        expr_free(expr.as->binexpr.lexpr);
        expr_free(expr.as->binexpr.rexpr);
        break;
    case GROUPING:
        expr_free(expr.as->groupexpr.expr);
        break;
    case TERMINAL:
        break;
    }
    free(expr.as);
}

Stmt make_letstmt(Token name, Expr value)
{
    AnyStmt *as = malloc(sizeof(AnyStmt));
    as->letstmt.name = name;
    as->letstmt.value = value;

    Stmt stmt = {
        .type = S_LET,
        .as = as,
    };

    return stmt;
}

Stmt make_ifstmt(Expr cond, Stmt thenb, Stmt elseb)
{
    AnyStmt *as = malloc(sizeof(AnyStmt));
    as->ifstmt.cond = cond;
    as->ifstmt.thenb = thenb;
    as->ifstmt.elseb = elseb;

    Stmt stmt_new = {
        .type = S_IF,
        .as = as,
    };

    return stmt_new;
}

Stmt make_forstmt(Expr init, Expr cond, Expr step, Stmt thenb)
{
    AnyStmt *as = malloc(sizeof(AnyStmt));
    as->forstmt.init = init;
    as->forstmt.cond = cond;
    as->forstmt.step = step;
    as->forstmt.thenb = thenb;

    Stmt stmt_new = {
        .type = S_FOR,
        .as = as,
    };

    return stmt_new;
}

Stmt make_whilestmt(Expr cond, Stmt thenb)
{
    AnyStmt *as = malloc(sizeof(AnyStmt));
    as->whilestmt.cond = cond;
    as->whilestmt.thenb = thenb;

    Stmt stmt_new = {
        .type = S_WHILE,
        .as = as,
    };

    return stmt_new;
}

Stmt make_funcstmt(Token name, Args args, Block block)
{
    AnyStmt *as = malloc(sizeof(AnyStmt));
    as->funcstmt.name = name;
    as->funcstmt.args = args;
    as->funcstmt.block = block;

    Stmt stmt = {
        .type = S_FUNC,
        .as = as,
    };

    return stmt;
}

Stmt make_blockstmt(Block block)
{
    AnyStmt *as = malloc(sizeof(AnyStmt));
    as->blockstmt.block = block;

    Stmt stmt_new = {
        .type = S_BLOCK,
        .as = as,
    };

    return stmt_new;
}

Stmt make_exprstmt(Expr expr)
{
    AnyStmt *as = malloc(sizeof(AnyStmt));
    as->exprstmt.expr = expr;

    Stmt stmt_new = {
        .type = S_EXPR,
        .as = as,
    };

    return stmt_new;
}

Stmt make_retstmt(Expr expr)
{
    AnyStmt *as = malloc(sizeof(AnyStmt));
    as->retstmt.expr = expr;

    Stmt stmt_new = {
        .type = S_RET,
        .as = as,
    };

    return stmt_new;
}

Stmt parse_exprstmt(Parser *p)
{
    Expr expr = parse_expr(p);
    Stmt exprstmt = make_exprstmt(expr);
    if (!is_token(p, T_SEMICOLON)) {
        printf("Expected ';' at line %zu\n",
                p->tokens[p->pos].line);
        exit(1);
    }
    p->pos++;
    return exprstmt;
}

Stmt parse_retstmt(Parser *p)
{
    if (is_token(p, T_RETURN)) {
        p->pos++;
        Expr expr = parse_expr(p);
        Stmt retstmt = make_retstmt(expr);
        if (!is_token(p, T_SEMICOLON)) {
            printf("Expected ';' at line %zu\n",
                    p->tokens[p->pos].line);
            exit(1);
        }
        p->pos++;
        return retstmt;
    } else {
        Stmt exprstmt = parse_exprstmt(p);
        return exprstmt;
    }
}

Stmt parse_funcstmt(Parser *p)
{
    if (is_token(p, T_FN)) {
        // Parse name
        p->pos++;
        Token name = p->tokens[p->pos];

        // Parse arguments
        Args args;
        v_init(args);

        p->pos++;
        while (is_token(p, T_NAME)) {
            Token arg = p->tokens[p->pos];
            v_append(args, arg);
            p->pos++;
        }

        // Parse block
        Block block;
        v_init(block);

        p->pos++; // {
        while (!is_token(p, T_RBRACE)) {
            Stmt stmt = parse_stmt(p);
            v_append(block, stmt);
        }
        p->pos++; // }

        Stmt stmt = make_funcstmt(name, args, block);

        return stmt;
    } else {
        Stmt retstmt = parse_retstmt(p);
        return retstmt;
    }
}

Stmt parse_blockstmt(Parser *p)
{
    if (is_token(p, T_LBRACE)) {
        Block block;
        v_init(block);

        p->pos++; // {
        while (!is_token(p, T_RBRACE)) {
            Stmt stmt = parse_stmt(p);
            v_append(block, stmt);
        }
        p->pos++; // }

        Stmt blockstmt = make_blockstmt(block);
        return blockstmt;
    } else {
        Stmt funcstmt = parse_funcstmt(p);
        return funcstmt;
    }
}

Stmt parse_whilestmt(Parser *p)
{
    if (is_token(p, T_WHILE)) {
        p->pos++;
        Expr cond = parse_expr(p);

        Stmt thenb = parse_stmt(p);

        Stmt whilestmt = make_whilestmt(cond, thenb);
        return whilestmt;
    } else {
        Stmt blockstmt = parse_blockstmt(p);
        return blockstmt;
    }
}

Stmt parse_forstmt(Parser *p)
{
    if (is_token(p, T_FOR)) {
        p->pos++; 
        Expr init = parse_expr(p);

        p->pos++;
        Expr cond = parse_expr(p);

        p->pos++;
        Expr step = parse_expr(p);

        Stmt thenb = parse_stmt(p);

        Stmt forstmt = make_forstmt(init, cond, step, thenb);
        return forstmt;
    } else {
        Stmt whilestmt = parse_whilestmt(p);
        return whilestmt;
    }
}

Stmt parse_ifstmt(Parser *p)
{
    if (is_token(p, T_IF)) {
        p->pos++;
        Expr cond = parse_expr(p);

        Stmt thenb = parse_stmt(p);

        Stmt elseb;
        if (is_token(p, T_ELSE)) {
            p->pos++;
            elseb = parse_stmt(p);
        } else {
            Block block;
            v_init(block);
            elseb = make_blockstmt(block);
        }

        Stmt ifstmt = make_ifstmt(cond, thenb, elseb);
        return ifstmt;
    } else {
        Stmt whilestmt = parse_forstmt(p);
        return whilestmt;
    }
}

Stmt parse_declstmt(Parser *p)
{
    if (is_token(p, T_LET)) {
        p->pos++;
        Token name = p->tokens[p->pos];
        p->pos++;
        if (!is_token(p, T_EQUAL)) {
            printf("Expected '=' at line %zu\n",
                    p->tokens[p->pos].line);
            exit(1);
        }
        p->pos++;
        Expr value = parse_expr(p);

        if (!is_token(p, T_SEMICOLON)) {
            printf("Expected ';' at line %zu\n",
                    p->tokens[p->pos].line);
            exit(1);
        }
        p->pos++;

        Stmt letstmt = make_letstmt(name, value);
        return letstmt;
    } else {
        Stmt ifstmt = parse_ifstmt(p);
        return ifstmt;
    }
}

Stmt parse_stmt(Parser *p)
{
    Stmt stmt = parse_declstmt(p);
    return stmt;
}

Program parse_program(Parser *p)
{
    Program program;
    v_init(program);

    while (!is_token(p, T_EOF)) {
        Stmt stmt = parse_stmt(p);
        v_append(program, stmt);
    }

    return program;
}

void print_stmt(Stmt stmt)
{
    switch (stmt.type) {
    case S_LET:
        printf("let ");
        print_token(stmt.as->letstmt.name);
        printf(" = ");
        print_expr(stmt.as->letstmt.value);
        printf(";\n");
        break;
    case S_IF:
        printf("if ");
        print_expr(stmt.as->ifstmt.cond);
        printf(" ");
        print_stmt(stmt.as->ifstmt.thenb);
        printf("else ");
        print_stmt(stmt.as->ifstmt.elseb);
        break;
    case S_FOR:
        printf("for ");
        print_expr(stmt.as->forstmt.init);
        printf("; ");
        print_expr(stmt.as->forstmt.step);
        printf("; ");
        print_expr(stmt.as->forstmt.cond);
        printf(" ");
        print_stmt(stmt.as->forstmt.thenb);
        break;
    case S_WHILE:
        printf("while ");
        print_expr(stmt.as->whilestmt.cond);
        printf(" ");
        print_stmt(stmt.as->whilestmt.thenb);
        break;
    case S_BLOCK:
        printf("{\n");
        Block block = stmt.as->blockstmt.block;
        for (size_t i = 0; i < block.size; i++) {
            print_stmt(block.items[i]);
        }
        printf("}\n");
        break;
    case S_FUNC: {
        printf("fn");
        print_token(stmt.as->funcstmt.name);
        Args args = stmt.as->funcstmt.args;
        for (size_t i = 0; i < args.size; i++) {
            print_token(args.items[i]);
        }
        printf("{\n");
        Block block = stmt.as->funcstmt.block;
        for (size_t i = 0; i < block.size; i++) {
            print_stmt(block.items[i]);
        }
        printf("}\n");
        break;
    }
    case S_EXPR:
        print_expr(stmt.as->exprstmt.expr);
        printf(";\n");
        break;
    case S_RET:
        printf("return");
        print_expr(stmt.as->retstmt.expr);
    }
}

void stmt_free(Stmt stmt)
{
    switch (stmt.type) {
    case S_LET:
        expr_free(stmt.as->letstmt.value);
        break;
    case S_IF:
        expr_free(stmt.as->ifstmt.cond);
        stmt_free(stmt.as->ifstmt.thenb);
        stmt_free(stmt.as->ifstmt.elseb);
        break;
    case S_FOR:
        expr_free(stmt.as->forstmt.init);
        expr_free(stmt.as->forstmt.step);
        expr_free(stmt.as->forstmt.cond);
        stmt_free(stmt.as->forstmt.thenb);
        break;
    case S_WHILE:
        expr_free(stmt.as->whilestmt.cond);
        stmt_free(stmt.as->whilestmt.thenb);
        break;
    case S_BLOCK: {
        Block block = stmt.as->blockstmt.block;
        for (size_t i = 0; i < block.size; i++) {
            stmt_free(block.items[i]);
        }
        free(block.items);
        break;
    }
    case S_FUNC: {
        Block block = stmt.as->funcstmt.block;
        for (size_t i = 0; i < block.size; i++) {
            stmt_free(block.items[i]);
        }
        free(block.items);
        break;
    }
    case S_EXPR:
        expr_free(stmt.as->exprstmt.expr);
        break;
    case S_RET:
        expr_free(stmt.as->retstmt.expr);
        break;
    }
    free(stmt.as);
}

void print_program(Program *p)
{
    for (size_t i = 0; i < p->size; i++) {
        print_stmt(p->items[i]);
    }
}

void program_free(Program *p)
{
    for (size_t i = 0; i < p->size; i++) {
        stmt_free(p->items[i]);
    }
    free(p->items);
}

// int main(void)
// {
//     Buffer b;
//     v_init(b);
//     FILE *f = fopen("./code.l", "r");
//     get_content(f, &b);
// 
//     Lexer l;
//     lexer_init(&l, b.items);
//     get_tokens(&l);
//     // print_tokens(&l);
// 
//     Parser p;
//     parser_init(&p, &l);
//     Program pr = parse_program(&p);
//     print_program(&pr);
// 
//     program_free(&pr); // Free statements and expressions
//                        // (program)
//     lexer_free(&l); // Free tokens (lexer and parser)
//     free(b.items); // Free content buffer
// 
//     return 0;
// }
