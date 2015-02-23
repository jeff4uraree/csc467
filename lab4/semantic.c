#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol.h"
#include "semantic.h"
#include "parser.tab.h"
#include "common.h"

#define ERROR(x) { fprintf(errorFile, "\nSEMANTIC ERROR: %s\n", x); errorOccurred = TRUE; }

// Need to use this to ensure result variables aren't assigned to in if/else
// statements. Every time an if/else is entered, this will be incremented. It
// will be decremented when leaving an if/else
int in_if_st = 0;

// Used to assign a unique ID to each declared variable
int next_id = 1;


void walk_for_expr(node * ast);

node * new_type_node(int type, int index) {
    node * ret = (node *) malloc(sizeof(node *));
    ret->kind = TYPE_NODE;
    ret->type.base_type = type;
    ret->type.index = index;
    return ret;
}

void insert_expr_type(node * ast, node * type) {
    node *type_node = (node *) malloc(sizeof(node *));
    type_node->kind = TYPE_NODE;
    if (type != NULL) {
        type_node->type.base_type = type->type.base_type;
        type_node->type.index = type->type.index;
    }

    if (ast->kind == UNARY_EXPRESSION_NODE) {
        ast->unary_expr.type = type_node;
    } else if (ast->kind == BINARY_EXPRESSION_NODE) {
        ast->binary_expr.type = type_node;
    } 
}

void check_binary_types(node *expr, node *lhs_type, node *rhs_type) {
    if (lhs_type == NULL || rhs_type == NULL) {
        ERROR("Unable to determine type of expression because an operand's type is unknown");
        return;
    }
    if (lhs_type->type.base_type != rhs_type->type.base_type) {
        ERROR("Both operands of a binary expression must have the same base type");
        return;
    }

    int left_is_scalar  = lhs_type->type.index == 1;
    int right_is_scalar = rhs_type->type.index == 1;
    int is_arithmetic = lhs_type->type.base_type != BVEC_T;

    int op = expr->binary_expr.op;
    switch (op) {
        case '+':
        case '-':
            if (!is_arithmetic) {
                ERROR("Operands must be of arithmetic type");
                return;
            }
            if (left_is_scalar != right_is_scalar) {
                ERROR("Operands must both be scalar or both vector");
                return;
            }
            break;
        case '*':
            if (!is_arithmetic) {
                ERROR("Operands must be of arithmetic type");
                return;
            }
            break;
        case '/':
        case '^':
            if (!is_arithmetic) {
                ERROR("Operands must be of arithmetic type");
                return;
            }
            if (!left_is_scalar || !right_is_scalar) {
                ERROR("Operands must both be scalar");
                return;
            }
            break;
        case OR:
        case AND:
            if (is_arithmetic) {
                ERROR("Operands must be of logical type");
                return;
            }
            if (left_is_scalar != right_is_scalar) {
                ERROR("Operands must both be scalar or both vector");
                return;
            }
            break;
        case '<':
        case LEQ:
        case '>':
        case GEQ:
            if (!is_arithmetic) {
                ERROR("Operands must be of arithmetic type");
                return;
            }
            if (!left_is_scalar || !right_is_scalar) {
                ERROR("Operands must both be scalar");
                return;
            }
            break;
        case EQ:
        case NEQ:
            if (!is_arithmetic) {
                ERROR("Operands must be of arithmetic type");
                return;
            }
            if (left_is_scalar != right_is_scalar) {
                ERROR("Operands must both be scalar or both vector");
                return;
            }
            break;
    }

    if (!left_is_scalar && !right_is_scalar && lhs_type->type.index != rhs_type->type.index) {
        ERROR("Operands must be of the same size when vector");
        return;
    }
}

// counts args for constructor
int count_args(node *args) {
    if (args == NULL || args->args.arg == NULL) {
        return 0;
    }

    int count = 1;
    while (args->args.next != NULL) {
        count++;
        args = args->args.next;
    }

    return count;
}

