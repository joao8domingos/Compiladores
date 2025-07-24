#ifndef _SEMANTICS_H
#define _SEMANTICS_H

#define names { "Program", "Function", "Parameters", "Parameter", "Arguments", "Identifier", "Natural", "Decimal", "Call", "If", "Declarations", "VarDeclaration", "VarSpec", "Type", "FuncDeclaration", "FuncBody", "VarsAndStatements", "Statement", "For", "Return", "Print", "ParseArgs", "FuncInvocation", "Expr", "FuncDecl", "FuncHeader", "FuncParams", "VarDecl", "Block", "Assign", "StrLit","ParamDecl", "Or", "And", "Lt", "Gt", "Eq", "Ne", "Le", "Ge", "Plus", "Sub", "Mul", "Mod", "Not", "Div", "String",  "Int", "Float32", "Bool", "Add", "Dummy", "Minus", "Dummy1"}

enum category {
    Program, Function, Parameters, Parameter, Arguments, Identifier, Natural, Decimal, Call, If,
    Declarations, VarDeclaration, VarSpec, Type, FuncDeclaration, FuncBody, VarsAndStatements, Statement, For, Return, Print, ParseArgs, FuncInvocation, Expr, FuncDecl, FuncHeader, FuncParams, VarDecl, Block, Assign, StrLit,ParamDecl,
    Or, And, Lt, Gt, Eq, Ne, Le, Ge, Plus, Sub, Mul, Mod, Not, Div, String, 
    Int, Float32, Bool, Add, Dummy, Minus, Dummy1
};

enum type {
    integer_type, float32_type, bool_type, string_type, no_type,  undef
};





struct node {
    enum category category;
    char *token;
    enum type type;
    int token_line;
    int token_column;
    struct node_list *children;
    int aux_param;
    int call_aux;
    enum type type_call[50];
    enum type type_call_user[50];
    int tam_user;
    int tam;
    int func_0_param;
    int is_var;
    int declared_func;
    int is_func;
    int parentesis;
    //int assign_left;
};

struct node_list {
    struct node *node;
    struct node_list *next;
};

struct scope_list {
    struct symbol_list *scope;
    struct scope_list *next;
    char *token;
};

struct scope_list *add_scope(struct scope_list *head);
struct node *newnode(enum category category, char *token);
void addchild(struct node *parent, struct node *child);
struct node *getchild(struct node *parent, int position);
void show(struct node *node, int depth);
enum type category_type(enum category category);
const char *type_name(enum type type);
void erro_call_print(struct node *expression, struct symbol_list *scope);


int check_program(struct node *program);

struct symbol_list {
    char *identifier;
    enum type type;
    int used;
    int line;
    int column;
    struct node *node;
    struct symbol_list *next;
};

struct symbol_list *insert_symbol(struct symbol_list *symbol_table, char *identifier, enum type type, struct node *node);
struct symbol_list *insert_symbol_varDecl(struct symbol_list *table, char *identifier, enum type type, struct node *node, int line, int column);
struct symbol_list *search_symbol(struct symbol_list *symbol_table, char *identifier);
void show_symbol_table();
void show_scope(struct symbol_list *scope, char *function);

#endif