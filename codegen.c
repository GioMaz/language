#include <stdio.h>

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

LLVMValueRef gen_expr(LLVMBuilderRef builder, Expr expr);

LLVMValueRef gen_unexpr(LLVMBuilderRef builder, UnExpr unexpr)
{
    LLVMValueRef value = gen_expr(builder, unexpr.expr);
    switch (unexpr.op.type) {
    case T_BANG:
        return LLVMBuildNeg(builder, value, "negtmp");
    case T_MINUS:
        return LLVMBuildNeg(builder, value, "negtmp");
    default:
        printf("Token '");
        print_token(unexpr.op);
        printf("' is not a unary operator\n");
        exit(1);
    }
}

LLVMValueRef gen_binexpr(LLVMBuilderRef builder, BinExpr binexpr)
{
    LLVMValueRef lhs = gen_expr(builder, binexpr.lexpr);
    LLVMValueRef rhs = gen_expr(builder, binexpr.rexpr);
    switch (binexpr.op.type) {
    case T_PLUS:
        return LLVMBuildFAdd(builder, lhs, rhs, "addtmp");
    case T_MINUS:
        return LLVMBuildFSub(builder, lhs, rhs, "subtmp");
    case T_STAR:
        return LLVMBuildFMul(builder, lhs, rhs, "multmp");
    case T_SLASH:
        return LLVMBuildFDiv(builder, lhs, rhs, "divtmp");
    default:
        printf("Token '");
        print_token(binexpr.op);
        printf("' is not a binary operator\n");
        exit(1);
    }
}

LLVMValueRef gen_termexpr(LLVMBuilderRef builder, TermExpr termexpr)
{
    switch (termexpr.term.type) {
    case T_DOUBLE: {
        double value = get_ddata(termexpr.term);
        return LLVMConstReal(LLVMDoubleType(), value);
    }
    case T_NAME:
        // return LLVMGetNamedGlobal();
    default:
        printf("Could not evaluate '");
        print_token(termexpr.term);
        printf("'\n");
        exit(1);
    }
}

LLVMValueRef gen_expr(LLVMBuilderRef builder, Expr expr)
{
    switch (expr.type) {
    case UNARY:
        return gen_unexpr(builder, expr.as->unexpr);
    case BINARY:
        return gen_binexpr(builder, expr.as->binexpr);
    case GROUPING:
        return gen_expr(builder, expr.as->groupexpr.expr);
    case TERMINAL:
        return gen_termexpr(builder, expr.as->termexpr);
    default:
        printf("Expression '");
        print_expr(expr);
        printf("' is not supported\n");
        exit(1);
    }
}

void gen_retstmt(LLVMBuilderRef builder, RetStmt retstmt)
{
    LLVMValueRef value = gen_expr(builder, retstmt.expr);
    LLVMBuildRet(builder, value);
}

LLVMValueRef gen_letstmt(LLVMBuilderRef builder, LetStmt letstmt)
{
    LLVMValueRef value = gen_expr(builder, letstmt.value);
    return LLVMBuildAlloca(builder, LLVMDoubleType(), letstmt.name.data);
}

void gen_stmt(LLVMBuilderRef builder, Stmt stmt)
{
    switch (stmt.type) {
    case S_LET:
        gen_letstmt(builder, stmt.as->letstmt);
        break;
    case S_IF:
        printf("IF\n");
        break;
    case S_FOR:
        printf("FOR\n");
        break;
    case S_WHILE:
        printf("WHILE\n");
        break;
    case S_BLOCK:
        printf("BLOCK\n");
        break;
    case S_EXPR:
        printf("EXPR\n");
        break;
    case S_FUNC:
        printf("FUNC\n");
        break;
    case S_RET:
        gen_retstmt(builder, stmt.as->retstmt);
        break;
    }
}

void gen_main(LLVMModuleRef module, LLVMBuilderRef builder, Program program)
{
    LLVMTypeRef main_proto = LLVMFunctionType(LLVMInt32Type(), NULL, 0, false);
    LLVMValueRef main_func = LLVMAddFunction(module, "main", main_proto);
    LLVMBasicBlockRef bb = LLVMAppendBasicBlock(main_func, "entry");
    LLVMPositionBuilderAtEnd(builder, bb);

    for (size_t i = 0; i < program.size; i++) {
        gen_stmt(builder, program.items[i]);
    }
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