// performs checks for number and types of arguments for constructors
void validate_constr_args(node *expr) {
    int count = 0;
    if (expr->constr.args->kind == ARGUMENTS_NODE) {
        count = count_args(expr->constr.args);
    }

    if (count != expr->constr.type->type.index) {
        ERROR("Wrong number of arguments");
        return;
    }

    node *args = expr->constr.args;
    while (args != NULL) {
        node *arg_type = get_expr_type(args->args.arg);
        if (expr->constr.type->type.base_type != arg_type->type.base_type || arg_type->type.index != 1) {
            ERROR("Incorrect type used in constructor");
            return;
        }
        args = args->args.next;
    }
}

// returns type of expr
node *handle_expr(node * expr) {
    char *name;
    innerNode* symbol;
    node *ret_type;
    node *arg1_type;
    node *arg2_type;
    int op;


    node_kind kind = expr->kind;
    switch (kind) {
        case CONSTRUCTOR_NODE:
            walk_for_expr(expr->constr.args);
            validate_constr_args(expr);
            return expr->constr.type;
        case FUNCTION_NODE:
            walk_for_expr(expr->func.args);
            ret_type = (node *) malloc(sizeof(node *));
            ret_type->kind = TYPE_NODE;
            switch (expr->func.code) {
                case 0: // DP3
	    	    if (count_args(expr->func.args) != 2) {
			ERROR("Incorrect number of arguments");
			return NULL;
                    }

	    	    // Check types of function arguments
		    arg1_type = get_expr_type(expr->func.args->args.arg);
		    
                    arg2_type = get_expr_type(expr->func.args->args.next->args.arg);
                    if (arg1_type->type.base_type == VEC_T && arg2_type->type.base_type == VEC_T) {
                        ret_type->type.base_type = VEC_T;
                        if (arg1_type->type.index == 3 && arg2_type->type.index == 3) {
                            ret_type->type.index = 1;
                        } else if (arg1_type->type.index == 4 && arg2_type->type.index == 4) {
                            ret_type->type.index = 1;
                        } else {
                            ERROR("Incorrect number of indices");
                            return NULL;
                        }
                    } else if (arg1_type->type.base_type == IVEC_T && arg2_type->type.base_type == IVEC_T) {
                        ret_type->type.base_type = IVEC_T;
                        if (arg1_type->type.index == 3 && arg2_type->type.index == 3) {
                            ret_type->type.index = 1;
                        } else if (arg1_type->type.index == 4 && arg2_type->type.index == 4) {
                            ret_type->type.index = 1;
                        } else {
                            ERROR("Incorrect number of indices");
                            return NULL;
                        }
                    } else {
                        // Error: incorrect argument types	
                        ERROR("Arguments of wrong type");
                        return NULL;
                    }
		
		    break;
                case 1: // LIT
                    ret_type->type.base_type = VEC_T;
                    ret_type->type.index = 4; 

	    	    if (count_args(expr->func.args) != 1) {
			ERROR("Incorrect number of arguments");
			return NULL;
                    }

		    arg1_type = get_expr_type(expr->func.args->args.arg);
		    
	    	    // Check types of function arguments
		    if (arg1_type->type.base_type != VEC_T) {
		    	ERROR("Arguments of wrong type");
		    	return NULL;
		    }
		    
		    // Check if the argument has the correct number of indices
		    if (arg1_type->type.index != 4) {
			ERROR("Argument has incorrect number of indices");
			return NULL;
		    }
		  
                    break;
                case 2: // RSQ
                    ret_type->type.base_type = VEC_T;
                    ret_type->type.index = 1;

	    	    if (count_args(expr->func.args) != 1) {
			ERROR("Incorrect number of arguments");
			return NULL;
                    }

		    arg1_type = get_expr_type(expr->func.args->args.arg);
		    
		    if (arg1_type->type.base_type != VEC_T && arg1_type->type.base_type != IVEC_T) {
			ERROR("Argument 1 of wrong type");
			return NULL;
		    }
		    
		    if (arg1_type->type.index != 1) {
			ERROR("Arguments has incorrect number of indices");
			return NULL;
                    }
		    
                    break;
            }

            expr->func.ret_type = ret_type;

            return ret_type;
        case UNARY_EXPRESSION_NODE:
            ret_type = handle_expr(expr->unary_expr.right);

            // Check that the expression type is compatible with the op
            if (expr->unary_expr.op == (int) '!' && ret_type != NULL && ret_type->type.base_type != BVEC_T) {
                ERROR("Only boolean types can be used with !");
            } else if (expr->unary_expr.op == UMINUS && ret_type != NULL && ret_type->type.base_type != IVEC_T && ret_type->type.base_type != VEC_T) {
                ERROR("Only arithmetic types can be used with -");
            }

            insert_expr_type(expr, ret_type);
            return ret_type;
        case BINARY_EXPRESSION_NODE:
            ret_type = handle_expr(expr->binary_expr.left);
            arg2_type = handle_expr(expr->binary_expr.right);

            check_binary_types(expr, ret_type, arg2_type);

            op = expr->binary_expr.op;
            if (op == '>' || op == '<' || op == GEQ || op == LEQ || op == EQ || op == NEQ) {
                ret_type->type.base_type = BVEC_T;
            }

            // Used when multiplying a scalar by a vector
            if (op == '*' && arg2_type != NULL && ret_type != NULL && arg2_type->type.index > ret_type->type.index) {
                ret_type->type.index = arg2_type->type.index;
            }

            insert_expr_type(expr, ret_type);
            return ret_type;
        case BOOL_NODE:
            return new_type_node(BVEC_T, 1);
        case INT_NODE:
            return new_type_node(IVEC_T, 1);
        case FLOAT_NODE:
            return new_type_node(VEC_T, 1);
        case EXPRESSION_NODE:
        case VAR_NODE:
            return handle_expr(expr->expr.ex);
        case IDENT_NODE:
            name = expr->id.name;
            symbol = lookup_innerNode(name);
            if (symbol == NULL) {
                ERROR("Using variable in expr which hasn't been declared");
                return NULL;
            }
            if (!strcmp(name, "gl_FragColor") ||
                !strcmp(name, "gl_FragDepth")) {
                ERROR("Attempting to read a result variable");
            }

            expr->id.id = symbol->id;

            return new_type_node(symbol->type, symbol->size);
        case INDEX_NODE:
            name = expr->index.name;
            symbol = lookup_innerNode(name);
            if (symbol == NULL) {
                ERROR("Using variable in expr which hasn't been declared");
                return NULL;
            }
            if (!strcmp(name, "gl_FragColor") ||
                !strcmp(name, "gl_FragDepth")) {
                ERROR("Attempting to read a result variable");
            }

            expr->index.id = symbol->id;

            return new_type_node(symbol->type, 1);
    }

    return NULL;
}

