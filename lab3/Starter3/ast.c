#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

#include "ast.h"
#include "common.h"
#include "parser.tab.h"

#define DEBUG_PRINT_TREE 0

node *ast = NULL;

// forward declare
void print_type(node * ast);

node *ast_allocate(node_kind kind, ...) {
  va_list args;

  // make the node
  node *ast = (node *) malloc(sizeof(node));
  memset(ast, 0, sizeof *ast);
  ast->kind = kind;

  va_start(args, kind); 

  switch(kind) {

  case UNKNOWN:
    printf("UNKNOWN in allocate\n");
    break;
  
  case SCOPE_NODE:
    ast->scope.decs = va_arg(args, node *);
    ast->scope.stmts = va_arg(args, node *);
    break;

  case STATEMENTS_NODE:
    ast->statements.next = va_arg(args, node *);
    ast->statements.stmt = va_arg(args, node *);
    break;

  case DECLARATIONS_NODE:
    ast->declarations.next = va_arg(args, node *);
    ast->declarations.dec = va_arg(args, node *);
    break;

  case DECLARATION_NODE:
    ast->dec.name = va_arg(args, char *);
    ast->dec.type = va_arg(args, node *);
    ast->dec.init = va_arg(args, node *);
    ast->dec.isConst = va_arg(args, int);
    break;

  case ARGUMENTS_NODE:
    ast->args.next = va_arg(args, node *);
    ast->args.arg = va_arg(args, node *);
    break;

  case IF_STATEMENT_NODE:
    ast->if_st.expr = va_arg(args, node *);
    ast->if_st.stmt1 = va_arg(args, node *);
    ast->if_st.stmt2 = va_arg(args, node *);
    break;

  case ASSIGNMENT_NODE:
    ast->assignment.var = va_arg(args, node *);
    ast->assignment.expr = va_arg(args, node *);
    ast->assignment.type = NULL; // set when traversing statements in semantic.c
    break;

  case EXPRESSION_NODE:
    ast->expr.ex = va_arg(args, node *);
    break;

  case UNARY_EXPRESSION_NODE:
    ast->unary_expr.op = va_arg(args, int);
    ast->unary_expr.right = va_arg(args, node *);
    ast->unary_expr.type = NULL; // set in semantic.c
    break;

  case BINARY_EXPRESSION_NODE:
    ast->binary_expr.op = va_arg(args, int);
    ast->binary_expr.left = va_arg(args, node *);
    ast->binary_expr.right = va_arg(args, node *);
    ast->binary_expr.type = NULL; // set in semantic.c
    break;

  case INT_NODE:
    ast->int_const.val = va_arg(args, int);
    break;

  case FLOAT_NODE:
    // double instead of float because of compiler warning
    ast->float_const.val = va_arg(args, double);
    break;

  case BOOL_NODE:
    ast->bool_const.val = va_arg(args, int);
    break;

  case TYPE_NODE:
    ast->type.base_type = va_arg(args, int);
    ast->type.index = va_arg(args, int);
    break;

  case IDENT_NODE:
    ast->id.name = va_arg(args, char *);
    break;

  case INDEX_NODE:
    ast->index.name = va_arg(args, char *);
    ast->index.index = va_arg(args, int);
    ast->index.type = NULL; // set when traversing statements in semantic.c
    break;

  case VAR_NODE:
    ast->expr.ex = va_arg(args, node *);
    break;

  case FUNCTION_NODE:
    ast->func.code = va_arg(args, int);
    ast->func.args = va_arg(args, node *);
    break;

  case CONSTRUCTOR_NODE:
    ast->constr.type = va_arg(args, node *);
    ast->constr.args = va_arg(args, node *);

    if (ast->constr.type->type.index == 1) {
	printf("Can't apply constructor to scalar type\n");
    }
    break;

  case NESTED_SCOPE_NODE:
    ast->expr.ex = va_arg(args, node *);
    break;
  }

  va_end(args);

  return ast;
}

void ast_free(node *ast) {

}

void print_declarations_inner(node * ast) {
    if (ast->kind != DECLARATIONS_NODE || ast->declarations.next == NULL) {
        if (ast->declarations.dec != NULL) {
            ast_print(ast->declarations.dec);
        }
        return;
    }
    print_declarations_inner(ast->declarations.next);
    printf(" ");
    ast_print(ast->declarations.dec);
}

void print_declarations(node * ast) {
    if (ast == NULL || ast->declarations.dec == NULL) {
        printf("(DECLARATIONS)");
        return;
    }

    printf("(DECLARATIONS");
    print_declarations_inner(ast);
    printf(")");
}

void print_declaration(node * ast) {
    printf("(DECLARATION ");
    printf("%s ", ast->dec.name);
    print_type(ast->dec.type);
    if (ast->dec.init != NULL) {
        printf(" ");
        ast_print(ast->dec.init);
    }
    printf(")");
}

void print_statements_inner(node * ast) {
    //if (ast->statements.next == NULL)
    //{
    //	printf("Will seg fault\n");
    //	return;
    //}
    if (ast->kind != STATEMENTS_NODE)
    {
	ast_print(ast);
	return;
    }

    if (ast->statements.next == NULL) {
        if (ast->statements.stmt != NULL) {
            ast_print(ast->statements.stmt);
        }
        return;
    }
    print_statements_inner(ast->statements.next);
    printf(" ");
    ast_print(ast->statements.stmt);
}

void print_statements(node * ast) {
    if (ast == NULL || ast->statements.stmt == NULL) {
        printf("(STATEMENTS)");
        return;
    }

    printf("(STATEMENTS");
    print_statements_inner(ast);
    printf(")");
}

