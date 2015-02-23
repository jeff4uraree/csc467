#ifndef _SYMBOL_H
#define _SYMBOL_H

typedef struct innerNode
{
	char *name;
	int type;
	int size;
	int isConst;
	int id;
	struct innerNode *next;
} innerNode;

typedef struct outerNode
{
	struct outerNode *next;
	struct outerNode *prev;
	struct innerNode *innerNodehead;

} outerNode;

void insert_outerNode();
void remove_outerNode();
void insert_innerNode(char* name, int type, int size, int isConst, int id);
innerNode* lookup_innerNode(char* name);
innerNode* lookup_onelevel_innerNode(char* name);



#endif