void walk_for_expr(node * ast) {
    if (ast == NULL) {
        return;
    }
    innerNode* ourNode;
    node *rhs_type;
    node_kind kind = ast->kind;
    switch (kind) {
        case DECLARATION_NODE:
            walk_for_expr(ast->dec.init);

            // Check that RHS type matches declared type
            if (ast->dec.init == NULL) {
                return; // Nothing left to check
            }
            rhs_type = get_expr_type(ast->dec.init);

            if (ast->dec.isConst == 1) {
                switch (ast->dec.init->kind) {
                    case VAR_NODE:
                        if (ast->dec.init->expr.ex->kind == IDENT_NODE) {
                            ourNode = lookup_innerNode(ast->dec.init->expr.ex->id.name);
                            // Variable is not uniform type
                            if (ourNode != NULL && 
                                strcmp(ourNode->name, "gl_Light_Half") &&
                                strcmp(ourNode->name, "gl_Light_Ambient") &&
                                strcmp(ourNode->name, "gl_Material_Shininess") &&
                                strcmp(ourNode->name, "env1") &&
                                strcmp(ourNode->name, "env2") &&
                                strcmp(ourNode->name, "env3")) {
                                ERROR("Assigned non constant value to constant declared variable");
                            }
                        } else if (ast->dec.init->expr.ex->kind == INDEX_NODE) {
                            ourNode = lookup_innerNode(ast->dec.init->expr.ex->index.name);
                            // Variable is not uniform type
                            if (ourNode != NULL && 
                                strcmp(ourNode->name, "gl_Light_Half") &&
                                strcmp(ourNode->name, "gl_Light_Ambient") &&
                                strcmp(ourNode->name, "gl_Material_Shininess") &&
                                strcmp(ourNode->name, "env1") &&
                                strcmp(ourNode->name, "env2") &&
                                strcmp(ourNode->name, "env3")) {
                                ERROR("Assigned non constant value to constant declared variable");
                            }
                        }
                        break;
                    case INT_NODE:
                    case FLOAT_NODE:
                    case BOOL_NODE:
                        break;
                    default:
                        ERROR("Assigned non constant value to constant declared variable");
                        break;
                }
            }

            if (rhs_type != NULL && (ast->dec.type->type.base_type != rhs_type->type.base_type || ast->dec.type->type.index != rhs_type->type.index)) {
                ERROR("Initial value type does not match declared variable's type");
            }
            break;
        case ASSIGNMENT_NODE:
            walk_for_expr(ast->assignment.expr);
            break;
        case IF_STATEMENT_NODE:
            walk_for_expr(ast->if_st.expr);
            walk_for_expr(ast->if_st.stmt1);
            walk_for_expr(ast->if_st.stmt2);
            break;
        case ARGUMENTS_NODE:
            walk_for_expr(ast->args.next);
            walk_for_expr(ast->args.arg);
            break;
        case UNARY_EXPRESSION_NODE:
        case BINARY_EXPRESSION_NODE:
        case VAR_NODE:
        case CONSTRUCTOR_NODE:
        case FUNCTION_NODE:
        case EXPRESSION_NODE:
            handle_expr(ast);
            break;
    }
}

