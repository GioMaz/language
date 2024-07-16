#ifndef CODEGEN_H
#define CODEGEN_H

#include "llvm-c/Core.h"

void print_module(LLVMModuleRef module);

typedef struct nvnode NvNode;
typedef struct nvnode {
    char *name;
    LLVMValueRef value;
    NvNode *left;
    NvNode *right;
} NVNode;

typedef struct nvalues NamedValues;
typedef struct nvalues {
    NvNode *root;
} NamedValues;

typedef struct {
    LLVMBuilderRef builder;
    NamedValues *nvalues;
} Codegen;

NvNode *nvnode_insert(NvNode *node, char *name, LLVMValueRef value);
void nv_insert(NamedValues *nvalues, char *name, LLVMValueRef value);
NvNode *nvnode_lookup(NvNode *node, char *name);
LLVMValueRef nv_lookup(NamedValues *nvalues, char *name);

LLVMValueRef gen_unexpr(Codegen *codegen, UnExpr unexpr);
LLVMValueRef gen_binexpr(Codegen *codegen, BinExpr binexpr);
LLVMValueRef gen_termexpr(Codegen *codegen, TermExpr termexpr);
LLVMValueRef gen_expr(Codegen *codegen, Expr expr);
void gen_retstmt(Codegen *codegen, RetStmt retstmt);
void gen_letstmt(Codegen *codegen, LetStmt letstmt);
void gen_ifstmt(Codegen *codegen, IfStmt ifstmt);
void gen_forstmt(Codegen *codegen, ForStmt forstmt);
void gen_blockstmt(Codegen *codegen, BlockStmt blockstmt);
void gen_exprstmt(Codegen *codegen, ExprStmt exprstmt);
void gen_stmt(Codegen *codegen, Stmt stmt);
void gen_main(LLVMModuleRef module, LLVMBuilderRef builder, Program program);

#endif
