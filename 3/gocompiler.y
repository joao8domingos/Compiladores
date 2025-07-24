//João Domingos 2022215704
//Miguel Domingos 2022216083
//110 PONTOS

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


enum category {
   Program, Function, Parameters, Parameter, Arguments, Identifier, Natural, Decimal, Call, If,
   Declarations, VarDeclaration, VarSpec, Type, FuncDeclaration, FuncBody, VarsAndStatements, Statement, For, Return, Print, ParseArgs, FuncInvocation, Expr, FuncDecl, FuncHeader, FuncParams, VarDecl, Block, Assign, StrLit,ParamDecl,
   Or, And, Lt, Gt, Eq, Ne, Le, Ge, Plus, Sub, Mul, Mod, Not, Div, String,
   Int, Float32, Bool, Add, Dummy, Minus, Dummy1
};

enum type {
   integer_type, float32_type, bool_type, string_type, no_type, undef
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

extern int yylineno;
extern const char *type_name(enum type type);

int rep = 0;
int count_block = 0;
char *var_type;
int conta_block = 0;

struct node *newnode(enum category category, char *token);
void addchild(struct node *parent, struct node *child);
int count_children(struct node *parent);

int yylex(void);
void yyerror(char *s);
void show(struct node *node, int depth);
void show_s(struct node *node, int depth);
void add_node_to_list(struct node_list **list, struct node *new_node);
void add_node_to_all(struct node_list *list, struct node *new_node);
void addchild_begin(struct node *parent, struct node *child);
int has_child_of_type(struct node *parent);
struct node *getchild(struct node *parent, int position);

#define names { "Program", "Function", "Parameters", "Parameter", "Arguments", "Identifier", "Natural", "Decimal", "Call", "If", "Declarations", "VarDeclaration", "VarSpec", "Type", "FuncDeclaration", "FuncBody", "VarsAndStatements", "Statement", "For", "Return", "Print", "ParseArgs", "FuncInvocation", "Expr", "FuncDecl", "FuncHeader", "FuncParams", "VarDecl", "Block", "Assign", "StrLit","ParamDecl", "Or", "And", "Lt", "Gt", "Eq", "Ne", "Le", "Ge", "Plus", "Sub", "Mul", "Mod", "Not", "Div", "String", "Int", "Float32", "Bool", "Add", "Dummy", "Minus", "Dummy1"}

char *list_cat[] = {
   "Program", "Function", "Parameters", "Parameter", "Arguments", "Identifier", "Natural", "Decimal", "Call", "If",
   "Declarations", "VarDeclaration", "VarSpec", "Type", "FuncDeclaration", "FuncBody", "VarsAndStatements", "Statement", "For", "Return", "Print", "ParseArgs", "FuncInvocation", "Expr", "FuncDecl", "FuncHeader", "FuncParams", "VarDecl", "Block", "Assign", "StrLit","ParamDecl",
   "Or", "And", "Lt", "Gt", "Eq", "Ne", "Le", "Ge", "Plus", "Sub", "Mul", "Mod", "Not", "Div", "String",
   "Int", "Float32", "Bool", "Add", "Dummy", "Minus", "Dummy1"


};

struct node *program;
char type_var[1024];
struct node_list *var_list;
int var_count = 0;


int count_children(struct node * node) {
   int count = 0;
   struct node_list * child = node->children->next;
   while (child && count < 2) {
       //if (child->node != NULL){
           if ((child->node->category != Dummy && child->node->category != Dummy1)|| count_children(child->node)) count++;
           child = child->next;
       //}
   }
   return count;
}



// Função para adicionar um nó à lista encadeada
void add_node_to_list(struct node_list **list, struct node *new_node) {
   //int count1 = 0;
   struct node_list *new_list_node = malloc(sizeof(struct node_list));
   new_list_node->node = new_node;
   new_list_node->next = NULL;

   if (*list == NULL) {
       // Se a lista estiver vazia, o novo nó se torna o primeiro nó
       *list = new_list_node;
   } else {
       // Caso contrário, percorre a lista até o final
       struct node_list *current = *list;
       while (current->next != NULL) {
           /*
           if (has_child_of_type(current->node)) {
               count1 = 1;
               break;

           }   
           */
           current = current->next;
       }
       // Adiciona o novo nó ao final da lista
       //if (count1 == 0){
           current->next = new_list_node;
       //}
   }
}


// Função para verificar se algum dos filhos do nó é do tipo especificado
int has_child_of_type(struct node *parent) {
   if (parent->children == NULL) {
       return 0;
   }

   struct node_list *current = parent->children;

   while (current != NULL) {
       if (current->node != NULL) {
           //printf("Category: %d\n", current->node->category);
           if (current->node->category == 48 || current->node->category == 47 || current->node->category == 49 || current->node->category == 46) {
           return 1;
       }
       }
       current = current->next;
   }
   return 0;

}


void add_node_to_all(struct node_list *list, struct node *new_node) {
   struct node_list *current = list;
   while (current != NULL) {
       if (!has_child_of_type(current->node)) {
           addchild_begin(current->node, new_node);
       }
       //addchild_begin(current->node, new_node);
       current = current->next;
   }
}

// get a pointer to a specific child, numbered 0, 1, 2, ...
struct node *getchild(struct node *parent, int position) {
   struct node_list *children = parent->children;
   while((children = children->next) != NULL)
       if(position-- == 0)
           return children->node;
   return NULL;
}

// category names #defined in ast.h
char *category_name[] = names;


// Function to recursively traverse the AST and show its content
void show(struct node *node, int depth) {
   if (node == NULL) return;

   if(node->category != Dummy && node->category != Dummy1){
       // Print the current node's category and token with indentation
       for (int i = 0; i < depth ; i++) {
           printf("..");
       }


       printf("%s", list_cat[node->category]);  // Changed %u to %d for the enum category
       if (node->token != NULL) {
           printf("(%s)", node->token);
       }
       //printf("   aux: %d", node->aux);
       printf("\n");
   }
   else if (node->category == Dummy){
       depth = depth -2;
   }
   else if (node->category == Dummy1){
       depth = depth - 1;
   }


   // Traverse the linked list of children and call show on each child node
   struct node_list *child = node->children;
   //depth = depth - node->aux;
   while (child != NULL) {
       if (child->node != NULL)
           show(child->node, depth + 1);  // Recursive call for each child node
       child = child->next;           // Move to the next child in the list
   }

}



void show_s(struct node *node, int depth) {
    if (node == NULL) return;
    //printf("TYPE: %s\n", type_name(node->type));
    if (node->category != Dummy && node->category != Dummy1) {
        // Imprime a categoria e o token do nó atual com indentação
        for (int i = 0; i < depth; i++) {
            printf("..");
        }

        printf("%s", list_cat[node->category]); // Nome da categoria

        if (node->token != NULL) {
            printf("(%s)", node->token); // Token associado, se existir
        }

        if (node->call_aux == 1) {
            if (node->category != VarDecl && node->category != ParamDecl) {
                if(strcmp(type_name(node->type),"undef") != 0){
                    if (node->tam > 0) {
                        for (int i = 0; i < node->tam; i++) {
                            if (i == 0 && node->tam == 1) {
                                if (node->type_call[i] == integer_type) {
                                    printf(" - (int)");
                                } else if (node->type_call[i] == float32_type) {
                                    printf(" - (float32)");
                                } else if (node->type_call[i] == bool_type) {
                                    printf(" - (bool)");
                                } else if (node->type_call[i] == string_type) {
                                    printf(" - (string)");
                                }
                            }
                            else if (i == 0){
                                if (node->type_call[i] == integer_type) {
                                    printf(" - (int");
                                } else if (node->type_call[i] == float32_type) {
                                    printf(" - (float32");
                                } else if (node->type_call[i] == bool_type) {
                                    printf(" - (bool");
                                } else if (node->type_call[i] == string_type) {
                                    printf(" - (string");
                                }
                            }
                            else if (i == node->tam - 1){
                                if (node->type_call[i] == integer_type) {
                                    printf(",int)");
                                } else if (node->type_call[i] == float32_type) {
                                    printf(",float32)");
                                } else if (node->type_call[i] == bool_type) {
                                    printf(",bool)");
                                } else if (node->type_call[i] == string_type) {
                                    printf(",string)");
                                }
                            }
                            else {
                                if (node->type_call[i] == integer_type) {
                                    printf(",int");
                                } else if (node->type_call[i] == float32_type) {
                                    printf(",float32");
                                } else if (node->type_call[i] == bool_type) {
                                    printf(",bool");
                                } else if (node->type_call[i] == string_type) {
                                    printf(",string");
                                }
                            }
                        }
                    }
                    else if (node->func_0_param == 1) {
                        printf(" - ()");
                    }
                    else if (node->is_func == 1){
                        printf(" - undef");
                    }
                    else {

                            if (node->type == integer_type) {
                                printf(" - (int)");
                            } else if (node->type == float32_type) {
                                printf(" - (float32)");
                            } else if (node->type == bool_type) {
                                printf(" - (bool)");
                            } else if (node->type == string_type) {
                                printf(" - (string)");
                            }
                            else if (node->type == no_type) {
                                printf(" - ()");
                            }


                    }
                }
                else if( strcmp(type_name(node->type),"undef") == 0){
                    //printf("ENTROU UNDEF\n");
                    //printf("\n---> %s\n",type_name(node->type));
                    printf(" - undef");
                }
            }
        }
        else {
            // Adiciona o tipo do nó
            if (node->category != VarDecl && node->category != ParamDecl) {
                //printf("\n%s\n",type_name(node->type));
                if (node->type == integer_type) {
                    printf(" - int");
                } else if (node->type == float32_type) {
                    printf(" - float32");
                } else if (node->type == bool_type) {
                    printf(" - bool");
                } else if (node->type == string_type) {
                    printf(" - string");
                }else if(strcmp(type_name(node->type),"undef") == 0){
                    printf(" - undef");
                }
            }
        }
        printf("\n");
    }

    // Ajusta a profundidade para "Dummy" e "Dummy1"
    if (node->category == Dummy) {
        depth -= 2;
    } else if (node->category == Dummy1) {
        depth -= 1;
    }

    // Percorre a lista encadeada de filhos e chama a função show_s recursivamente
    struct node_list *child = node->children;
    while (child != NULL) {
        if (child->node != NULL) {
            show_s(child->node, depth + 1); // Chamada recursiva para o próximo filho
        }
        child = child->next; // Move para o próximo filho na lista
    }
}




void addchild_begin(struct node *parent, struct node *child) {
   struct node_list *new_child = malloc(sizeof(struct node_list));
   new_child->node = child;
   new_child->next = parent->children;
   parent->children = new_child;
}

// create a node of a given category with a given lexical symbol
struct node *newnode(enum category category, char *token) {
   struct node *new = malloc(sizeof(struct node));
   new->category = category;
   new->token = token;
   new->type = no_type;
   new->is_var = 1;
   new->declared_func = 0;
   new->is_func = 0;
   new->parentesis = 0;
   //new->assign_left = 0;
   new->children = malloc(sizeof(struct node_list));
   new->children->node = NULL;
   new->children->next = NULL;
   return new;
}

// append a node to the list of children of the parent node
void addchild(struct node *parent, struct node *child) {
   struct node_list *new = malloc(sizeof(struct node_list));
   new->node = child;
   new->next = NULL;
   struct node_list *children = parent->children;
   while(children->next != NULL)
       children = children->next;
   children->next = new;
}


%}

