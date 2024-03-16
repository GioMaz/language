#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser.h"

typedef struct envnode EnvNode;
typedef struct envnode {
    Token lvalue;
    Token rvalue;
    EnvNode *left;
    EnvNode *right;
} EnvNode;

typedef struct env Env;
typedef struct env {
    EnvNode *root;
    Env *upper;
} Env;

EnvNode *en_define(EnvNode *en, Token lvalue, Token rvalue);
void env_define(Env *env, Token lvalue, Token rvalue);
EnvNode *en_get(EnvNode *en, Token lvalue);
Token env_get(Env *env, Token lvalue);
EnvNode *en_assign(EnvNode *en, Token lvalue, Token rvalue);
void env_assign(Env *env, Token lvalue, Token rvalue);
void free_en(EnvNode *en);
void free_env(Env *env);
void print_en(EnvNode *en);
void print_env(Env *env);

Token eval_expr(Expr expr, Env *env);
void eval_stmt(Stmt stmt, Env *env);

#endif
