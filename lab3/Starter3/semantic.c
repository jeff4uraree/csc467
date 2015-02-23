#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol.h"
#include "semantic.h"
#include "parser.tab.h"

int ErrorFlg;

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
    type_node->type.base_type = type->type.base_type;
    type_node->type.index = type->type.index;

    if (ast->kind == UNARY_EXPRESSION_NODE) {
        ast->unary_expr.type = type_node;
    } else if (ast->kind == BINARY_EXPRESSION_NODE) {
        ast->binary_expr.type = type_node;
    } 
}

void check_binary_types(node *expr, node *lhs_type, node *rhs_type) {
    if (lhs_type->type.base_type != rhs_type->type.base_type) {
        printf("Both operands of a binary expression must have the same base type\n");
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
                printf("Operands of %c must be of arithmetic type\n", op);
                return;
            }
            if (left_is_scalar != right_is_scalar) {
                printf("Operands of %c must both be scalar or both vector\n", op);
                return;
            }
            break;
        case '*':
            if (!is_arithmetic) {
                printf("Operands of %c must be of arithmetic type\n", op);
                return;
            }
            break;
        case '/':
        case '^':
            if (!is_arithmetic) {
                printf("Operands of %c must be of arithmetic type\n", op);
                return;
            }
            if (!left_is_scalar || !right_is_scalar) {
                printf("Operands of %c must both be scalar\n", op);
                return;
            }
            break;
        case OR:
        case AND:
            if (is_arithmetic) {
                printf("Operands must be of logical type\n");
                return;
            }
            if (left_is_scalar != right_is_scalar) {
                printf("Operands must both be scalar or both vector\n");
                return;
            }
            break;
        case '<':
        case LEQ:
        case '>':
        case GEQ:
            if (!is_arithmetic) {
                printf("Operands must be of arithmetic type\n");
                return;
            }
            if (!left_is_scalar || !right_is_scalar) {
                printf("Operands must both be scalar\n");
                return;
            }
            break;
        case EQ:
        case NEQ:
            if (!is_arithmetic) {
                printf("Operands must be of arithmetic type\n");
                return;
            }
            if (left_is_scalar != right_is_scalar) {
                printf("Operands must both be scalar or both vector\n");
                return;
            }
            break;
    }

    if (!left_is_scalar && !right_is_scalar && lhs_type->type.index != rhs_type->type.index) {
        printf("Operands must be of the same size when vector\n");
        return;
    }
}

