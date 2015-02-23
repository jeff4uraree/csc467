#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"
#include "common.h"
#include "parser.tab.h"

#define OUT(x) { fprintf(outputFile, "%s\n", x); }

// keeps track of whether we are currently generating code for the condition of
// an if statement
int is_cond = 0;

// keeps track of whether we are currently generating code within an if
// statement
int in_if = 0;

// keeps track of whether we are currently generating code within a nested scope
int in_nested_scope = 0;

// keeps track of which temps have been declared. non-decreasing
// ex. if value is 2, then tempVar1 and tempVar2 have been declared
int declared_tmp = 0;

int declared_cond_tmp = 0;

// foward declare
void gen(node * ast);
void gen_expr(node * ast, int ret_tmp);

// translates index into a vector into the corresponding component
char index_to_cmp(int index) {
    switch (index) {
        case 0: return 'x';
        case 1: return 'y';
        case 2: return 'z';
        case 3: return 'w';
        default:
            return '0'; // shouldn't happen
    }
}

char * handle_predefined(char * name) {
    if (!strcmp(name, "gl_FragColor")) {
        return "result.color";
    } else if (!strcmp(name, "gl_FragDepth")) {
        return "result.depth";
    } else if (!strcmp(name, "gl_FragCoord")) {
        return "fragment.position"; 
    } else if (!strcmp(name, "gl_TexCoord")) {
        return "fragment.texcoord";
    } else if (!strcmp(name, "gl_Color")) {
        return "fragment.color";
    } else if (!strcmp(name, "gl_Secondary")) {
        return "fragment.color.secondary";
    } else if (!strcmp(name, "gl_FogFragCoord")) {
        return "fragment.fogcoord";
    } else if (!strcmp(name, "env1")) {
        return "program.env[1]";
    } else if (!strcmp(name, "env2")) {
        return "program.env[2]";
    } else if (!strcmp(name, "env3")) {
        return "program.env[3]";
    } else if (!strcmp(name, "gl_Light_Half")) {
        return "state.light[0].half";
    } else if (!strcmp(name, "gl_Light_Ambient")) {
        return "state.lightmodel.ambient";
    } else if (!strcmp(name, "gl_Material_Shininess")) {
        return "state.material.shininess";
    }

    return name;
}

int is_predefined(char * name) {
    return strcmp(name, handle_predefined(name));
}

// returns the name of the variable, modifying it if in a nested scope
char * var_name(char * name, int id, int index, int is_id) {
    if (is_id && !is_predefined(name)) {
        // 1 for '$', 1 for '_' and 9 for ID
        char * ret = (char *) malloc(sizeof(char) * (strlen(name) + 11));
        snprintf(ret, strlen(name) + 11, "$%d_%s", id, name);
        return ret;
    } else if (!is_id && !is_predefined(name)) {
        // 1 for '$', 1 for '_' and 9 for ID, 1 for '.' and 1 for the index
        char * ret = (char *) malloc(sizeof(char) * (strlen(name) + 13));
        snprintf(ret, strlen(name) + 13, "$%d_%s.%c", id, name, index_to_cmp(index));
        return ret;
    }

    return name;
}

char * gen_var(node * ast) {
    char *ret;

    if (ast->kind == IDENT_NODE && !is_predefined(ast->id.name)) {
        return var_name(ast->id.name, ast->id.id, -1, 1);
    } else if (ast->kind == INDEX_NODE && !is_predefined(ast->index.name)) {
        return var_name(ast->index.name, ast->index.id, ast->index.index, 0);
    }

    switch (ast->kind) {
        case VAR_NODE:
            return gen_var(ast->expr.ex);
        case IDENT_NODE:
            return handle_predefined(ast->id.name);
        case INDEX_NODE:
            ret = (char *) malloc(sizeof(char) * (strlen(handle_predefined(ast->index.name)) + 3));
            snprintf(ret, strlen(handle_predefined(ast->index.name)) + 3, "%s.%c", handle_predefined(ast->index.name), index_to_cmp(ast->index.index));
            return ret;
        default:
            return NULL;
    }
}

char * int_to_cond_tmp(int tmp) {
    char * ret = (char *) malloc(sizeof(char) * (strlen("tempVarCond") + 3));
    snprintf(ret, strlen("tempVarCond") + 3, "tempVarCond%d", tmp);
    return ret;
}

