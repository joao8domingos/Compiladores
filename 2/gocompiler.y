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

struct node {
    enum category category;
    char *token;
    struct node_list *children;
};

struct node_list {
    struct node *node;
    struct node_list *next;
};

extern int yylineno;

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
void add_node_to_list(struct node_list **list, struct node *new_node);
void add_node_to_all(struct node_list *list, struct node *new_node);
void addchild_begin(struct node *parent, struct node *child);
int has_child_of_type(struct node *parent);

char *list_cat[] = {
    "Program", "Function", "Parameters", "Parameter", "Arguments", "Identifier", "Natural", "Decimal", "Call", "If",
    "Declarations", "VarDeclaration", "VarSpec", "Type", "FuncDeclaration", "FuncBody", "VarsAndStatements", "Statement", "For", "Return", "Print", "ParseArgs", "FuncInvocation", "Expr", "FuncDecl", "FuncHeader", "FuncParams", "VarDecl", "Block", "Assign", "StrLit","ParamDecl",
    "Or", "And", "Lt", "Gt", "Eq", "Ne", "Le", "Ge", "Plus", "Sub", "Mul", "Mod", "Not", "Div", "String", 
    "Int", "Float32", "Bool", "Add", "Dummy", "Minus", "Dummy1"


};

struct node *program;
char type_var[100];
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
/*
%left LOW
%left AND OR
%left '=' '<' '>'
%left EQ GE GT LE LT NE
%left '+' '-'
%left PLUS MINUS
%left '*' '/'
%left STAR DIV
%left '(' ')'
%left NOT
%left MOD
%nonassoc IF
%nonassoc ELSE
*/
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
                                                }
        | PACKAGE IDENTIFIER SEMICOLON {
                                        $$ =  program = newnode(Program, NULL);
                                        //printf("2. Program\n");
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
                                       
                                        
                                }
    | IDENTIFIER Type { 
                        $$ = newnode(VarDecl,NULL);
                        addchild($$,$2);
                        addchild($$,newnode(Identifier,$1));

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
                                        
                                        //printf("9. FuncDeclaration\n");
                                        
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
                                        
                                        }
    
    ;

Parameters: Parameters COMMA IDENTIFIER Type   { 
                                        $$ = $1;  // Continuamos a usar o nó FuncParams anterior
                                        struct node *param = newnode(ParamDecl, NULL);
                                        addchild(param, $4);
                                        addchild(param, newnode(Identifier, $3));
                                        addchild($$, param);
                                        
                                        //printf("Adicionado novo parâmetro!\n");
                                        }
        | IDENTIFIER Type { 
                            $$ = newnode(FuncParams, NULL);  // FuncParams será o nó raiz dos parâmetros
                            
                            struct node *param = newnode(ParamDecl, NULL);
                            addchild(param, $2);
                            addchild(param, newnode(Identifier, $1));
                            addchild($$, param);
                            
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

                    }
                |   FOR ExprOpt LBRACE StatementList RBRACE
                    { 
                        $$ = newnode(For, NULL);
                        addchild($$, $2);
                        struct node * afor = newnode(Block, NULL);
                        addchild($$, afor);
                        addchild(afor, $4);

                    }
                |   RETURN ExprOpt
                    { 
                        $$ = newnode(Return, NULL);
                        addchild($$, $2); 
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
                                    }
    |IDENTIFIER LPAR Expr ExprList RPAR {
                                                $$ = newnode(Call, NULL);
                                                addchild($$,newnode(Identifier, $1));
                                                addchild($$, $3);
                                                addchild($$, $4);
                                                //printf("FuncInovation->2\n");
                                                
    }

    | IDENTIFIER LPAR Expr RPAR {  $$ = newnode(Call, NULL);
                                    addchild($$,newnode(Identifier, $1));
                                    addchild($$, $3);

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
                                        }
    | ExprList COMMA Expr{               
                                        $$ = $1;
                                        if ($$ != NULL){ //------------------------>PODE DAR ERRO FOI ACRESCENTADO UM DUMMY 6/11 22:23 O QUE ESTA COMENTADO ERA O QUE ESATVA ANTES
                                            struct node* dummy = newnode(Dummy,NULL);
                                            addchild(dummy,$3);
                                            addchild($$,dummy);
                                            //addchild($$, $3);
                                        }
                                            
                                        //printf("ExprList->1\n");
                                        }
                                        
    ;



Expr: Expr OR Expr {
                                       
                                        $$ = newnode(Or, NULL);
                                        
                                        addchild($$, $1);
                                        addchild($$, $3);
                                        
                                        
                                        //printf("entrei aqui no OR!\n");

                                        
                                        }        
    | Expr AND Expr {      
                                        $$ = newnode(And, NULL);
                                        addchild($$, $1);
                                        addchild($$, $3);
                                        //printf("entrei aqui no AND!\n");
                                        }
    | Expr LT Expr {
                                        $$ = newnode(Lt, NULL);
                                        addchild($$, $1);
                                        addchild($$, $3);
                                        }
    | Expr GT Expr {
                                        $$ = newnode(Gt, NULL);
                                        addchild($$, $1);
                                        addchild($$, $3);
                                        }
    | Expr EQ Expr {
                                        $$ = newnode(Eq, NULL);
                                        addchild($$, $1);
                                        addchild($$, $3);
                                        //printf("12\n");
                                        }
    | Expr NE Expr {
                                        $$ = newnode(Ne, NULL);
                                        addchild($$, $1);
                                        addchild($$, $3);
                                        }
    | Expr LE Expr {
                                        $$ = newnode(Le, NULL);
                                        addchild($$, $1);
                                        addchild($$, $3);
                    }
    | Expr GE Expr {
                                        $$ = newnode(Ge, NULL);
                                        addchild($$, $1);
                                        addchild($$, $3);
                    }
    | Expr PLUS Expr {
                                        $$ = newnode(Add, NULL);
                                        addchild($$, $1);
                                        addchild($$, $3);
                                        }
    | Expr MINUS Expr {
                                        $$ = newnode(Sub, NULL);
                                        addchild($$, $1);
                                        addchild($$, $3);
                                        }
    | Expr STAR Expr {
                                        $$ = newnode(Mul, NULL);
                                        addchild($$, $1);
                                        addchild($$, $3);
                                        }
    | Expr DIV Expr {
                                        $$ = newnode(Div, NULL);
                                        addchild($$, $1);
                                        addchild($$, $3);
                                        }
    | Expr MOD Expr {
                                        $$ = newnode(Mod, NULL);
                                        addchild($$, $1);
                                        addchild($$, $3);
                                        }
    | NOT Expr %prec NOT{                       
                                        $$ = newnode(Not, NULL);
                                        addchild($$, $2);
                                        }
    | MINUS Expr %prec NOT {                      
                                        $$ = newnode(Minus, NULL);
                                        addchild($$, $2);
                                        }
    | PLUS Expr %prec NOT {                       
                                        $$ = newnode(Plus, NULL);
                                        addchild($$, $2);
                                        }
    | NATURAL {//printf("CHEGOU\n");
                                        $$ = newnode(Natural, $1);
                                        //printf("CHEGOU2\n");
                                        //printf("Natural\n");
                                        }
    | DECIMAL {
                                        $$ = newnode(Decimal, $1);
                                        //printf("Decimal\n");
                                        }
    | IDENTIFIER {
                                        $$ = newnode(Identifier, $1);
                                        //printf("Identifier\n");
                                        }
    | FuncInvocation {

                                        $$ = $1;
                                        }
    | LPAR Expr RPAR {
                                        $$ = $2;
                                        //printf("11\n");
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