void insert_index_type(node * ast) {
    char *name = ast->index.name;
    innerNode* symbol = lookup_innerNode(name);
    if (symbol == NULL) {
        ERROR("Indexing into variable which hasn't been declared");
        return;
    }

    ast->index.type = new_type_node(symbol->type, 1);
    ast->index.id = symbol->id;
}

void walk_expr_for_index(node * expr) {
    if (expr == NULL) {
        return;
    }

    node_kind kind = expr->kind;
    switch (kind) {
        case CONSTRUCTOR_NODE:
            walk_expr_for_index(expr->constr.args);
            break;
        case FUNCTION_NODE:
            walk_expr_for_index(expr->func.args);
            break;
        case UNARY_EXPRESSION_NODE:
            walk_expr_for_index(expr->unary_expr.right);
            break;
        case BINARY_EXPRESSION_NODE:
            walk_expr_for_index(expr->binary_expr.left);
            walk_expr_for_index(expr->binary_expr.right);
            break;
        case EXPRESSION_NODE:
        case VAR_NODE:
            walk_expr_for_index(expr->expr.ex);
            break;
        case INDEX_NODE:
            insert_index_type(expr);
	    vector_index_check(expr);
            break;
        case ARGUMENTS_NODE:
            walk_expr_for_index(expr->args.next);
            walk_expr_for_index(expr->args.arg);
            break;
        case ASSIGNMENT_NODE:
            walk_expr_for_index(expr->assignment.var);
            walk_expr_for_index(expr->assignment.expr);
            break;
        case IF_STATEMENT_NODE:
            walk_expr_for_index(expr->if_st.expr);
            walk_expr_for_index(expr->if_st.stmt1);
            walk_expr_for_index(expr->if_st.stmt2);
            break;
        default:
            ; // we don't care about these types
    }
}

void insert_assignment_type(node * ast) {
    node *var_node = ast->assignment.var;

    char *name;
    if (var_node->kind == IDENT_NODE) {
        name = var_node->id.name;
    } else if (var_node->kind == INDEX_NODE) {
        name = var_node->index.name;
    }

    innerNode* symbol = lookup_innerNode(name);
    if (symbol == NULL) {
        ERROR("Assigning to variable which hasn't been declared");
        return;
    }

    node *rhs_type = get_expr_type(ast->assignment.expr);
    int lhs_size;
    if (var_node->kind == IDENT_NODE) {
        lhs_size = symbol->size;
    } else if (var_node->kind == INDEX_NODE) {
	lhs_size = 1;
    }
    if (symbol->type != rhs_type->type.base_type || lhs_size != rhs_type->type.index) {
	ERROR("Assigning wrong type to variable");
	return;
    }

    ast->assignment.type = new_type_node(symbol->type, 1);

    if (var_node->kind == IDENT_NODE) {
        var_node->id.id = symbol->id;
    } else if (var_node->kind == INDEX_NODE) {
        var_node->index.id = symbol->id;
    }
}

