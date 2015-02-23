#ifndef _SEMANTIC_H
#define _SEMANTIC_H

#include "ast.h"
#include "symbol.h"


int semantic_check( node *ast);
void traverse_declarations_inner(node  * ast);
void traverse_statements_inner(node  * ast);
void traverse_scope(node * ast);
void const_check (node *expr);
void vector_index_check (node * expr);
node* get_expr_type (node * expr);

#endif
