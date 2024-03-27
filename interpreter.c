#include <stdio.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

Token bool_negate(Token t)
{
    if (t.type == T_TRUE) {
        return make_token(T_FALSE);
    } else if (t.type == T_FALSE) {
        return make_token(T_TRUE);
    } else if (t.type == T_DOUBLE) {
        double n = ((double *)t.data)[0];
        if (n) {
            return make_token(T_FALSE);
        } else {
            return make_token(T_TRUE);
        }
    } else {
        printf("Cannot negate token '");
        print_token(t);
        printf("'\n");
        exit(1);
    }
}

Token double_negate(Token t)
{
    double n = ((double *)t.data)[0];
    n = -n;
    double *n_new = malloc(sizeof(double));
    memcpy(n_new, &n, sizeof(double));
    return make_double(n_new);
}

Token eval_unexpr(UnExpr unexpr, Env *env)
{
    Token arg = eval_expr(unexpr.expr, env);
    switch (unexpr.op.type) {
    case T_BANG:
        return bool_negate(arg);
    case T_MINUS:
        return double_negate(arg);
    default:
        printf("Token '");
        print_token(unexpr.op);
        printf("' is not a unary operator\n");
        exit(1);
    }
}

Token eval_binexpr(BinExpr binexpr, Env *env)
{
    Token lt = eval_expr(binexpr.lexpr, env);
    Token rt = eval_expr(binexpr.rexpr, env);

    if (lt.type != T_DOUBLE || rt.type != T_DOUBLE) {
        printf("Binary expression must be between two doubles\n");
        exit(1);
    }
    double *n_new = malloc(sizeof(double));

    switch (binexpr.op.type) {
    case T_PLUS:
        *n_new = get_ddata(lt) + get_ddata(rt);
        break;
    case T_MINUS:
        *n_new = get_ddata(lt) - get_ddata(rt);
        break;
    case T_STAR:
        *n_new = get_ddata(lt) * get_ddata(rt);
        break;
    case T_SLASH:
        *n_new = get_ddata(lt) / get_ddata(rt);
        break;
    case T_EQUAL: {
        Token lvalue = binexpr.lexpr.as->termexpr.term;
        env_assign(env, lvalue, rt);
        *n_new = get_ddata(rt);
        break;
    case T_LESS: {
        double ltd = get_ddata(lt);
        double rtd = get_ddata(rt);
        Token res = ltd < rtd
            ? make_token(T_TRUE)
            : make_token(T_FALSE);
        return res;
    }
    case T_GREATER:
        return get_ddata(lt) > get_ddata(rt)
            ? make_token(T_TRUE)
            : make_token(T_FALSE);
    }
    case T_2EQUAL:
        return get_ddata(lt) == get_ddata(rt)
            ? make_token(T_TRUE)
            : make_token(T_FALSE);
    default:
        printf("Binary operation '");
        print_token(binexpr.op);
        printf("' is not supported\n");
        exit(1);
    }

    return make_double(n_new);
}

Token eval_termexpr(TermExpr termexpr, Env *env)
{
    switch (termexpr.term.type) {
    case T_TRUE:
    case T_FALSE:
    case T_DOUBLE:
    case T_STRING:
        return termexpr.term;
    case T_NAME:
        return env_get(env, termexpr.term);
    default:
        printf("Could not evaluate literal '");
        print_token(termexpr.term);
        printf("'\n");
        exit(1);
    }
}

Token eval_expr(Expr expr, Env *env)
{
    switch (expr.type) {
    case UNARY:
        return eval_unexpr(expr.as->unexpr, env);
    case BINARY:
        return eval_binexpr(expr.as->binexpr, env);
    case GROUPING:
        return eval_expr(expr.as->groupexpr.expr, env);
    case TERMINAL:
        return eval_termexpr(expr.as->termexpr, env);
    default:
        printf("Expression '");
        print_expr(expr);
        printf("' is not supported\n");
        exit(1);
    }
}

void eval_letstmt(LetStmt letstmt, Env *env)
{
    Token name = letstmt.name;
    Token value = eval_expr(letstmt.value, env);
    env_define(env, name, value);
    // printf("Assigned '");
    // print_token(value);
    // printf("' to '");
    // print_token(name);
    // printf("'\n");
}

bool is_thruty(Token t)
{
    if (t.type == T_TRUE)
        return true;

    if (t.type == T_DOUBLE) {
        return get_ddata(t) != 0;
    }
    return false;
}

void eval_ifstmt(IfStmt ifstmt, Env *env)
{
    Token cond = eval_expr(ifstmt.cond, env);
    if (is_thruty(cond)) {
        eval_stmt(ifstmt.thenb, env);
    } else {
        eval_stmt(ifstmt.elseb, env);
    }
}

void eval_forstmt(ForStmt forstmt, Env *env)
{
    eval_expr(forstmt.init, env);
    // Token term = eval_expr(forstmt.cond, env);
    // print_token(term);
    // is_thruty(term);
    while (is_thruty(eval_expr(forstmt.cond, env))) {
        eval_expr(forstmt.step, env);
        eval_stmt(forstmt.thenb, env);
    }
}

void eval_whilestmt(WhileStmt whilestmt, Env *env)
{
    while (is_thruty(eval_expr(whilestmt.cond, env))) {
        eval_stmt(whilestmt.thenb, env);
    }
}

void eval_blockstmt(BlockStmt blockstmt, Env *env)
{
    Block block = blockstmt.block;
    Env localenv = {0};
    localenv.upper = env;
    for (size_t i = 0; i < block.size; i++) {
        eval_stmt(block.items[i], &localenv);
    }
}