%token IF ELSE SEMICOLON COMMA BLANKID ASSIGN STAR DIV MINUS PLUS EQ GE GT LBRACE LE LPAR LSQ LT MOD NE NOT AND OR RBRACE RPAR RSQ PACKAGE RETURN FOR VAR PRINT PARSEINT FUNC CMDARGS FLOAT32 BOOL INT STRING
%token<lexeme> IDENTIFIER NATURAL DECIMAL STRLIT RESERVED
%type <node> program Declarations VarDeclaration VarSpec Type FuncDeclaration Parameters FuncBody VarsAndStatements Statement ParseArgs FuncInvocation Expr OptType StatementList ExprList ElseOpt ExprOpt PrintArg//StatementList1//OptParameters IdentifierList ->tirei

%locations
%{
#define LOCATE(node, line, column) { node->token_line = line; node->token_column = column; }
%}

%left LOW
%left OR
%left AND
%left EQ NE LT LE GT GE
%left PLUS MINUS
%left STAR DIV MOD
%left '(' ')'
%left NOT
%nonassoc IF
%nonassoc ELSE


%union {
   char *lexeme;
   struct node *node;
}


/* START grammar rules section -- BNF grammar */

%%

program: PACKAGE IDENTIFIER SEMICOLON Declarations {
                                               //$$ =  program = newnode(Program, NULL);
                                               //printf("1. Program\n");
                                               //addchild($$, $4);
                                               $$=$4;
                                               //LOCATE($2, @2.first_line, @2.first_column);
                                               }
       | PACKAGE IDENTIFIER SEMICOLON {
                                       $$ =  program = newnode(Program, NULL);
                                       //printf("2. Program\n");
                                       //LOCATE($2, @2.first_line, @2.first_column);
                                       }
   ;

