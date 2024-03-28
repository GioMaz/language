#include <stdio.h>
#include <string.h>

#include "llvm-c/Core.h"

#include "vector.h"
#include "lexer.h"
#include "parser.h"

void print_module(LLVMModuleRef module)
{
    char *ir = LLVMPrintModuleToString(module);
    printf("=== START IR ===\n%s==== END IR ====\n", ir);
    free(ir);
}

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
    NamedValues nvalues;
} Codegen;

NvNode *nvnode_insert(NvNode *node, char *name, LLVMValueRef value)
{
    if (node) {
        int cmp = strcmp(name, node->name);
        if (cmp < 0) {
            node->left = nvnode_insert(node->left, name, value);
            return node;
        } else if (cmp > 0) {
            node->right = nvnode_insert(node->right, name, value);
            return node;
        } else {
            printf("Name %s is already defined\n", name);
            exit(1);
        }
    } else {
        node = malloc(sizeof(NVNode));
        node->name = name;
        node->value = value;
        node->left = node->right = NULL;
        return node;
    }
}

void nv_insert(NamedValues *nvalues, char *name, LLVMValueRef value)
{
    nvalues->root = nvnode_insert(nvalues->root, name, value);
}

NvNode *nvnode_lookup(NvNode *node, char *name)
{
    if (node) {
        int cmp = strcmp(name, node->name);
        if (cmp < 0) {
            return nvnode_lookup(node->left, name);
        } else if (cmp > 0) {
            return nvnode_lookup(node->right, name);
        } else {
            return node;
        }
    } else {
        printf("Name %s is not defined\n", name);
        exit(1);
    }
}

LLVMValueRef nv_lookup(NamedValues *nvalues, char *name)
{
    return nvnode_lookup(nvalues->root, name)->value;
}

LLVMValueRef gen_expr(Codegen *codegen, Expr expr);

LLVMValueRef gen_unexpr(Codegen *codegen, UnExpr unexpr)
{
    LLVMValueRef value = gen_expr(codegen, unexpr.expr);
    switch (unexpr.op.type) {
    case T_BANG:
        return LLVMBuildNeg(codegen->builder, value, "negtmp");
    case T_MINUS:
        return LLVMBuildNeg(codegen->builder, value, "negtmp");
    default:
        printf("Token '");
        print_token(unexpr.op);
        printf("' is not a unary operator\n");
        exit(1);
    }
}

LLVMValueRef gen_binexpr(Codegen *codegen, BinExpr binexpr)
{
    LLVMValueRef lhs = gen_expr(codegen, binexpr.lexpr);
    LLVMValueRef rhs = gen_expr(codegen, binexpr.rexpr);
    switch (binexpr.op.type) {
    case T_PLUS:
        return LLVMBuildFAdd(codegen->builder, lhs, rhs, "addtmp");
    case T_MINUS:
        return LLVMBuildFSub(codegen->builder, lhs, rhs, "subtmp");
    case T_STAR:
        return LLVMBuildFMul(codegen->builder, lhs, rhs, "multmp");
    case T_SLASH:
        return LLVMBuildFDiv(codegen->builder, lhs, rhs, "divtmp");
    case T_LESS: {
        LLVMValueRef cmp = LLVMBuildFCmp(codegen->builder, LLVMRealULT, lhs, rhs, "lttmp");
        return LLVMBuildFPCast(codegen->builder, cmp, LLVMInt1Type(), "lesstmp");
    }
    case T_GREATER: {
        LLVMValueRef cmp = LLVMBuildFCmp(codegen->builder, LLVMRealUGT, lhs, rhs, "lttmp");
        return LLVMBuildFPCast(codegen->builder, cmp, LLVMInt1Type(), "lesstmp");
    }
    case T_EQUAL: {
        if (binexpr.lexpr.type != TERMINAL) {
            printf("Bro\n");
            exit(1);
        }
        Token ltok = binexpr.lexpr.as->termexpr.term;
        LLVMValueRef lvalue = nv_lookup(&codegen->nvalues, ltok.data);
    }
    default:
        printf("Token '");
        print_token(binexpr.op);
        printf("' is not a binary operator\n");
        exit(1);
    }
}

LLVMValueRef gen_termexpr(Codegen *codegen, TermExpr termexpr)
{
    switch (termexpr.term.type) {
    case T_DOUBLE: {
        double value = get_ddata(termexpr.term);
        return LLVMConstReal(LLVMDoubleType(), value);
    }
    case T_NAME: {
        return nv_lookup(&codegen->nvalues, termexpr.term.data);
    }
    default:
        printf("Could not evaluate '");
        print_token(termexpr.term);
        printf("'\n");
        exit(1);
    }
}

LLVMValueRef gen_expr(Codegen *codegen, Expr expr)
{
    switch (expr.type) {
    case UNARY:
        return gen_unexpr(codegen, expr.as->unexpr);
    case BINARY:
        return gen_binexpr(codegen, expr.as->binexpr);
    case GROUPING:
        return gen_expr(codegen, expr.as->groupexpr.expr);
    case TERMINAL:
        return gen_termexpr(codegen, expr.as->termexpr);
    default:
        printf("Expression '");
        print_expr(expr);
        printf("' is not supported\n");
        exit(1);
    }
}