void print_if(node * ast) {
    printf("(IF ");
    ast_print(ast->if_st.expr);
    printf(" ");
    ast_print(ast->if_st.stmt1);
    if (ast->if_st.stmt2 != NULL) {
        printf(" ");
        ast_print(ast->if_st.stmt2);
    }
    printf(")");
}

void print_arguments_inner(node * ast) {
    if (ast->kind != ARGUMENTS_NODE || ast->args.next == NULL) {
        if (ast->args.arg != NULL) {
            ast_print(ast->args.arg);
        }
        return;
    }
    print_arguments_inner(ast->args.next);
    printf(" ");
    ast_print(ast->args.arg);
}

void print_arguments(node * ast) {
    if (ast == NULL || ast->args.arg == NULL) {
        return;
    }

    print_arguments_inner(ast);
}

void print_scope(node * ast) {
    printf("(SCOPE ");
    print_declarations(ast->scope.decs);
    printf(" ");
    print_statements(ast->scope.stmts);
    printf(")");
}

void print_unary(node * ast) {
    char op = (ast->unary_expr.op == UMINUS) ? '-' : '!';
    printf("(UNARY ");
    print_type(ast->unary_expr.type);
    printf(" %c ", op);
    ast_print(ast->unary_expr.right);
    printf(")");
}

void print_binary(node * ast) {
    char* op;
    switch(ast->binary_expr.op) {
        case AND: op = "&&"; break;
        case OR:  op = "||"; break;
        case EQ:  op = "=="; break;
        case NEQ: op = "!="; break;
        case LEQ: op = "<="; break;
        case GEQ: op = ">="; break;
        default:
                 op = (char *) &(ast->binary_expr.op);
    }
    printf("(BINARY ");
    print_type(ast->binary_expr.type);
    printf(" %s ", op);
    ast_print(ast->binary_expr.left);
    printf(" ");
    ast_print(ast->binary_expr.right);
    printf(")");
}

void print_bool(node * ast) {
    printf("%s", ast->bool_const.val == 0 ? "false" : "true");
}

void print_int(node * ast) {
    printf("%d", ast->int_const.val);
}

void print_float(node * ast) {
    printf("%f", ast->float_const.val);
}

void print_ident(node * ast) {
    printf("%s", ast->id.name);
}

void print_index(node * ast) {
    printf("(INDEX ");
    // TODO: fix
    if (ast->index.type != NULL) {
        print_type(ast->index.type);
    }
    printf(" %s %d)", ast->index.name, ast->index.index);
}

void print_type(node * ast) {
    switch(ast->type.base_type) {



        case VEC_T: if (ast->type.index == 1) { printf("float"); return;} else {printf("vec");} break;
        case IVEC_T: if (ast->type.index == 1) { printf("int"); return; } else {printf("ivec");} break;
        case BVEC_T: if (ast->type.index == 1) {  printf("bool"); return; } else {printf("bvec");} break;
    }

    printf("%d", ast->type.index + 0);
}

void print_constructor(node * ast) {
    printf("(CALL ");
    print_type(ast->constr.type);
    printf(" ");
    ast_print(ast->constr.args);
    printf(")");
}

void print_function(node * ast) {
    printf("(CALL ");
    switch(ast->func.code) {
        case 0: printf("dp3"); break;
        case 1: printf("lit"); break;
        case 2: printf("rsq"); break;
    }
    printf(" ");
    ast_print(ast->func.args);
    printf(")");
}

void print_assignment(node * ast) {
    printf("(ASSIGN ");
    if (ast->assignment.type != NULL) {
        print_type(ast->assignment.type);
    }
    printf(" ");
    ast_print(ast->assignment.var);
    printf(" ");
    ast_print(ast->assignment.expr);
    printf(")");
}

void ast_print(node * ast) {
    if (ast == NULL) {
        printf("ast is NULL\n");
        return;
    }

    switch(ast->kind) {
        case UNKNOWN:
            printf("(UNKNOWN)");
            break;
        case SCOPE_NODE:
            print_scope(ast);
            break;
        case EXPRESSION_NODE:
        case VAR_NODE:
        case NESTED_SCOPE_NODE:
            ast_print(ast->expr.ex);
            break;
        case UNARY_EXPRESSION_NODE:
            print_unary(ast);
            break;
        case BINARY_EXPRESSION_NODE:
            print_binary(ast);
            break;
        case BOOL_NODE:
            print_bool(ast);
            break;
        case INT_NODE:
            print_int(ast);
            break;
        case FLOAT_NODE:
            print_float(ast);
            break;
        case TYPE_NODE:
            print_type(ast);
            break;
        case IDENT_NODE:
            print_ident(ast);
            break;
        case INDEX_NODE:
            print_index(ast);
            break;
        case CONSTRUCTOR_NODE:
            print_constructor(ast);
            break;
        case FUNCTION_NODE:
            print_function(ast);
            break;
        case STATEMENTS_NODE:
            print_statements(ast);
            break;
        case DECLARATIONS_NODE:
            print_declarations(ast);
            break;
        case DECLARATION_NODE:
            print_declaration(ast);
            break;
        case ARGUMENTS_NODE:
            print_arguments(ast);
            break;
        case IF_STATEMENT_NODE:
            print_if(ast);
            break;
        case ASSIGNMENT_NODE:
            print_assignment(ast);
            break;
	case BLNK_STMT_NODE:
	    break;
    }
}