Declarations: VarDeclaration SEMICOLON{
                                       //$$ = $1;
                                       $$ =  program = newnode(Program, NULL);
                                       addchild($$,$1);
                                       //$$->aux = -1;


                                       }
   | FuncDeclaration SEMICOLON{

                                       //$$ = $1;
                                       $$ =  program = newnode(Program, NULL);
                                       addchild($$,$1);
                                       //$$->aux = -1;

                                       //printf("aux = -1\n");

                                       }
   | Declarations VarDeclaration SEMICOLON{
                                       $$ = $1;
                                       addchild($$, $2);
                                       }
   | Declarations FuncDeclaration SEMICOLON{
                                       $$ = $1;
                                       //$$->aux = 1;
                                       addchild($$, $2);
                                       //$2->aux = 1;
                                       }
   ;



VarDeclaration: VAR VarSpec {
                                       //$$ = newnode(VarDecl, NULL);
                                       //addchild($$, $2);
                                       $$ = $2;
                                       }
   | VAR LPAR VarSpec SEMICOLON RPAR {
                                       //$$ = newnode(VarDecl, NULL);
                                       //addchild($$, $3);
                                       $$ = $3;
                                       //printf("1. VarDecl --- VarSpec\n");
                                       }
   ;
VarSpec: IDENTIFIER OptType Type {
                                       $$ = newnode(VarDecl, NULL);
                                       //if(var_type != NULL){
                                           //addchild($$, newnode(var_type,NULL));
                                       //}
                                       //else{
                                       addchild($$,$3);
                                       //}
                                       addchild($$,newnode(Identifier,$1));
                                       addchild($$,$2);
                                       //printf("VarSpec: %d  %d\n", @1.first_line, @1.first_column);
                                       LOCATE(getchild($$, 1), @1.first_line, @1.first_column);

                               }
   | IDENTIFIER Type {
                       $$ = newnode(VarDecl,NULL);
                       addchild($$,$2);
                       addchild($$,newnode(Identifier,$1));
                       //printf("VarSpec: %d  %d\n", @1.first_line, @1.first_column);
                       LOCATE(getchild($$, 1), @1.first_line, @1.first_column);
                       }

   ;