// returns the name of the tmp var corresponding to the given int
char * int_to_tmp(int tmp) {
    char * ret;
    if (!is_cond) {
        // declare a temp if we need to
        if (tmp > declared_tmp) {
            fprintf(outputFile, "TEMP tempVar%d;\n", tmp);
            declared_tmp++;
        }

        ret = (char *) malloc(sizeof(char) * (strlen("tempVar") + 10));
        snprintf(ret, strlen("tempVar") + 10, "tempVar%d", tmp);
    } else {
        // declare a temp if we need to
        if (tmp > declared_cond_tmp) {
            fprintf(outputFile, "TEMP tempVarCond%d;\n", tmp);
            declared_cond_tmp++;
        }

        ret = int_to_cond_tmp(tmp);
    }
    return ret;
}

// returns how many recursive calls were made + 1
int gen_args(node * ast, int arg_tmp) {
    // arg, next
    if (ast->kind != ARGUMENTS_NODE || ast->args.next == NULL) {
        if (ast->args.arg != NULL) {
            gen_expr(ast->args.arg, arg_tmp);
        }
        return 1;
    }

    int calls = gen_args(ast->args.next, arg_tmp);
    gen_expr(ast->args.arg, arg_tmp + calls);

    return calls + 1;
}

// ret_tmp is the temp variable used to return the value of this expr
// i.e. if 1, the result of the expr would be in tempVar1
void gen_expr(node * ast, int ret_tmp) {
    char *str;

    switch (ast->kind) {
        case VAR_NODE:
        case IDENT_NODE:
        case INDEX_NODE:
            str = gen_var(ast);
            fprintf(outputFile, "MOV %s, %s;\n", int_to_tmp(ret_tmp), str);
            break;
        case INT_NODE:
            fprintf(outputFile, "MOV %s, %f;\n", int_to_tmp(ret_tmp), (float) ast->int_const.val);
            break;
        case FLOAT_NODE:
            fprintf(outputFile, "MOV %s, %f;\n", int_to_tmp(ret_tmp), ast->float_const.val);
            break;
        case BOOL_NODE:
            fprintf(outputFile, "MOV %s, %f;\n", int_to_tmp(ret_tmp), (float) ast->bool_const.val);
            break;
        case EXPRESSION_NODE:
            gen_expr(ast->expr.ex, 1);
            break;
        case UNARY_EXPRESSION_NODE:
            gen_expr(ast->unary_expr.right, 1);
            if (ast->unary_expr.op == UMINUS) {
                fprintf(outputFile, "MOV %s, -%s;\n", int_to_tmp(ret_tmp), int_to_tmp(ret_tmp));
            } else if (ast->unary_expr.op == (int) '!') {
                fprintf(outputFile, "SLT %s, %s, %f;\n", int_to_tmp(ret_tmp), int_to_tmp(ret_tmp), 1.0);
            }
            break;
        case BINARY_EXPRESSION_NODE:
            // TODO: verify that these work properly with vectors (and with
            // scalar/vector for MUL)
            gen_expr(ast->binary_expr.left, ret_tmp);
            gen_expr(ast->binary_expr.right, ret_tmp + 1);
            switch (ast->binary_expr.op) {
                case '+':
                    fprintf(outputFile, "ADD %s, %s, %s;\n", int_to_tmp(ret_tmp), int_to_tmp(ret_tmp), int_to_tmp(ret_tmp + 1));
                    break;
                case '-':
                    fprintf(outputFile, "SUB %s, %s, %s;\n", int_to_tmp(ret_tmp), int_to_tmp(ret_tmp), int_to_tmp(ret_tmp + 1));
                    break;
                case '*':
                    fprintf(outputFile, "MUL %s, %s, %s;\n", int_to_tmp(ret_tmp), int_to_tmp(ret_tmp), int_to_tmp(ret_tmp + 1));
                    break;
                case '/':
                    fprintf(outputFile, "RCP %s, %s.x;\n",   int_to_tmp(ret_tmp + 1), int_to_tmp(ret_tmp + 1));
                    fprintf(outputFile, "MUL %s, %s, %s;\n", int_to_tmp(ret_tmp), int_to_tmp(ret_tmp), int_to_tmp(ret_tmp + 1));
                    break;
                case '^':
                    fprintf(outputFile, "POW %s, %s.x, %s.x;\n", int_to_tmp(ret_tmp), int_to_tmp(ret_tmp), int_to_tmp(ret_tmp + 1));
                    break;
                case AND:
                    fprintf(outputFile, "MIN %s, %s, %s;\n", int_to_tmp(ret_tmp), int_to_tmp(ret_tmp), int_to_tmp(ret_tmp + 1));
                    break;
                case OR:
                    fprintf(outputFile, "MAX %s, %s, %s;\n", int_to_tmp(ret_tmp), int_to_tmp(ret_tmp), int_to_tmp(ret_tmp + 1));
                    break;
                case '<':
                    fprintf(outputFile, "SLT %s, %s, %s;\n", int_to_tmp(ret_tmp), int_to_tmp(ret_tmp), int_to_tmp(ret_tmp + 1));
                    break;
                case '>':
                    fprintf(outputFile, "SLT %s, %s, %s;\n", int_to_tmp(ret_tmp), int_to_tmp(ret_tmp + 1), int_to_tmp(ret_tmp));
                    break;
                case GEQ:
                    fprintf(outputFile, "SGE %s, %s, %s;\n", int_to_tmp(ret_tmp), int_to_tmp(ret_tmp), int_to_tmp(ret_tmp + 1));
                    break;
                case LEQ:
                    fprintf(outputFile, "SGE %s, %s, %s;\n", int_to_tmp(ret_tmp), int_to_tmp(ret_tmp + 1), int_to_tmp(ret_tmp));
                    break;
                case EQ:
                    fprintf(outputFile, "SGE %s, %s, %s;\n", int_to_tmp(ret_tmp + 2), int_to_tmp(ret_tmp), int_to_tmp(ret_tmp + 1));
                    fprintf(outputFile, "SGE %s, %s, %s;\n", int_to_tmp(ret_tmp), int_to_tmp(ret_tmp + 1), int_to_tmp(ret_tmp));
                    fprintf(outputFile, "MIN %s, %s, %s;\n", int_to_tmp(ret_tmp), int_to_tmp(ret_tmp), int_to_tmp(ret_tmp + 2));
                    break;
                case NEQ:
                    fprintf(outputFile, "SLT %s, %s, %s;\n", int_to_tmp(ret_tmp + 2), int_to_tmp(ret_tmp), int_to_tmp(ret_tmp + 1));
                    fprintf(outputFile, "SLT %s, %s, %s;\n", int_to_tmp(ret_tmp), int_to_tmp(ret_tmp + 1), int_to_tmp(ret_tmp));
                    fprintf(outputFile, "MAX %s, %s, %s;\n", int_to_tmp(ret_tmp), int_to_tmp(ret_tmp), int_to_tmp(ret_tmp + 2));
                    break;
            }
            break;
        case CONSTRUCTOR_NODE:
            gen_args(ast->constr.args, ret_tmp);
            // NOTE: first component is already set
            if (ast->constr.type->type.index > 1) {
                fprintf(outputFile, "MOV %s.y, %s;\n", int_to_tmp(ret_tmp), int_to_tmp(ret_tmp + 1));
            }
            if (ast->constr.type->type.index > 2) {
                fprintf(outputFile, "MOV %s.z, %s;\n", int_to_tmp(ret_tmp), int_to_tmp(ret_tmp + 2));
            }
            if (ast->constr.type->type.index > 3) {
                fprintf(outputFile, "MOV %s.w, %s;\n", int_to_tmp(ret_tmp), int_to_tmp(ret_tmp + 3));
            }
            break;
        case FUNCTION_NODE:
            gen_args(ast->func.args, ret_tmp);
            switch (ast->func.code) {
                case 0: // DP3
                    fprintf(outputFile, "DP3 %s, %s, %s;\n", int_to_tmp(ret_tmp), int_to_tmp(ret_tmp), int_to_tmp(ret_tmp + 1));
                    break;
                case 1: // LIT
                    fprintf(outputFile, "LIT %s, %s;\n", int_to_tmp(ret_tmp), int_to_tmp(ret_tmp));
                    break;
                case 2: // RSQ
                    fprintf(outputFile, "RSQ %s, %s.x;\n", int_to_tmp(ret_tmp), int_to_tmp(ret_tmp));
                    break;
            }
            break;
        default:
            printf("gen_expr: shouldn't happen\n");
    }
}