// returns type of expr
node *handle_expr(node * expr) {
    char *name;
    innerNode* symbol;
    node *ret_type;
    node *arg1_type;
    node *arg2_type;


    node_kind kind = expr->kind;
    switch (kind) {
        case CONSTRUCTOR_NODE:
            walk_for_expr(expr->constr.args);
            return expr->constr.type;
        case FUNCTION_NODE:
            walk_for_expr(expr->func.args);
            ret_type = (node *) malloc(sizeof(node *));
            ret_type->kind = TYPE_NODE;
            switch (expr->func.code) {
                case 0: // DP3
		    // TODO: can return different types depending on args
                    //ret_type->type.base_type = IVEC_T;
                    //ret_type->type.index = 4;
		    // printf("%d\n", expr->func.args->args.arg);
	    	    // Check types of function arguments
			    
		    arg1_type = get_expr_type(expr->func.args->args.arg);
		    
		    if (expr->func.args->args.next->args.next != NULL)
		    {
			printf("Error: incorrect number of arguments\n");
			ErrorFlg = 1;
			return NULL;
		    }

		    if (expr->func.args->args.next != NULL)
		    {
			arg2_type = get_expr_type(expr->func.args->args.next->args.arg);
			//printf("Correct number of args\n");
			if (arg1_type->type.base_type == VEC_T && arg2_type->type.base_type == VEC_T)
			{
				ret_type->type.base_type = VEC_T;
				if (arg1_type->type.index == 3 && arg2_type->type.index == 3)
				{
					ret_type->type.index = 3;
				}
				/*
				else
				{
					printf("case1:\n");
					printf("arg1 index = %d\n", arg1_type->type.index);
					printf("arg2 index = %d\n", arg2_type->type.index);
					printf("Error: Incorrect number of indices\n");
					ErrorFlg = 1;
					return NULL;
				} 
				*/
				else if (arg1_type->type.index == 4 && arg2_type->type.index == 4)
				{
					ret_type->type.index = 4;
				}
				else
				{
					printf("case2:\n");
					printf("arg1 index = %d\n", arg1_type->type.index);
					printf("arg2 index = %d\n", arg2_type->type.index);
					printf("Error: Incorrect number of indices\n");
					ErrorFlg = 1;
					return NULL;
				}
			}
			else if (arg1_type->type.base_type == IVEC_T && arg2_type->type.base_type == IVEC_T)
			{
				ret_type->type.base_type = IVEC_T;
				if (arg1_type->type.index == 3 && arg2_type->type.index == 3)
				{
					ret_type->type.index = 3;
				}
				/*
				else
				{
					printf("case3:\n");
					printf("arg1 index = %d\n", arg1_type->type.index);
					printf("arg2 index = %d\n", arg2_type->type.index);
					printf("Error: Incorrect number of indices\n");
					ErrorFlg = 1;
					return NULL;
				}
				*/
				else if (arg1_type->type.index == 4 && arg2_type->type.index == 4)
				{
					ret_type->type.index = 4;
				}
				else
				{
					printf("case4:\n");
					printf("arg1 index = %d\n", arg1_type->type.index);
					printf("arg2 index = %d\n", arg2_type->type.index);
					printf("Error: Incorrect number of indices\n");
					ErrorFlg = 1;
					return NULL;
				}
			}
			// Error: incorrect argument types	
			else
			{
				printf("Error: Arguments of wrong type\n");
				ErrorFlg = 1;
				return NULL;
			}
		    }
		    // error case: incorrect number of arguments
		    else
		    {
			printf("Error: incorrect number of arguments\n");
			ErrorFlg = 1;
			return NULL;
		    }
		
		    break;
                case 1: // LIT
                    ret_type->type.base_type = VEC_T;
                    ret_type->type.index = 4; 

		    arg1_type = get_expr_type(expr->func.args->args.arg);
		    
	    	    // Check types of function arguments
		    if (arg1_type->type.base_type != VEC_T)
		    {
		    	printf("Error: arguments of wrong type\n");
		    	ErrorFlg = 1;
		    	return NULL;
		    }
		    
		    
		    // Check there is a second argument
		    if (expr->func.args->args.next != NULL)
		    {
		    	return NULL;
		    }
		    
		    
		    // Check there is a second argument
		    if (expr->func.args->args.next != NULL)
		    {
			printf("Error: Incorrect number of arguments\n");
			ErrorFlg = 1;
			return NULL;

		    }
		
		    // Check if the argument has the correct number of indices
		    if (arg1_type->type.index != 4)
		    {
			printf("Error: Argument has incorrect number of indices \n");
			ErrorFlg = 1;
			return NULL;
		    }
		  
                    break;
                case 2: // RSQ
		
                    ret_type->type.base_type = VEC_T;
                    ret_type->type.index = 1;

		    arg1_type = get_expr_type(expr->func.args->args.arg);
		    
		    if (arg1_type->type.base_type != VEC_T && arg1_type->type.base_type != IVEC_T)
		    {
			printf("Error: argument 1 of wrong type\n");
			ErrorFlg = 1;
			return NULL;
		    }
		    
		    // Check if there is a second argument
		    if (expr->func.args->args.next != NULL)
		    {
			printf("Error: Incorrect number of arguments\n");
			ErrorFlg = 1;
			return NULL;	
		    }

		    if (arg1_type->type.index != 1)
		    {
			printf("Error: Arguments has incorrect number of indices \n");
			ErrorFlg = 1;
			return NULL;
                    }
		    
                    break;
            }
            return ret_type;
        case UNARY_EXPRESSION_NODE:
            ret_type = handle_expr(expr->unary_expr.right);

            // Check that the expression type is compatible with the op
            if (expr->unary_expr.op == (int) '!' && ret_type != BVEC_T) {
                printf("Only boolean types can be used with !\n");
            } else if (expr->unary_expr.op == UMINUS && ret_type->type.base_type != IVEC_T && ret_type->type.base_type != VEC_T) {
                printf("Only arithmetic types can be used with -\n");
            }

            insert_expr_type(expr, ret_type);
            return ret_type;
        case BINARY_EXPRESSION_NODE:
            ret_type = handle_expr(expr->binary_expr.left);
            arg2_type = handle_expr(expr->binary_expr.right);

            check_binary_types(expr, ret_type, arg2_type);

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
                printf("Using variable in expr which hasn't been declared %s\n", name);
                return;
            }

            return new_type_node(symbol->type, symbol->size);
        case INDEX_NODE:
            name = expr->index.name;
            symbol = lookup_innerNode(name);
            if (symbol == NULL) {
                printf("Using variable in expr which hasn't been declared %s\n", name);
                return;
            }

            return new_type_node(symbol->type, symbol->size);
	
    }
}