OptType: COMMA IDENTIFIER {
                                       struct node *vardecl = newnode(VarDecl, NULL);
                                       struct node *dummy = newnode(Dummy, NULL);

                                       addchild(vardecl, newnode(Identifier, $2));
                                       add_node_to_list(&var_list, vardecl);
                                       addchild(dummy, vardecl);
                                       //printf("TYPE_VAR: %s\n", type_var);
                                       $$ = dummy;
                                       //printf("OptType: %d  %d\n", @2.first_line, @2.first_column);
                                       LOCATE(getchild(vardecl, 0), @2.first_line, @2.first_column);

}
   | OptType COMMA IDENTIFIER {
                                       $$ = $1;
                                       struct node *vardecl = newnode(VarDecl, NULL);

                                       addchild(vardecl, newnode(Identifier, $3));
                                       add_node_to_list(&var_list, vardecl);

                                       //addchild(dummy, vardecl);
                                       //addchild($$, dummy);
                                       addchild($$, vardecl);
                                       //addchild(dummy1, vardecl);
                                       //addchild($$, newnode(Identifier, $3));
                                       //printf("TYPE_VAR: %s\n", type_var);
                                       //printf("OptType: %d  %d\n", @3.first_line, @3.first_column);
                                       LOCATE(getchild(vardecl, 0), @3.first_line, @3.first_column);

                                       }
   ;
Type: INT {
                                       $$ = newnode(Int, NULL);
                                       add_node_to_all(var_list, $$);
                                       strcpy(type_var, "Int");
                                       //strcpy(var_type, Int);
                                       //printf("ENTROU NO TYPE INT!\n");

                                       }
   | FLOAT32 {
                                       $$ = newnode(Float32, NULL);
                                       add_node_to_all(var_list, $$);
                                       strcpy(type_var, "Float32");
                                       //printf("entrou float32!\n");
                                       //strcpy(var_type, Float32);

                                       }
   | BOOL {
                                       $$ = newnode(Bool, NULL);
                                       add_node_to_all(var_list, $$);
                                       strcpy(type_var, "Bool");
                                       //strcpy(var_type, Bool);

                                       }
   | STRING {
                                       $$ = newnode(String, NULL); //meti Strlit estrava String antes
                                       add_node_to_all(var_list, $$);
                                       strcpy(type_var, "String");

                                       //strcpy(var_type, String);
                                       }
   ;