void gen_if_mov(char * dst, char * src) {
    fprintf(outputFile, "CMP %s, %s, %s, %s;\n", dst, int_to_cond_tmp(in_if), dst, src);
}

void gen_assignment(node * ast) {
    gen_expr(ast->assignment.expr, 1);
    if (!in_if) {
        if (ast->assignment.var->kind == INDEX_NODE) {
            // If LHS is index then RHS should use first component
            fprintf(outputFile, "MOV %s, %s.x;\n", gen_var(ast->assignment.var), int_to_tmp(1));
        } else {
            fprintf(outputFile, "MOV %s, %s;\n", gen_var(ast->assignment.var), int_to_tmp(1));
        }
    } else {
        if (ast->assignment.var->kind == INDEX_NODE) {
            // If LHS is index then RHS should use first component
            char *src = (char *) malloc(sizeof(char) * (strlen(int_to_tmp(1)) + 3));
            snprintf(src, strlen(int_to_tmp(1)) + 3, "%s.x", int_to_tmp(1));
            gen_if_mov(gen_var(ast->assignment.var), src);
        } else {
            gen_if_mov(gen_var(ast->assignment.var), int_to_tmp(1));
        }
    }
}

void gen_statement(node * ast) {
    switch (ast->kind) {
        case ASSIGNMENT_NODE:
            gen_assignment(ast);
            break;
        case IF_STATEMENT_NODE:
            in_if++;

            fprintf(outputFile, "\n# Start if, depth: %d\n", in_if);

            is_cond = 1;
            gen_expr(ast->if_st.expr, in_if);
            if (in_if > 1) {
                // in nested if, so we need to consider the condition of the
                // enclosing if

                // first add back 0.5 to treat as a boolean (i.e. 0 or 1)
                fprintf(outputFile, "ADD %s, %s, %f;\n", int_to_cond_tmp(in_if - 1), int_to_cond_tmp(in_if - 1), 0.5);

                // AND with the enclosing if, so that if the inclosing one is
                // false, we don't allow assignments
                fprintf(outputFile, "MIN %s, %s, %s;\n", int_to_cond_tmp(in_if), int_to_cond_tmp(in_if - 1), int_to_cond_tmp(in_if));
            }
            // change bool from 0/1 to -0.5/0.5 so that it can be used with CMP
            fprintf(outputFile, "SUB %s, %s, %f;\n", int_to_cond_tmp(in_if), int_to_cond_tmp(in_if), 0.5);
            is_cond = 0;

            fprintf(outputFile, "# Start then, depth: %d\n", in_if);
            gen_statement(ast->if_st.stmt1);
            fprintf(outputFile, "# End then, depth: %d\n", in_if);

            // flip cond for else (so that CMP works properly in assignments
            fprintf(outputFile, "CMP %s, %s, %f, %f;\n", int_to_cond_tmp(in_if), int_to_cond_tmp(in_if), 0.5, -0.5);

            if (in_if > 1) {
                // in nested if, so we need to consider the condition of the
                // enclosing if

                // first add back 0.5 to treat as a boolean (i.e. 0 or 1)
                fprintf(outputFile, "ADD %s, %s, %f;\n", int_to_cond_tmp(in_if), int_to_cond_tmp(in_if), 0.5);

                // AND with the enclosing if, so that if the inclosing one is
                // false, we don't allow assignments
                fprintf(outputFile, "MIN %s, %s, %s;\n", int_to_cond_tmp(in_if), int_to_cond_tmp(in_if - 1), int_to_cond_tmp(in_if));

                // sub 0.5 again to convert to -0.5/0.5
                fprintf(outputFile, "SUB %s, %s, %f;\n", int_to_cond_tmp(in_if), int_to_cond_tmp(in_if), 0.5);
            }

            if (ast->if_st.stmt2 != NULL) {
                fprintf(outputFile, "# Start else, depth: %d\n", in_if);
                gen_statement(ast->if_st.stmt2);
            }

            if (in_if > 1) {
                // sub 0.5 to convert back to -0.5/0.5. This is needed for the
                // case where there are multiple if statements that are nested
                // at the same level so that when the next if at the same level
                // is started, it can add back 0.5 to treat the cond var as a
                // boolean
                fprintf(outputFile, "SUB %s, %s, %f;\n", int_to_cond_tmp(in_if - 1), int_to_cond_tmp(in_if - 1), 0.5);
            }

            fprintf(outputFile, "# End if, depth: %d\n\n", in_if);

            in_if--;
            break;
        case NESTED_SCOPE_NODE:
            in_nested_scope++;
            gen(ast->expr.ex);
            in_nested_scope--;
            break;
        default:
            printf("gen_statement: shouldn't happen\n");
    }
}