void eval_exprstmt(ExprStmt exprstmt, Env *env)
{
    Token value = eval_expr(exprstmt.expr, env);
    // print_token(value);
    // print_token(value);
}

void eval_retstmt(RetStmt retstmt, Env *env)
{
    Token value = eval_expr(retstmt.expr, env);
    exit(get_ddata(value));
    // print_token(value);
    // print_token(value);
}

void eval_stmt(Stmt stmt, Env *env)
{
    // print_stmt(stmt);

    switch (stmt.type) {
    case S_LET:
        eval_letstmt(stmt.as->letstmt, env);
        break;
    case S_IF:
        eval_ifstmt(stmt.as->ifstmt, env);
        break;
    case S_FOR:
        eval_forstmt(stmt.as->forstmt, env);
        break;
    case S_WHILE:
        eval_whilestmt(stmt.as->whilestmt, env);
        break;
    case S_BLOCK:
        eval_blockstmt(stmt.as->blockstmt, env);
        break;
    case S_EXPR:
        eval_exprstmt(stmt.as->exprstmt, env);
        break;
    case S_RET:
        eval_retstmt(stmt.as->retstmt, env);
        break;
    default:
        printf("Statement '");
        print_stmt(stmt);
        printf("' is not supported\n");
    }
}

void eval_program(Program *pr)
{
    Env env = {0};
    for (size_t i = 0; i < pr->size; i++) {
        eval_stmt(pr->items[i], &env);
    }
    print_env(&env);
    free_env(&env);
}

int token_cmp(Token t1, Token t2)
{
    return strcmp((char *)t1.data, (char *)t2.data);
}

EnvNode *en_define(EnvNode *en, Token lvalue, Token rvalue)
{
    if (en) {
        int cmp = token_cmp(lvalue, en->lvalue);
        if (cmp < 0) {
            en->left = en_define(en->left, lvalue, rvalue);
            return en;
        } else if (cmp > 0) {
            en->right = en_define(en->right, lvalue, rvalue);
            return en;
        } else {
            printf("Variable '");
            print_token(lvalue);
            printf("' is already defined\n");
            exit(1);
        }
    } else {
        en = malloc(sizeof(EnvNode));
        en->lvalue = lvalue;
        en->rvalue = rvalue;
        en->left = en->right = NULL;
        return en;
    }
}

void env_define(Env *env, Token lvalue, Token rvalue)
{
    env->root = en_define(env->root, lvalue, rvalue);
}

EnvNode *en_assign(EnvNode *en, Token lvalue, Token rvalue)
{
    if (en) {
        int cmp = token_cmp(lvalue, en->lvalue);
        if (cmp < 0) {
            return en_assign(en->left, lvalue, rvalue);
        } else if (cmp > 0) {
            return en_assign(en->right, lvalue, rvalue);
        } else {
            en->rvalue = rvalue;
            return en;
        }
    } else {
        return NULL;
    }
}

void env_assign(Env *env, Token lvalue, Token rvalue)
{
    EnvNode *en = en_assign(env->root, lvalue, rvalue);
    if (en) {
        return;
    } else if (en == NULL && env->upper) {
        env_assign(env->upper, lvalue, rvalue);
    } else {
        printf("Undefined variable '");
        print_token(lvalue);
        printf("' at line %zu\n", lvalue.line);
        exit(1);
    }
}

EnvNode *en_get(EnvNode *en, Token lvalue)
{
    if (en) {
        int cmp = token_cmp(lvalue, en->lvalue);
        if (cmp < 0) {
            return en_get(en->left, lvalue);
        } else if (cmp > 0) {
            return en_get(en->right, lvalue);
        } else {
            return en;
        }
    } else {
        return NULL;
    }
}

Token env_get(Env *env, Token lvalue)
{
    EnvNode *en = en_get(env->root, lvalue);
    if (en) {
        return en->rvalue;
    } else if (en == NULL && env->upper) {
        return env_get(env->upper, lvalue);
    } else {
        printf("Undefined variable '");
        print_token(lvalue);
        printf("' at line %zu\n", lvalue.line);
        exit(1);
    }
}

void print_en(EnvNode *en)
{
    if (en) {
        print_token(en->lvalue);
        printf(" = ");
        print_token(en->rvalue);
        printf("\n");
        print_en(en->left);
        print_en(en->right);
    }
}

void print_env(Env *env)
{
    print_en(env->root);
}

void free_en(EnvNode *en)
{
    if (en) {
        free_en(en->left);
        free_en(en->right);
        free(en);
    }
}

void free_env(Env *env)
{
    free_en(env->root);
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: %s <source.l>\n", argv[0]);
        exit(1);
    }

    Buffer b;
    v_init(b);
    FILE *f = fopen(argv[1], "r");
    if (f == NULL) {
        printf("Could not open file %s\n", argv[1]);
        exit(1);
    }
    get_content(f, &b);

    // Lex
    Lexer l;
    lexer_init(&l, b.items);
    get_tokens(&l);
    // print_tokens(&l);

    // Parse
    Parser p;
    parser_init(&p, &l);
    Program pr = parse_program(&p);
    // print_program(&pr);

    // Evaluate
    Stmt firststmt = pr.items[0];
    eval_program(&pr);

    // Free memory
    program_free(&pr); // Free statements and expressions
                       // (program)
    lexer_free(&l); // Free tokens (lexer and parser)
    free(b.items); // Free content buffer

    return 0;
}