FuncDeclaration: FUNC IDENTIFIER LPAR RPAR FuncBody {
                                                   $$ = newnode(FuncDecl, NULL);
                                                   struct node *funcHeader = newnode(FuncHeader, NULL);
                                                   struct node *funcParams = newnode(FuncParams, NULL);
                                                   struct node *iden = newnode(Identifier, $2);
                                                   addchild(funcHeader, iden);
                                                   addchild(funcHeader, funcParams);
                                                   addchild($$, funcHeader);

                                                   if($5 != NULL){
                                                       addchild($$, $5);
                                                   }
                                                   //printf("FuncDeclaration: %d  %d\n", @2.first_line, @2.first_column);
                                                   LOCATE(getchild(funcHeader, 0), @2.first_line, @2.first_column);
                                                   //printf("ENTROU2\n");
                                       }

   |FUNC IDENTIFIER LPAR RPAR Type FuncBody {
                                       $$ = newnode(FuncDecl, NULL);
                                       struct node *funcHeader = newnode(FuncHeader, NULL);
                                       struct node *funcParams = newnode(FuncParams, NULL);
                                       struct node *iden = newnode(Identifier, $2);
                                       addchild(funcHeader, iden);
                                       if($5!=NULL){ //-------------------------------------------------------------->troquei o if de lugar e o que estava dentro dele
                                           addchild(funcHeader, $5);
                                       }
                                       addchild(funcHeader, funcParams);
                                       addchild($$, funcHeader);

                                       if($6 != NULL){
                                           addchild($$, $6);
                                       }
                                       //printf("Entrou X\n");
                                       //printf("FuncDeclaration: %d  %d\n", @2.first_line, @2.first_column);
                                       LOCATE(getchild(funcHeader, 0), @2.first_line, @2.first_column);
                                       }
   | FUNC IDENTIFIER LPAR Parameters RPAR Type FuncBody {
                                       $$ = newnode(FuncDecl, NULL);
                                       //printf("3. FuncDecl\n");
                                       struct node *funcHeader = newnode(FuncHeader, NULL);
                                       //struct node *funcParams = newnode(FuncParams, NULL);
                                       struct node *iden = newnode(Identifier, $2);
                                       addchild(funcHeader, iden);
                                       if($6 != NULL){ //-------------------------------------------------------------->troquei o if de lugar e o que estava dentro dele
                                           addchild(funcHeader, $6);
                                       }
                                       //printf("4. Identifier\n");
                                       //addchild(funcHeader, funcParams); -------------------------------------------->
                                       //printf("5. FuncParams\n");
                                       if($4 != NULL){
                                           addchild(funcHeader, $4);
                                       }

                                       //printf("6. Parameters\n");
                                       addchild($$, funcHeader);
                                       //printf("7. FuncHeader\n");

                                       //printf("8. FuncBody\n");
                                       if($7 != NULL){
                                           addchild($$, $7);
                                       }
                                       //printf("FuncDeclaration: %d  %d\n", @2.first_line, @2.first_column);
                                       //printf("9. FuncDeclaration\n");
                                       LOCATE(getchild(funcHeader, 0), @2.first_line, @2.first_column);
                                       }
   | FUNC IDENTIFIER LPAR Parameters RPAR FuncBody {
                                       $$ = newnode(FuncDecl, NULL);
                                       struct node *funcHeader = newnode(FuncHeader, NULL);
                                       //struct node *funcParams = newnode(FuncParams, NULL);
                                       struct node *iden = newnode(Identifier, $2);
                                       //struct node *funcParams = newnode(FuncParams, NULL);

                                       if($4 != NULL){
                                           //addchild(funcParams, $4);

                                       }

                                       addchild(funcHeader, iden);
                                       addchild(funcHeader, $4);
                                       //addchild(funcHeader, $4);


                                       addchild($$, funcHeader);
                                       if($6 != NULL){
                                           addchild($$, $6);
                                       }
                                       //printf("10.fUNCdECLARATION!\n");
                                       //printf("FuncDeclaration: %d  %d\n", @2.first_line, @2.first_column);
                                       LOCATE(getchild(funcHeader, 0), @2.first_line, @2.first_column);
                                       }

   ;

Parameters: Parameters COMMA IDENTIFIER Type   {
                                       $$ = $1;  // Continuamos a usar o nó FuncParams anterior
                                       struct node *param = newnode(ParamDecl, NULL);
                                       addchild(param, $4);
                                       addchild(param, newnode(Identifier, $3));
                                       addchild($$, param);
                                       //printf("Parameters: %d  %d\n", @3.first_line, @3.first_column);
                                       LOCATE(getchild(param, 1), @3.first_line, @3.first_column);
                                       //printf("Adicionado novo parâmetro!\n");
                                       }
       | IDENTIFIER Type {
                           $$ = newnode(FuncParams, NULL);  // FuncParams será o nó raiz dos parâmetros

                           struct node *param = newnode(ParamDecl, NULL);
                           addchild(param, $2);
                           addchild(param, newnode(Identifier, $1));
                           addchild($$, param);
                           //printf("Parameters: %d  %d\n", @1.first_line, @1.first_column);
                           LOCATE(getchild(param, 1), @1.first_line, @1.first_column);
                           //printf("PARAMETERS ENTROU!\n");
                       }
   ;


FuncBody: LBRACE VarsAndStatements RBRACE {
                                           $$ = newnode(FuncBody,NULL);
                                           if($2 != NULL){
                                               addchild($$,$2);
                                           }

                                           //printf("Entrou3\n");
                                       }
   ;