void gen_retstmt(Codegen *codegen, RetStmt retstmt)
{
    LLVMValueRef value = gen_expr(codegen, retstmt.expr);
    LLVMBuildRet(codegen->builder, value);
}

void gen_stmt(Codegen *codegen, Stmt stmt);

void gen_letstmt(Codegen *codegen, LetStmt letstmt)
{
    LLVMValueRef value = gen_expr(codegen, letstmt.value);
    nv_insert(&codegen->nvalues, letstmt.name.data, value);
}

void gen_ifstmt(Codegen *codegen, IfStmt ifstmt)
{
    // If
    LLVMValueRef cond = gen_expr(codegen, ifstmt.cond);
    LLVMBasicBlockRef bb = LLVMGetInsertBlock(codegen->builder);
    LLVMValueRef parent = LLVMGetBasicBlockParent(bb);

    // Then
    LLVMBasicBlockRef thenb = LLVMAppendBasicBlock(parent, "then");
    LLVMPositionBuilderAtEnd(codegen->builder, thenb);
    gen_stmt(codegen, ifstmt.thenb);

    // Else
    LLVMBasicBlockRef elseb = LLVMAppendBasicBlock(parent, "else");
    LLVMPositionBuilderAtEnd(codegen->builder, elseb);
    gen_stmt(codegen, ifstmt.elseb);

    // End
    LLVMBasicBlockRef end = LLVMAppendBasicBlock(parent, "end");
    LLVMPositionBuilderAtEnd(codegen->builder, thenb);
    LLVMBuildBr(codegen->builder, end);
    LLVMPositionBuilderAtEnd(codegen->builder, elseb);
    LLVMBuildBr(codegen->builder, end);

    LLVMPositionBuilderAtEnd(codegen->builder, bb);
    LLVMValueRef br = LLVMBuildCondBr(codegen->builder, cond, thenb, elseb);
    LLVMPositionBuilderAtEnd(codegen->builder, end);
}

void gen_forstmt(Codegen *codegen, ForStmt forstmt)
{
    LLVMValueRef init = gen_expr(codegen, forstmt.init);
}

void gen_blockstmt(Codegen *codegen, BlockStmt blockstmt)
{
    Block block = blockstmt.block;
    for (size_t i = 0; i < block.size; i++) {
        gen_stmt(codegen, block.items[i]);
    }
}

void gen_exprstmt(Codegen *codegen, ExprStmt exprstmt)
{
    gen_expr(codegen, exprstmt.expr);
}

void gen_stmt(Codegen *codegen, Stmt stmt)
{
    switch (stmt.type) {
    case S_LET:
        gen_letstmt(codegen, stmt.as->letstmt);
        break;
    case S_IF:
        gen_ifstmt(codegen, stmt.as->ifstmt);
        break;
    case S_FOR:
        gen_forstmt(codegen, stmt.as->forstmt);
        break;
    case S_WHILE:
        printf("WHILE\n");
        break;
    case S_BLOCK:
        gen_blockstmt(codegen, stmt.as->blockstmt);
        break;
    case S_EXPR:
        gen_exprstmt(codegen, stmt.as->exprstmt);
        break;
    case S_FUNC:
        printf("FUNC\n");
        break;
    case S_RET:
        gen_retstmt(codegen, stmt.as->retstmt);
        break;
    }
}

void gen_main(LLVMModuleRef module, LLVMBuilderRef builder, Program program)
{
    LLVMTypeRef main_proto = LLVMFunctionType(LLVMDoubleType(), NULL, 0, false);
    LLVMValueRef main_func = LLVMAddFunction(module, "main", main_proto);
    LLVMBasicBlockRef bb = LLVMAppendBasicBlock(main_func, "entry");
    LLVMPositionBuilderAtEnd(builder, bb);

    NamedValues nvalues = { 0 };
    Codegen codegen = {
        .builder = builder,
        .nvalues = nvalues,
    };

    for (size_t i = 0; i < program.size; i++) {
        gen_stmt(&codegen, program.items[i]);
    }

    LLVMBuildRet(builder, LLVMConstReal(LLVMDoubleType(), 0));
}

int main(int argc, char **argv)
{
    Buffer b;
    v_init(b);
    FILE *f = fopen("code.l", "r");
    get_content(f, &b);

    // Lex
    Lexer l;
    lexer_init(&l, b.items);
    get_tokens(&l);

    // Parse
    Parser p;
    parser_init(&p, &l);
    Program pr = parse_program(&p);

    // Generate
    LLVMContextRef context = LLVMContextCreate();
    LLVMModuleRef module = LLVMModuleCreateWithNameInContext("l_program", context);
    LLVMBuilderRef builder = LLVMCreateBuilderInContext(context);

    gen_main(module, builder, pr);
    // LLVMValueRef value = gen_expr(builder, pr.items[0].as->letstmt.value);
    // LLVMBuildRet(builder, value);

    print_module(module);

    return 0;
}