void walk_for_expr(node * ast) {
    if (ast == NULL) {
        return;
    }
    innerNode* ourNode;
    innerNode* lhsNode;
    node *rhs_type;
    node_kind kind = ast->kind;
    switch (kind) {
        case DECLARATION_NODE:
            walk_for_expr(ast->dec.init);

            // Check that RHS type matches declared type
            rhs_type = get_expr_type(ast->dec.init);
	    // Check is rhs is ID
	    //printf("%d\n", ast->dec.init->kind);
	    //printf("%d\n", ((1 << 2) | (1 << 11)));
	    
	    //lhsNode = lookup_innerNode(ast->dec.name);
	    if (ast->dec.isConst == 1)
	    {
		printf("Constant variable\n");
	    

	    	if (ast->dec.init->kind == VAR_NODE)
	    	{
		
			printf("We have var node\n");
			ourNode = lookup_innerNode(ast->dec.init->expr.ex->id.name);
			//printf("name of node is: %s\n", ourNode->name);
			// Case 1: variable is not uniform type
			if (strcmp(ourNode->name, "gl_Light_Half") && strcmp(ourNode->name, "gl_Light_Ambient") && strcmp(ourNode->name, "gl_Light_Half") && strcmp(ourNode->name, "env1") && strcmp(ourNode->name, "env2") && strcmp(ourNode->name, "env3"))
			{
				printf("Error: Assigned non constant value to constant declared variable\n");
				ErrorFlg = 1;
			}
	    	}
	    	// if it is not a variable then it must be a literal 
	    }
            if (ast->dec.type->type.base_type != rhs_type->type.base_type || ast->dec.type->type.index != rhs_type->type.index) {
                printf("Initial value type does not match declared variable's type\n");
            }
	    //if (ast->dec.
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
        printf("Indexing into variable which hasn't been declared %s\n", name);
        return;
    }

    ast->index.type = new_type_node(symbol->type, 1);
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
        printf("Assigning to variable which hasn't been declared %s\n", name);
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
	printf("Assigning wrong type to variable\n");
	return;
    }

    ast->assignment.type = new_type_node(symbol->type, 1);
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

void traverse_declarations_inner(node  * ast)
{
    innerNode* ourNode;
	innerNode* target;
    if (ast->kind != DECLARATIONS_NODE || ast->declarations.next == NULL) {
        if (ast->declarations.dec != NULL) {
			target = lookup_onelevel_innerNode(ast->declarations.dec->dec.name);
			// Check for duplicate declaration calls, if we are able to find another node in symbol table with same name, return and set errorflag
			if (target != NULL)
			{
				printf("Found a duplicate!\n");
				// Set the global
				ErrorFlg = 1;
				return;
			}

			walk_expr_for_index(ast->declarations.dec->dec.init);
			walk_for_expr(ast->declarations.dec);
			insert_innerNode(ast->declarations.dec->dec.name, ast->declarations.dec->dec.type->type.base_type, ast->declarations.dec->dec.type->type.index, ast->declarations.dec->dec.isConst);
		}
		return;
    }

    traverse_declarations_inner(ast->declarations.next);

	target = lookup_onelevel_innerNode(ast->declarations.dec->dec.name);
    if (target != NULL)
    {  
	printf("Found a duplicate!\n");
	// Set the global
	ErrorFlg = 1;
	return;
    }

    walk_expr_for_index(ast->declarations.dec->dec.init);
    walk_for_expr(ast->declarations.dec);
    insert_innerNode(ast->declarations.dec->dec.name, ast->declarations.dec->dec.type->type.base_type, ast->declarations.dec->dec.type->type.index, ast->declarations.dec->dec.isConst);
}