VarsAndStatements:  /* empty */
                   {
                       $$ = newnode(Dummy1, NULL);
                   }
               |   VarsAndStatements VarDeclaration SEMICOLON
                   {
                       $$ = $1;
                       addchild($$, $2);
                   }
               |   VarsAndStatements Statement SEMICOLON
                   {
                       $$ = $1;
                       addchild($$, $2);
                   }
               |   VarsAndStatements SEMICOLON
                   {
                       $$ = $1;
                   }
               ;


Statement:          IDENTIFIER ASSIGN Expr
                   {
                       $$ = newnode(Assign, NULL);
                       addchild($$, newnode(Identifier, $1));
                       addchild($$, $3);
                       //printf("Statement: %d  %d\n", @1.first_line, @1.first_column);
                       //LOCATE(getchild($$, 0), @1.first_line, @1.first_column);
                       LOCATE($$, @2.first_line, @2.first_column);
                       LOCATE(getchild($$, 0), @1.first_line, @1.first_column);
                   }
               |   LBRACE StatementList RBRACE
                   {
                       /*TODO: check if number of blocks > 1*/

                       //printf("Number of children: %d\n", count_children($2));
                       if (count_children($2) > 1) {
                           $$ = newnode(Block, NULL);
                           addchild($$, $2);
                       } else {
                           $$ = $2;
                       }
                       //make_block($$);
                   }
               |   IF Expr LBRACE StatementList RBRACE ElseOpt
                   {
                       $$ = newnode(If, NULL);
                       //create a block for if, and other for else.
                       struct node * ifs = newnode(Block, NULL);
                       struct node * elses = newnode(Block, NULL);
                       addchild($$, $2);
                       addchild($$, ifs);
                       addchild($$, elses);
                       addchild(ifs, $4);
                       addchild(elses, $6);
                       if (getchild($$, 0)->parentesis == 0){
                        LOCATE($$, @2.first_line, @2.first_column);
                       }
                       else {
                        LOCATE($$, @2.first_line, @2.first_column + 1);
                       }
                       //LOCATE(getchild($$, 0), @2.first_line, @2.first_column);


                   }
               |   FOR ExprOpt LBRACE StatementList RBRACE
                   {
                       $$ = newnode(For, NULL);
                       addchild($$, $2);
                       struct node * afor = newnode(Block, NULL);
                       addchild($$, afor);
                       addchild(afor, $4);
                       //LOCATE(getchild($$, 0), @2.first_line, @2.first_column);
                       if (getchild($$, 0)->parentesis == 0){
                        LOCATE($$, @2.first_line, @2.first_column);
                       }
                       else {
                        LOCATE($$, @2.first_line, @2.first_column + 1);
                       }
                       //LOCATE($$, @2.first_line, @2.first_column);
                       
                   }
               |   RETURN ExprOpt
                   {
                       $$ = newnode(Return, NULL);
                       addchild($$, $2);
                       /*
                       if ($2 != NULL) {
                           LOCATE(getchild($$, 0), @2.first_line, @2.first_column);
                       } else {
                            printf("RETURN!!!!\n");
                           LOCATE($$, @1.first_line, @1.first_column);
                       }
                       */

                      if (getchild($$, 0)->category == Dummy1){
                           LOCATE($$, @1.first_line, @1.first_column);
                       }
                       else{
                           LOCATE($$, @2.first_line, @2.first_column);
                       }
                       
                   }
               |   FuncInvocation
                   {
                       $$ = $1;
                   }
               |   ParseArgs
                   {
                       $$ = $1;
                   }
               |   PRINT LPAR PrintArg RPAR
                   {
                       $$ = newnode(Print, NULL);
                       addchild($$, $3);
                       LOCATE(getchild($$, 0), @3.first_line, @3.first_column);
                   }
               |   error
                   {
                       $$ = newnode(Dummy, NULL);
                   }
               ;

StatementList:         /* empty */
                   {
                       $$ = newnode(Dummy1, NULL);
                   }
               |   StatementList Statement SEMICOLON
                   {
                       $$ = $1;
                       addchild($$, $2);
                   }
               ;

ElseOpt:            /* empty */
                   { $$ = newnode(Dummy1, NULL); }
               |   ELSE LBRACE StatementList RBRACE
                   { $$ = $3; }
               ;

ExprOpt:            /* empty */
                   { $$ = newnode(Dummy1, NULL); }
               |   Expr
                   { $$ = $1; }
               ;
PrintArg:           Expr
                   { $$ = $1; }
               |   STRLIT
                   { $$ = newnode(StrLit, $1); }

