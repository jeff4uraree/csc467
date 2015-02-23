
#ifndef AST_H_
#define AST_H_ 1

#include <stdarg.h>

// Dummy node just so everything compiles, create your own node/nodes
//
// The code provided below is an example ONLY. You can use/modify it,
// but do not assume that it is correct or complete.
//
// There are many ways of making AST nodes. The approach below is an example
// of a descriminated union. If you choose to use C++, then I suggest looking
// into inheritance.

// forward declare
struct node_;
typedef struct node_ node;
extern node *ast;

typedef enum {
  UNKNOWN               = 0,

  SCOPE_NODE            = (1 << 0),
  
  EXPRESSION_NODE       = (1 << 2),
  UNARY_EXPRESSION_NODE = (1 << 2) | (1 << 3),
  BINARY_EXPRESSION_NODE= (1 << 2) | (1 << 4),
  BOOL_NODE             = (1 << 2) | (1 << 5), 
  INT_NODE              = (1 << 2) | (1 << 6), 
  FLOAT_NODE            = (1 << 2) | (1 << 7),
  TYPE_NODE             = (1 << 2) | (1 << 8),
  IDENT_NODE            = (1 << 2) | (1 << 9),
  INDEX_NODE            = (1 << 2) | (1 << 10),
  VAR_NODE              = (1 << 2) | (1 << 11),
  FUNCTION_NODE         = (1 << 2) | (1 << 12),
  CONSTRUCTOR_NODE      = (1 << 2) | (1 << 13),
  ARGUMENTS_NODE        = (1 << 2) | (1 << 14),

  STATEMENTS_NODE       = (1 << 1) | (1 << 15),
  IF_STATEMENT_NODE     = (1 << 1) | (1 << 16),
  ASSIGNMENT_NODE       = (1 << 1) | (1 << 17),
  NESTED_SCOPE_NODE     = (1 << 1) | (1 << 18),

  DECLARATIONS_NODE     = (1 << 1) | (1 << 19),
  DECLARATION_NODE      = (1 << 1) | (1 << 20),
  BLNK_STMT_NODE	= (1 << 1) | (1 << 21)

} node_kind;

struct node_ {

  // an example of tagging each node with a type
  node_kind kind;

  union {
    struct {
      // linked lists?
      // NULL if empty
      node *decs;
      node *stmts;
    } scope;

    struct {
        node *dec;
        node *next;
    } declarations;

    struct {
        char *name;
        node *type;
        node *init; // may be NULL
	int isConst;
    } dec;

    struct {
        node *arg;
        node *next;
    } args;

    struct {
        node *ex;
    } expr;

    struct {
        node *stmt;
        node *next;
    } statements;

    struct {
        node *expr;
        node *stmt1;
        node *stmt2; // may be NULL
    } if_st;
  
    struct {
      int op;
      node *right;
      node *type;
    } unary_expr;

    struct {
      int op;
      node *left;
      node *right;
      node *type;
    } binary_expr;

    struct {
        int val;
    } bool_const;

    struct {
        int val;
    } int_const;

    struct {
        float val;
    } float_const;

    struct {
        int base_type;
        int index;
    } type;

    struct {
        char *name;
    } id;

    struct {
        char *name;
        int index;
        node *type;
    } index;

    struct {
        // not sure why var is needed,
        // for now it contains either id or index
        int is_id; // true if ID, false if index
        node *val;
    } var;

    struct {
        node *type;
        node *args;
    } constr;

    struct {
        int code;
        node *args;
    } func;

    struct {
        node *var;
        node *expr;
        node *type;
    } assignment;
  };
};

node *ast_allocate(node_kind type, ...);
void ast_free(node *ast);
void ast_print(node * ast);

#endif /* AST_H_ */