void walk_for_asn(node * expr) {
    if (expr == NULL) {
        return;
    }

    node_kind kind = expr->kind;
    switch (kind) {
        case ASSIGNMENT_NODE:
	    const_check(expr);
            insert_assignment_type(expr);
            break;
        case IF_STATEMENT_NODE:
            walk_for_asn(expr->if_st.stmt1);
            walk_for_asn(expr->if_st.stmt2);
            break;
        default:
            ; // we don't care about these types
    }
}

void traverse_declarations_inner(node  * ast) {
    innerNode* target;
    if (ast->kind != DECLARATIONS_NODE || ast->declarations.next == NULL) {
        if (ast->declarations.dec != NULL) {
            target = lookup_onelevel_innerNode(ast->declarations.dec->dec.name);
            // Check for duplicate declaration calls, if we are able to find another node in symbol table with same name, return and set errorflag
            if (target != NULL) {
                ERROR("Duplicate variable declaration in the current scope");
                return;
            }

            walk_expr_for_index(ast->declarations.dec->dec.init);
            walk_for_expr(ast->declarations.dec);
            insert_innerNode(ast->declarations.dec->dec.name, ast->declarations.dec->dec.type->type.base_type, ast->declarations.dec->dec.type->type.index, ast->declarations.dec->dec.isConst, next_id);
            ast->declarations.dec->dec.id = next_id;
            next_id++;
        }
        return;
    }

    traverse_declarations_inner(ast->declarations.next);

    target = lookup_onelevel_innerNode(ast->declarations.dec->dec.name);
    if (target != NULL) {  
	ERROR("Duplicate variable declaration in the current scope");
	return;
    }

    walk_expr_for_index(ast->declarations.dec->dec.init);
    walk_for_expr(ast->declarations.dec);
    insert_innerNode(ast->declarations.dec->dec.name, ast->declarations.dec->dec.type->type.base_type, ast->declarations.dec->dec.type->type.index, ast->declarations.dec->dec.isConst, next_id);
    ast->declarations.dec->dec.id = next_id;
    next_id++;
}

void traverse_statements_inner(node * ast) {
   if (ast == NULL || ast->statements.stmt == NULL) {
	return;
   }

    node* type;
    switch (ast->kind) {
	case IF_STATEMENT_NODE:
            in_if_st++; // Enter if statement

            walk_expr_for_index(ast->if_st.expr);
            walk_for_expr(ast->if_st.expr);
            type = get_expr_type(ast->if_st.expr);
            if (type->type.base_type != BVEC_T || type->type.index != 1) {
                    ERROR("Condition in if statement must be of boolean type");
            }

            traverse_statements_inner(ast->if_st.stmt1);
            traverse_statements_inner(ast->if_st.stmt2);

            in_if_st--; // Leave if statement
	break;

	case NESTED_SCOPE_NODE:
            insert_outerNode();
            traverse_scope(ast->expr.ex);
            remove_outerNode();
	break;

	case STATEMENTS_NODE:
            traverse_statements_inner(ast->statements.next);
            traverse_statements_inner(ast->statements.stmt);
	break;

        case ASSIGNMENT_NODE:
            if (in_if_st) {
                char *name = NULL;
                if (ast->expr.ex->kind == IDENT_NODE) {
                    name = ast->expr.ex->id.name;
                } else if (ast->expr.ex->kind == INDEX_NODE) {
                    name = ast->expr.ex->index.name;
                }

                // Catch assignment to result variable
                if (name != NULL &&
                    (!strcmp(name, "gl_FragColor") ||
                     !strcmp(name, "gl_FragDepth") ||
                     !strcmp(name, "gl_FragCoord"))) {
                    ERROR("Attempting to write to a result variable in an if statement");
                }
            }

            // fall through
	default:
            walk_expr_for_index(ast);
            walk_for_expr(ast);
            walk_for_asn(ast);
    }
}