ParseArgs: IDENTIFIER COMMA BLANKID ASSIGN PARSEINT LPAR CMDARGS LSQ Expr RSQ RPAR {
                                               $$ = newnode(ParseArgs, NULL);
                                               addchild($$,newnode(Identifier, $1));
                                               addchild($$, $9);
                                               //printf("ParseArgs: %d  %d\n", @1.first_line, @1.first_column);
                                               LOCATE(getchild($$, 0), @5.first_line, @5.first_column);
           }
       | IDENTIFIER COMMA BLANKID ASSIGN PARSEINT LPAR error RPAR {
                                               //yyerror("syntax error");
                                               //$$ = newnode(ParseArgs, NULL);
                                               $$ = newnode(Dummy1, NULL);
                                               addchild($$,newnode(Identifier, $1));
                                               //yyerrok; //yyclearin;
           }
   ;


FuncInvocation: IDENTIFIER LPAR RPAR {
                                               $$ = newnode(Call, NULL);
                                               addchild($$,newnode(Identifier, $1));
                                               //printf("FuncInovation->1\n");
                                               //printf("FuncInvocation: %d  %d\n", @1.first_line, @1.first_column);
                                               LOCATE(getchild($$, 0), @1.first_line, @1.first_column);
                                               LOCATE($$, @1.first_line, @1.first_column);
                                   }
   |IDENTIFIER LPAR Expr ExprList RPAR {
                                               $$ = newnode(Call, NULL);
                                               addchild($$,newnode(Identifier, $1));
                                               addchild($$, $3);
                                               addchild($$, $4);
                                               //printf("FuncInovation->2\n");
                                               //printf("FuncInvocation: %d  %d\n", @1.first_line, @1.first_column);
                                               LOCATE(getchild($$, 0), @1.first_line, @1.first_column);
                                               LOCATE($$, @1.first_line, @1.first_column);

   }

   | IDENTIFIER LPAR Expr RPAR {  $$ = newnode(Call, NULL);
                                   addchild($$,newnode(Identifier, $1));
                                   addchild($$, $3);
                                   //printf("FuncInvocation: %d  %d\n", @1.first_line, @1.first_column);
                                   //LOCATE(getchild($$, 0), @1.first_line, @1.first_column);
                                   LOCATE(getchild($$, 0), @1.first_line, @1.first_column);
                                   LOCATE($$, @1.first_line, @1.first_column);

           }
   | IDENTIFIER LPAR error RPAR {

       //yyerror("syntax error");
       //$$ = newnode(Call, NULL);
       $$ = newnode(Dummy1, NULL);
       addchild($$, newnode(IDENTIFIER, $1));
       //yyerrok; //yyclearin;

   }
   ;
ExprList: COMMA Expr                   {
                                       $$ = $2;
                                       //printf("ExprList->0\n");
                                       //LOCATE($$, @2.first_line, @2.first_column);
                                       }
   | ExprList COMMA Expr{               
                                       $$ = $1;
                                       if ($$ != NULL){ //------------------------>PODE DAR ERRO FOI ACRESCENTADO UM DUMMY 6/11 22:23 O QUE ESTA COMENTADO ERA O QUE ESATVA ANTES
                                           struct node* dummy = newnode(Dummy,NULL);
                                           addchild(dummy,$3);
                                           addchild($$,dummy);
                                           //addchild($$, $3);
                                           //LOCATE(getchild($$, 1), @1.first_line, @1.first_column);
                                           
                                       }

                                       //printf("ExprList->1\n");
                                       }

   ;