void gen_statements(node * ast) {
    if (ast == NULL || ast->statements.stmt == NULL) {
        return;
    }

    gen_statements(ast->statements.next);
    gen_statement(ast->statements.stmt);
}

// used for generating a string representing the RHS of a const declaration
char * gen_lit(node * ast) {
    int size = 10;
    char *str = (char *) malloc(sizeof(char) * size);

    switch (ast->kind) {
        case INT_NODE:
            snprintf(str, size, "%f", (float) ast->int_const.val);
            break;
        case FLOAT_NODE:
            snprintf(str, size, "%f", ast->float_const.val);
            break;
        case BOOL_NODE:
            snprintf(str, size, "%f", (float) ast->bool_const.val);
            break;
        default:
            printf("gen_lit: shouldn't happen\n");
    }

    return str;
}

void gen_declaration(node * ast) {
    // dec fields: name, type, init (nullable expr), isConst
    if (ast->dec.isConst) {
        char *varName = NULL;
        if (ast->dec.init->expr.ex->kind == IDENT_NODE) {
            varName = ast->dec.init->expr.ex->id.name;
        } else if (ast->dec.init->expr.ex->kind == INDEX_NODE) {
            varName = ast->dec.init->expr.ex->index.name;
        }

        char *name = var_name(ast->dec.name, ast->dec.id, -1, 1);
        if (varName != NULL && is_predefined(varName)) {
            char *regName = handle_predefined(varName);
            fprintf(outputFile, "TEMP %s;\n", name);
            fprintf(outputFile, "MOV %s, %s;\n", name, gen_var(ast->dec.init->expr.ex));
        } else {
            char *lit = gen_lit(ast->dec.init);
            fprintf(outputFile, "PARAM %s = { %s, %s, %s, %s };\n", name, lit, lit, lit, lit);
        }
    } else {
        char *name = var_name(ast->dec.name, ast->dec.id, -1, 1);
        fprintf(outputFile, "TEMP %s;\n", name);
        if (ast->dec.init != NULL) {
            gen_expr(ast->dec.init, 1);
            fprintf(outputFile, "MOV %s, %s;\n", name, int_to_tmp(1));
        }
    }
}

void gen_declarations(node * ast) {
    if (ast == NULL || ast->declarations.dec == NULL) {
        return;
    }

    gen_declarations(ast->declarations.next);
    gen_declaration(ast->declarations.dec);
}

void gen_scope(node * ast) {
    gen_declarations(ast->scope.decs);
    gen_statements(ast->scope.stmts);
}

void gen(node * ast) {
    switch(ast->kind) {
        case SCOPE_NODE:
            gen_scope(ast);
            break;
        case EXPRESSION_NODE:
        case VAR_NODE:
        case NESTED_SCOPE_NODE:
            gen(ast->expr.ex);
            break;
        case STATEMENTS_NODE:
            gen_statements(ast);
            break;
        case DECLARATIONS_NODE:
            gen_declarations(ast);
            break;
        case DECLARATION_NODE:
            gen_declaration(ast);
            break;
        case ASSIGNMENT_NODE:
            gen_assignment(ast);
            break;
        default:
            printf("gen: shouldn't happen\n");
    }
}

void genCode(node *ast) {
    OUT("!!ARBfp1.0");
    OUT("");

    gen(ast);

    OUT("");
    OUT("END");
}