void traverse_statements_inner(node * ast)
{
   if (ast->statements.stmt == NULL) {
	return;
   }
    node* type;
    innerNode* ourNode;
    switch (ast->kind) {
	case IF_STATEMENT_NODE:
		printf("INSIDE IF STMT\n");
		walk_expr_for_index(ast->if_st.expr);
		walk_for_expr(ast->if_st.expr);
		type = get_expr_type(ast->if_st.expr);
		if (type->type.base_type != BVEC_T || type->type.index != 1)
		{
			printf("if condition is not boolean type\n");
		}

		traverse_statements_inner(ast->if_st.stmt1);
		traverse_statements_inner(ast->if_st.stmt2);
	break;

	case NESTED_SCOPE_NODE:
		insert_outerNode();
		traverse_scope(ast->expr.ex);
		printf("NESTED SCOPE\n");
		remove_outerNode();
	break;

	case STATEMENTS_NODE:
    	    printf("RECURSIVE CALL!\n");
	    traverse_statements_inner(ast->statements.next);
	    traverse_statements_inner(ast->statements.stmt);

	break;

	default:
		walk_expr_for_index(ast);
		walk_for_expr(ast);
		walk_for_asn(ast);
    }
}

void traverse_scope(node * ast) {
    if (ast->scope.decs != NULL)
    {
    	traverse_declarations_inner(ast->scope.decs);
    }
    if (ast->scope.stmts != NULL)
    {
    	traverse_statements_inner(ast->scope.stmts);
    }
}

void const_check (node *expr) {
     innerNode* ourNode;

     // Case where we are getting name of identifier
     if (expr->assignment.var->kind == IDENT_NODE)
     {
	     ourNode = lookup_innerNode(expr->assignment.var->id.name);
     }

     // Case where we are getting name of index
     if (expr->assignment.var->kind == INDEX_NODE)
     {
	     ourNode = lookup_innerNode(expr->assignment.var->index.name);
     }

     
     if (ourNode != NULL)
     {
	  if (ourNode->isConst == 1)
	  {
		printf("made assignment on const variable\n");
		ErrorFlg = 1;
		return;
	  }
     }


}

void vector_index_check(node * expr)
{
     innerNode* ourNode;	
     ourNode = lookup_innerNode(expr->index.name);
     if (expr->index.index >= ourNode->size || expr->index.index < 0)
     {
	printf("Vector index out of bound\n");
	ErrorFlg = 1;
	return;
     }
}

node* get_expr_type(node * expr)
{
		innerNode* ourNode;
     switch (expr->kind)
     {
	case UNARY_EXPRESSION_NODE:
		printf("I am in here\n");
		return expr->unary_expr.type;
	case BINARY_EXPRESSION_NODE:
		printf("I am in here\n");
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
		printf("I am in here\n");
		ourNode = lookup_innerNode(expr->id.name);
		return new_type_node(ourNode->type, ourNode->size);
	case INDEX_NODE:
		printf("I am in here\n");
		ourNode = lookup_innerNode(expr->index.name);
		return new_type_node(ourNode->type, ourNode->size);
	case VAR_NODE:
		return get_expr_type(expr->expr.ex);
	case FUNCTION_NODE:
		printf("I am in here\n");
		switch(expr->func.code)
		{
			// Function DP3
			case 0: 
	
				// TODO check arguments type
				return new_type_node(IVEC_T, 1);
			// Function LIT
			case 1:
				return new_type_node(VEC_T, 4);
			// Function RSQ
			case 2:
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
  insert_innerNode("gl_FragColor", VEC_T, 4, 0);
  insert_innerNode("gl_FragDepth", BOOL_T, 1, 0);
  insert_innerNode("gl_FragCoord", VEC_T, 4, 0);
  insert_innerNode("gl_TexCoord", VEC_T, 4, 1);
  insert_innerNode("gl_Color", VEC_T, 4, 1);
  insert_innerNode("gl_Secondary", VEC_T, 4, 1);
  insert_innerNode("gl_FogFragCoord", VEC_T, 4, 1);
  insert_innerNode("gl_Light_Half", VEC_T, 4, 1);
  insert_innerNode("gl_Light_Ambient", VEC_T, 4, 1);
  insert_innerNode("gl_Material_Shininess", VEC_T, 4, 1);
  insert_innerNode("env1", VEC_T, 4, 1);
  insert_innerNode("env2", VEC_T, 4, 1);
  insert_innerNode("env3", VEC_T, 4, 1);

  traverse_scope(ast);
  innerNode* ourNode;  

  remove_outerNode();
  return 0; // failed checks
}