Expr: Expr OR Expr {

                                       $$ = newnode(Or, NULL);

                                       addchild($$, $1);
                                       addchild($$, $3);
                                       //LOCATE(getchild($$, 0), @2.first_line, @2.first_column);
                                       LOCATE($$, @2.first_line, @2.first_column);


                                       }        
   | Expr AND Expr {      
                                       $$ = newnode(And, NULL);
                                       addchild($$, $1);
                                       addchild($$, $3);
                                       //printf("entrei aqui no AND!\n");
                                       //LOCATE(getchild($$, 0), @2.first_line, @2.first_column);
                                       LOCATE($$, @2.first_line, @2.first_column);

                                       }
   | Expr LT Expr {
                                       $$ = newnode(Lt, NULL);
                                       addchild($$, $1);
                                       addchild($$, $3);
                                       //LOCATE(getchild($$, 0), @2.first_line, @2.first_column);
                                       LOCATE($$, @2.first_line, @2.first_column);
                                       }
   | Expr GT Expr {
                                       $$ = newnode(Gt, NULL);
                                       addchild($$, $1);
                                       addchild($$, $3);
                                       //LOCATE(getchild($$, 0), @2.first_line, @2.first_column);
                                       LOCATE($$, @2.first_line, @2.first_column);

                                       }
   | Expr EQ Expr {
                                       $$ = newnode(Eq, NULL);
                                       addchild($$, $1);
                                       addchild($$, $3);
                                       //printf("12\n");
                                       //LOCATE(getchild($$, 0), @2.first_line, @2.first_column);
                                       LOCATE($$, @2.first_line, @2.first_column);

                                       }
   | Expr NE Expr {
                                       $$ = newnode(Ne, NULL);
                                       addchild($$, $1);
                                       addchild($$, $3);
                                       //LOCATE(getchild($$, 0), @2.first_line, @2.first_column);
                                       LOCATE($$, @2.first_line, @2.first_column);
                                       }
   | Expr LE Expr {
                                       $$ = newnode(Le, NULL);
                                       addchild($$, $1);
                                       addchild($$, $3);
                                       //LOCATE(getchild($$, 0), @2.first_line, @2.first_column);
                                        LOCATE($$, @2.first_line, @2.first_column);
                   }
   | Expr GE Expr {
                                       $$ = newnode(Ge, NULL);
                                       addchild($$, $1);
                                       addchild($$, $3);
                                       //LOCATE(getchild($$, 0), @2.first_line, @2.first_column);
                                       LOCATE($$, @2.first_line, @2.first_column);

                   }
   | Expr PLUS Expr {
                                       $$ = newnode(Add, NULL);
                                       addchild($$, $1);
                                       addchild($$, $3);
                                       //LOCATE(getchild($$, 0), @2.first_line, @2.first_column);
                                        LOCATE($$, @2.first_line, @2.first_column);
                                       }
   | Expr MINUS Expr {
                                       $$ = newnode(Sub, NULL);
                                       addchild($$, $1);
                                       addchild($$, $3);
                                       //LOCATE(getchild($$, 0), @2.first_line, @2.first_column);
                                       LOCATE($$, @2.first_line, @2.first_column);
                                       }
   | Expr STAR Expr {
                                       $$ = newnode(Mul, NULL);
                                       addchild($$, $1);
                                       addchild($$, $3);
                                       //LOCATE(getchild($$, 0), @2.first_line, @2.first_column);
                                       LOCATE($$, @2.first_line, @2.first_column);
                                       }
   | Expr DIV Expr {
                                       $$ = newnode(Div, NULL);
                                       addchild($$, $1);
                                       addchild($$, $3);
                                       //LOCATE(getchild($$, 0), @2.first_line, @2.first_column);
                                       LOCATE($$, @2.first_line, @2.first_column);
                                       }
   | Expr MOD Expr {
                                       $$ = newnode(Mod, NULL);
                                       addchild($$, $1);
                                       addchild($$, $3);
                                       LOCATE($$, @2.first_line, @2.first_column);

                                       }
   | NOT Expr %prec NOT{                       
                                       $$ = newnode(Not, NULL);
                                       addchild($$, $2);
                                       //LOCATE(getchild($$, 0), @1.first_line, @1.first_column);
                                       LOCATE($$, @1.first_line, @1.first_column);

                                       }
   | MINUS Expr %prec NOT {                      
                                       $$ = newnode(Minus, NULL);
                                       addchild($$, $2);
                                       LOCATE($$, @1.first_line, @1.first_column);

                                       }
   | PLUS Expr %prec NOT {                       
                                       $$ = newnode(Plus, NULL);
                                       addchild($$, $2);
                                        LOCATE($$, @1.first_line, @1.first_column);
                                       }
   | NATURAL {//printf("CHEGOU\n");
                                       $$ = newnode(Natural, $1);
                                       //printf("CHEGOU2\n");
                                       //printf("Natural\n");
                                       LOCATE($$, @1.first_line, @1.first_column);
                                       }
   | DECIMAL {
                                       $$ = newnode(Decimal, $1);
                                       //printf("Decimal\n");
                                       LOCATE($$, @1.first_line, @1.first_column);
                                       }
   | IDENTIFIER {
                                       $$ = newnode(Identifier, $1);
                                       //printf("Identifier\n");
                                       LOCATE($$, @1.first_line, @1.first_column);
                                       //printf("Expr IDEN: %d  %d\n", @1.first_line, @1.first_column);

                                       }
   | FuncInvocation {

                                       $$ = $1;

                                       }
   | LPAR Expr RPAR {   
                                       $$ = $2;
                                       $$->parentesis = 1;
                                       //LOCATE($$, @2.first_line, @2.first_column);
                                       
                                       }
   | LPAR error RPAR {
       //yyerror("syntax error");
       $$ = newnode(Dummy1, NULL);
       //yyerrok; //yyclearin;
   }
   ;

%%

/* START subroutines section */

// all needed functions are collected in the .l and ast.* files