void traverse_scope(node * ast) {
    if (ast->scope.decs != NULL) {
    	traverse_declarations_inner(ast->scope.decs);
    }
    if (ast->scope.stmts != NULL) {
    	traverse_statements_inner(ast->scope.stmts);
    }
}

void const_check(node *expr) {
     innerNode* ourNode;

     // Case where we are getting name of identifier
     if (expr->assignment.var->kind == IDENT_NODE) {
         ourNode = lookup_innerNode(expr->assignment.var->id.name);
     }

     // Case where we are getting name of index
     if (expr->assignment.var->kind == INDEX_NODE) {
         ourNode = lookup_innerNode(expr->assignment.var->index.name);
     }

     
     if (ourNode != NULL) {
	  if (ourNode->isConst == 1) {
		ERROR("Made assignment to const variable");
		return;
	  }
     }
}

void vector_index_check(node * expr) {
     innerNode* ourNode;	
     ourNode = lookup_innerNode(expr->index.name);
     if (expr->index.index >= ourNode->size || expr->index.index < 0) {
	ERROR("Vector index out of bounds");
	return;
     }
}

node* get_expr_type(node * expr) {
    innerNode* ourNode;
     switch (expr->kind) {
	case UNARY_EXPRESSION_NODE:
            return expr->unary_expr.type;
	case BINARY_EXPRESSION_NODE:
            return expr->binary_expr.type;
	case BOOL_NODE:
            return new_type_node(BVEC_T, 1);
	case INT_NODE:
            return new_type_node(IVEC_T, 1);
	case FLOAT_NODE:
            return new_type_node(VEC_T, 1);
	case CONSTRUCTOR_NODE:
            return expr->constr.type;
	case IDENT_NODE:
            ourNode = lookup_innerNode(expr->id.name);
            if (ourNode == NULL) {
                return NULL;
            }
            return new_type_node(ourNode->type, ourNode->size);
	case INDEX_NODE:
            ourNode = lookup_innerNode(expr->index.name);
            if (ourNode == NULL) {
                return NULL;
            }
            return new_type_node(ourNode->type, 1);
	case VAR_NODE:
            return get_expr_type(expr->expr.ex);
	case FUNCTION_NODE:
            if (expr->func.ret_type != NULL) {
                return expr->func.ret_type;
            }

            // Fallback in case there was a problem with the arguments 
            switch (expr->func.code) {
                case 0: // Function DP3
                    return new_type_node(IVEC_T, 1);
                case 1: // Function LIT
                    return new_type_node(VEC_T, 4);
                case 2: // Function RSQ
                    return new_type_node(VEC_T, 1);
                default:
                    return NULL;
            }
	default:
            return NULL;
     }
}

int semantic_check(node *ast) {
  insert_outerNode();

  // Insert predefined variables into outer most scope
  insert_innerNode("gl_FragColor", VEC_T, 4, 0, -1);
  insert_innerNode("gl_FragDepth", BVEC_T, 1, 0, -1);

  insert_innerNode("gl_FragCoord", VEC_T, 4, 1, -1);
  insert_innerNode("gl_TexCoord", VEC_T, 4, 1, -1);
  insert_innerNode("gl_Color", VEC_T, 4, 1, -1);
  insert_innerNode("gl_Secondary", VEC_T, 4, 1, -1);
  insert_innerNode("gl_FogFragCoord", VEC_T, 4, 1, -1);

  insert_innerNode("gl_Light_Half", VEC_T, 4, 1, -1);
  insert_innerNode("gl_Light_Ambient", VEC_T, 4, 1, -1);
  insert_innerNode("gl_Material_Shininess", VEC_T, 4, 1, -1);

  insert_innerNode("env1", VEC_T, 4, 1, -1);
  insert_innerNode("env2", VEC_T, 4, 1, -1);
  insert_innerNode("env3", VEC_T, 4, 1, -1);

  traverse_scope(ast);

  remove_outerNode();
  return 0; // failed checks
}
