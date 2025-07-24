#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantics.h"
#include <ctype.h>

int semantic_errors = 0;

struct symbol_list *symbol_table;

extern char *category_name[];
struct scope_list *lista_de_scopes;


const char *type_name(enum type type) {
   switch (type) {
       case integer_type:
           return "int";
       case float32_type:
           return "float32";
       case bool_type:
           return "bool";
       case string_type:
           return "string";
       case undef:
           return "undef";
       case no_type:
       default:
           return "none";
   }
}

enum type category_type(enum category category) {
   switch (category) {
       case Int:
           return integer_type;
       case Float32:
           return float32_type;
       case Bool:
           return bool_type;
       case String:
           return string_type;
       default:
           return no_type;
   }
}


void add_scope_to_list(struct symbol_list *scope,char *token) {
   // Criar um novo nó para a lista
   struct scope_list *new_node = (struct scope_list *)malloc(sizeof(struct scope_list));
   if (new_node == NULL) {
       perror("Erro ao alocar memória para scope_list");
       return;
   }

   // Inicializar o novo nó
   new_node->scope = scope;
   new_node->next = NULL;
   new_node->token = token;

   // Se a lista está vazia, o novo nó se torna a cabeça
   if (lista_de_scopes == NULL) {
       lista_de_scopes = new_node;
   } else {
       // Caso contrário, percorra até o final e adicione o nó
       struct scope_list *current = lista_de_scopes;
       while (current->next != NULL) {
           current = current->next;
       }
       current->next = new_node;
   }
}

// insert a new symbol in the list, unless it is already there
struct symbol_list *insert_symbol(struct symbol_list *table, char *identifier, enum type type, struct node *node) {
   struct symbol_list *new = (struct symbol_list *) malloc(sizeof(struct symbol_list));
   new->identifier = strdup(identifier);
   new->type = type;
   new->node = node;
   new->next = NULL;
   new->used = 0;

   for (int i = 0; i < 50; i++) {
       new->node->type_call[i] = no_type;
   }
   new->node->tam = 0;

   struct symbol_list *symbol = table;
   while(symbol != NULL) {
       if(symbol->next == NULL) {
           symbol->next = new;    /* insert new symbol at the tail of the list */
           break;
       } else if(strcmp(symbol->next->identifier, identifier) == 0) {
           free(new);
           return NULL;           /* return NULL if symbol is already inserted */
       }
       symbol = symbol->next;
   }
   return new;
}


struct symbol_list *insert_symbol_varDecl(struct symbol_list *table, char *identifier, enum type type, struct node *node, int line, int column) {
   struct symbol_list *new = (struct symbol_list *) malloc(sizeof(struct symbol_list));
   new->identifier = strdup(identifier);
   new->type = type;
   new->node = node;
   new->next = NULL;
   new->used = 0;
   new->line = line;
   new->column = column;
   new->node->is_var = 1;

   for (int i = 0; i < 50; i++) {
       new->node->type_call[i] = no_type;
   }
   new->node->tam = 0;

   struct symbol_list *symbol = table;
   while(symbol != NULL) {
       if(symbol->next == NULL) {
           symbol->next = new;    /* insert new symbol at the tail of the list */
           break;
       } else if(strcmp(symbol->next->identifier, identifier) == 0) {
           free(new);
           return NULL;           /* return NULL if symbol is already inserted */
       }
       symbol = symbol->next;
   }
   return new;
}

// look up a symbol by its identifier
struct symbol_list *search_symbol(struct symbol_list *table, char *identifier) {
   struct symbol_list *symbol;
   for(symbol = table->next; symbol != NULL; symbol = symbol->next) {
       if(strcmp(symbol->identifier, identifier) == 0) {
           return symbol;
       }
   }
   return NULL;
}

void show_symbol_table() {
   struct symbol_list *symbol;

    printf("===== Global Symbol Table =====\n");

   for(symbol = symbol_table->next; symbol != NULL; symbol = symbol->next) {
       //printf("Symbol %s : %s\n", symbol->identifier, type_name(symbol->type));
       printf("%s\t", symbol->identifier);

       if (symbol->node != NULL && symbol->node->category == FuncHeader) { //symbol->node é o FuncHeader
           //printf("  Parameters:\n");

           //printf("%s\n",getchild(symbol->node,1)->category);

           //se o funcHeader tiver Type ou não como filho
           if (getchild(symbol->node, 1)->category == Int || getchild(symbol->node, 1)->category == Float32 || getchild(symbol->node, 1)->category == Bool || getchild(symbol->node, 1)->category == String ) { //O 2º filh do funcHeader pode ou nao ser um Type, entao vamos verificar
               //struct node *parameters = getchild(symbol->node, 1);
               struct node *parameters = getchild(symbol->node, 2);
               struct node_list *parameter = parameters->children;
               //printf("%s\n",parameters->category);

               printf("(");
               while((parameter = parameter->next) != NULL) {

                   if(getchild(parameter->node, 0) != NULL && parameter->next!=NULL){


                           printf("%s,",type_name(category_type(getchild(parameter->node, 0)->category))); // getchild(parameter->node, 0) é o ParamDecl e o filho 0 do paramDecl é o TYPE


                   }
                   else if(getchild(parameter->node, 0) != NULL){



                           printf("%s", type_name(category_type(getchild(parameter->node, 0)->category)));

                   }

               }
               printf(")\t");
           }
           else{
               struct node *parameters = getchild(symbol->node, 1);
               struct node_list *parameter = parameters->children;
               printf("(");
               while((parameter = parameter->next) != NULL) {
                   if(getchild(parameter->node, 0) != NULL && parameter->next!=NULL){


                           printf("%s,",type_name(category_type(getchild(parameter->node, 0)->category))); // getchild(parameter->node, 0) é o ParamDecl e o filho 0 do paramDecl é o TYPE

                   }
                   else if(getchild(parameter->node, 0) != NULL){

                           printf("%s",type_name(category_type(getchild(parameter->node, 0)->category)));

                   }

               }
               printf(")\t");
               //printf("%s\n",parameters->category);
           }

       }
       else{
           printf("\t");
       }

           printf("%s\n",type_name(symbol->type));


   }
   printf("\n");
}

void show_scope(struct symbol_list *scope, char *function) {
   struct symbol_list *symbol;
   struct symbol_list *symbol_global;

   printf("===== Function %s", function);

   //mostrar parametros de entrada da função
   for(symbol_global = symbol_table->next; symbol_global != NULL; symbol_global = symbol_global->next) {
       if(strcmp(symbol_global->identifier,function) == 0){

             if (getchild(symbol_global->node, 1)->category == Int || getchild(symbol_global->node, 1)->category == Float32 || getchild(symbol_global->node, 1)->category == Bool || getchild(symbol_global->node, 1)->category == String ) { //O 2º filh do funcHeader pode ou nao ser um Type, entao vamos verificar
                   //struct node *parameters = getchild(symbol->node, 1);
                   struct node *parameters = getchild(symbol_global->node, 2);
                   struct node_list *parameter = parameters->children;
                   //printf("%s\n",parameters->category);

                   printf("(");
                   while((parameter = parameter->next) != NULL) {

                       if(getchild(parameter->node, 0) != NULL && parameter->next!=NULL){

                           printf("%s,",type_name(category_type(getchild(parameter->node, 0)->category))); // getchild(parameter->node, 0) é o ParamDecl e o filho 0 do paramDecl é o TYPE


                       }
                       else if(getchild(parameter->node, 0) != NULL){

                           printf("%s", type_name(category_type(getchild(parameter->node, 0)->category)));
                       }

                   }
                   printf(")");
               }
               else{
                   struct node *parameters = getchild(symbol_global->node, 1);
                   struct node_list *parameter = parameters->children;                     
                   printf("(");
                   while((parameter = parameter->next) != NULL) {
                       if(getchild(parameter->node, 0) != NULL && parameter->next!=NULL){


                           printf("%s,",type_name(category_type(getchild(parameter->node, 0)->category))); // getchild(parameter->node, 0) é o ParamDecl e o filho 0 do paramDecl é o TYPE

                       }
                       else if(getchild(parameter->node, 0) != NULL){

                           printf("%s",type_name(category_type(getchild(parameter->node, 0)->category)));

                       }

                   }
                   printf(")");
                   //printf("%s\n",parameters->category);
               }

       }

   }
   printf(" Symbol Table =====\n");

   for(symbol = scope->next; symbol != NULL; symbol = symbol->next) {
       if(symbol->node->aux_param == 1){ //------------------------------------------------------------------------------>

           if(strcmp(type_name(symbol->type),"integer") == 0){
               printf("%s\t\tint\tparam\n",symbol->identifier);
           }
           else{
               printf("%s\t\t%s\tparam\n", symbol->identifier, type_name(symbol->type));
           }
       }
       else{
           if(strcmp(type_name(symbol->type),"integer") == 0){
               printf("%s\t\tint\n",symbol->identifier);
           }
           else{
               printf("%s\t\t%s\n", symbol->identifier, type_name(symbol->type));
           }

       }

   }

   printf("\n");
}


void check_expression(struct node *expression, struct symbol_list *scope) {
   int erro_chamada_var_nexiste = 0;
   int problem = 0;
   int passed = 0;

   switch(expression->category) {
       case Identifier:
            problem = 0;
            passed = 0;
           //printf("IDENTIFIER 1111111: %s\n", expression->token);
           //printf("TOKEN %s is_var %d\n", expression->token, expression->is_var);
           if(search_symbol(scope, expression->token) == NULL && search_symbol(symbol_table, expression->token) == NULL) {
               printf("Line %d, column %d: Cannot find symbol %s\n", expression->token_line, expression->token_column, expression->token);
               semantic_errors++;
               //expression->type = no_type;
               expression->type = undef; // Define um tipo inválido para evitar erros em cascata
           } else {
               //printf("SEGMENTATION FAULT VAI SER AQUI!\n");
               if (search_symbol(scope, expression->token) != NULL) {
                    //printf("TOKEN Search_symbol %s is_var %d\n", search_symbol(scope, expression->token)->identifier, search_symbol(scope, expression->token)->node->is_var);
                    if (expression->is_var == search_symbol(scope, expression->token)->node->is_var) {
                        struct symbol_list *aux_list = search_symbol(scope, expression->token);
                        expression->type = search_symbol(scope, expression->token)->type;
                        //if (expression->assign_left != 1)
                            aux_list->used = 1;
                        passed = 1;
                    }
                    else if (passed == 0) {
                        //printf("Line %d, column %d: Symbol %s is not a variable\n", expression->token_line, expression->token_column, expression->token);
                        //semantic_errors++;
                        //expression->type = undef;
                        problem += 1;
                    }

               }
               if (passed == 0) {
                if (search_symbol(symbol_table, expression->token) != NULL) {
                        //printf("TOKEN Search_symbol %s is_var %d\n", search_symbol(symbol_table, expression->token)->identifier, search_symbol(symbol_table, expression->token)->node->is_var);
                        if (expression->is_var == search_symbol(symbol_table, expression->token)->node->is_var) {
                            struct symbol_list *aux_list = search_symbol(symbol_table, expression->token);
                            expression->type = search_symbol(symbol_table, expression->token)->type;
                            //if (expression->assign_left != 1)
                            aux_list->used = 1;
                            passed = 1;
                        }
                        else {
                            //printf("Line %d, column %d: Symbol %s is not a variable\n", expression->token_line, expression->token_column, expression->token);
                            //semantic_errors++;
                            //expression->type = undef;
                            problem += 1;
                        }
                }
               }

               if (problem > 0 && passed == 0) {
                   printf("Line %d, column %d: Cannot find symbol %s\n", expression->token_line, expression->token_column, expression->token);
                   semantic_errors++;
                   expression->type = undef;
               }
           }
           //printf("TYPE IDENTIFIER: %s\n", type_name(expression->type));
           
           break;

       case Natural:
           expression->type = integer_type; // Define tipo "integer" para literais naturais
           break;

       case Decimal:
           expression->type = float32_type; // Define tipo "float32" para literais decimais
           break;

       case Assign: {

           if (search_symbol(symbol_table, getchild(expression, 0)->token) == NULL && search_symbol(scope, getchild(expression, 0)->token) == NULL) {
                //getchild(expression, 0)->assign_left = 1;
               check_expression(getchild(expression, 0), scope);
               check_expression(getchild(expression, 1), scope);
               //getchild(expression, 0)->type = undef;
               
               if (strcmp(type_name(getchild(expression, 0)->type), "none") == 0 ) {
                    //printf("ola4\n");
                    getchild(expression, 0)->type = undef;
               }
               
               printf("Line %d, column %d: Operator = cannot be applied to types %s, %s\n", expression->token_line,
                              expression->token_column,
                              type_name(getchild(expression, 0)->type),
                              type_name(getchild(expression, 1)->type));
                
                if (getchild(expression, 0)->category == FuncHeader) {
                            getchild(expression, 0)->type = no_type;
                        }
               //expression->type = undef;
               //printf("OLA 3\n");
            
           } else {
               struct node *filho1 = NULL;
               //getchild(expression, 0)->assign_left = 1;
               if (search_symbol(scope, getchild(expression, 0)->token) != NULL) {
                   //struct symbol_list *aux_list = search_symbol(scope, getchild(expression, 0)->token);
                   //aux_list->used = 1;
                   filho1 = search_symbol(scope, getchild(expression, 0)->token)->node;
                   filho1->type = search_symbol(scope, getchild(expression, 0)->token)->type;
                   //printf("TYPE IDENTIFIER: %s\n", type_name(filho1->type));
               }
               else {
                   //struct symbol_list *aux_list = search_symbol(symbol_table, getchild(expression, 0)->token);
                   //aux_list->used = 1;
                   filho1 = search_symbol(symbol_table, getchild(expression, 0)->token)->node;
                   filho1->type = search_symbol(symbol_table, getchild(expression, 0)->token)->type;
                   //printf("TYPE IDENTIFIER: %s\n", type_name(filho1->type));
               }
               check_expression(getchild(expression, 0), scope);
               check_expression(getchild(expression, 1), scope);

               if (getchild(expression, 1)->type != no_type) {
                   //printf("OLA 6\n");
                   //if (search_symbol(scope, getchild(expression, 0)->token)->type != getchild(expression, 1)->type) {
                   if (filho1->type != getchild(expression, 1)->type) {
                       //printf("OLA 7\n");
                       
                       if (strcmp(type_name(filho1->type), "none") == 0 ) {
                            //printf("ola3\n");
                            filho1->type = undef;
                        }
                        
                       printf("Line %d, column %d: Operator = cannot be applied to types %s, %s\n",
                              expression->token_line,
                              expression->token_column,
                              type_name(filho1->type),
                              type_name(getchild(expression, 1)->type));

                        if (filho1->category == FuncHeader) {
                            filho1->type = no_type;
                        }
                       semantic_errors++;
                       expression->type = filho1->type;
                   }
                   else {
                       if (getchild(expression, 1)->type == string_type || getchild(expression, 1)->type == undef) {
                            
                            if (strcmp(type_name(filho1->type), "none") == 0) {
                                //printf("ola2\n");
                                filho1->type = undef;
                            }
                            
                           printf("Line %d, column %d: Operator = cannot be applied to types %s, %s\n",
                              expression->token_line,
                              expression->token_column,
                              type_name(filho1->type),
                              type_name(getchild(expression, 1)->type));
                            
                            if (filho1->category == FuncHeader) {
                                filho1->type = no_type;
                            }

                           semantic_errors++;
                           expression->type = filho1->type;
                       }
                       else {
                           expression->type = getchild(expression, 1)->type;
                           getchild(expression, 0)->type = getchild(expression, 1)->type;
                       }
                   }
               }
               else {
                   //printf("OLA 9\n");
                   if (getchild(expression, 1)->category == Call) {
                       //printf("OLA 10\n");
                       /*
                       if (strcmp(type_name(filho1->type), "none") == 0) {
                            //printf("ola1\n");
                            filho1->type = undef;
                        }
                        */
                       printf("Line %d, column %d: Operator = cannot be applied to types %s, %s\n",
                              expression->token_line,
                              expression->token_column,
                              type_name(filho1->type),
                              type_name(getchild(expression, 1)->type));
                       semantic_errors++;
                       expression->type = undef;
                   }
               }
           }
           expression->type = getchild(expression, 0)->type;
           break; }
       case Eq:
       case Ne:
           expression->type = bool_type;

           check_expression(getchild(expression, 0), scope);
           check_expression(getchild(expression, 1), scope);

           // Verifica tipos de ambos os lados
           if (getchild(expression, 0)->type != no_type && getchild(expression, 1)->type != no_type) {
               if (getchild(expression, 0)->type == getchild(expression, 1)->type) {
                   if (getchild(expression, 0)->type == undef) {
                       // Operadores de comparação não são permitidos para strings
                       if (expression->category == Eq) {
                           printf("Line %d, column %d: Operator == cannot be applied to types %s, %s\n",
                              expression->token_line,
                              expression->token_column,
                              type_name(getchild(expression, 0)->type),
                              type_name(getchild(expression, 1)->type));
                       }
                       else if (expression->category == Ne) {
                           printf("Line %d, column %d: Operator != cannot be applied to types %s, %s\n",
                               expression->token_line,
                               expression->token_column,
                               type_name(getchild(expression, 0)->type),
                               type_name(getchild(expression, 1)->type));
                       }
                       semantic_errors++;
                       //expression->type = undef;
                   }
               } else {
                   // Incompatibilidade de tipos
                   //expression->type = no_type;
                   if (expression->category == Eq) {
                       printf("Line %d, column %d: Operator == cannot be applied to types %s, %s\n",
                          expression->token_line,
                          expression->token_column,
                          type_name(getchild(expression, 0)->type),
                          type_name(getchild(expression, 1)->type));
                   }
                   else if (expression->category == Ne) {
                       printf("Line %d, column %d: Operator != cannot be applied to types %s, %s\n",
                          expression->token_line,
                          expression->token_column,
                          type_name(getchild(expression, 0)->type),
                          type_name(getchild(expression, 1)->type));
                   }
                   semantic_errors++;
                   //expression->type = undef;
               }
           } else {
               //expression->type = undef; // Caso um dos lados não tenha tipo definido
           }

           break;
       case Or:
       case And:
           expression->type = bool_type;

           check_expression(getchild(expression, 0), scope);
           check_expression(getchild(expression, 1), scope);

           // Verifica tipos de ambos os lados
           if (getchild(expression, 0)->type != no_type && getchild(expression, 1)->type != no_type) {
               if (getchild(expression, 0)->type == getchild(expression, 1)->type) {
                   if (getchild(expression, 0)->type == string_type || getchild(expression, 0)->type == integer_type || getchild(expression, 0)->type == float32_type || getchild(expression, 0)->type == undef) {
                       // Operadores de comparação não são permitidos para strings
                       if (expression->category == And) {
                           printf("Line %d, column %d: Operator && cannot be applied to types %s, %s\n",
                               expression->token_line,
                                 expression->token_column,
                               type_name(getchild(expression, 0)->type),
                               type_name(getchild(expression, 1)->type));
                       }
                       else if (expression->category == Or) {
                           printf("Line %d, column %d: Operator || cannot be applied to types %s, %s\n",
                               expression->token_line,
                                 expression->token_column,
                               type_name(getchild(expression, 0)->type),
                               type_name(getchild(expression, 1)->type));
                       }
                       semantic_errors++;
                       //expression->type = undef;
                   }
                   // Propaga o tipo compatível
                   //expression->type = getchild(expression, 0)->type;
               } else {
                   if (expression->category == And) {
                           printf("Line %d, column %d: Operator && cannot be applied to types %s, %s\n",
                               expression->token_line,
                                 expression->token_column,
                               type_name(getchild(expression, 0)->type),
                               type_name(getchild(expression, 1)->type));
                       }
                       else if (expression->category == Or) {
                           printf("Line %d, column %d: Operator || cannot be applied to types %s, %s\n",
                               expression->token_line,
                                 expression->token_column,
                               type_name(getchild(expression, 0)->type),
                               type_name(getchild(expression, 1)->type));
                       }
                       semantic_errors++;
                   //expression->type = undef;
               }
           } else {
               //expression->type = undef; // Caso um dos lados não tenha tipo definido
           }
           break;
       case Lt:
       case Gt:
       case Le:
       case Ge:
           expression->type = bool_type;

           check_expression(getchild(expression, 0), scope);
           check_expression(getchild(expression, 1), scope);

           // Verifica tipos de ambos os lados
           if (getchild(expression, 0)->type != no_type && getchild(expression, 1)->type != no_type) {
               if (getchild(expression, 0)->type == getchild(expression, 1)->type) {
                   if (getchild(expression, 0)->type == string_type || getchild(expression, 0)->type == bool_type || getchild(expression, 0)->type == undef) {
                       // Operadores de comparação não são permitidos para strings
                       if (expression->category == Lt) {
                           printf("Line %d, column %d: Operator < cannot be applied to types %s, %s\n",
                              expression->token_line,
                                expression->token_column,
                              type_name(getchild(expression, 0)->type),
                              type_name(getchild(expression, 1)->type));
                       }
                       else if (expression->category == Gt) {
                       printf("Line %d, column %d: Operator > cannot be applied to types %s, %s\n",
                                expression->token_line,
                                    expression->token_column,
                              type_name(getchild(expression, 0)->type),
                              type_name(getchild(expression, 1)->type));
                       semantic_errors++;
                       }
                       else if (expression->category == Le) {
                           printf("Line %d, column %d: Operator <= cannot be applied to types %s, %s\n",
                                 expression->token_line,
                                    expression->token_column,
                               type_name(getchild(expression, 0)->type),
                               type_name(getchild(expression, 1)->type));
                           semantic_errors++;
                       }
                       else if (expression->category == Ge) {
                           printf("Line %d, column %d: Operator >= cannot be applied to types %s, %s\n",
                                    expression->token_line,
                                        expression->token_column,
                               type_name(getchild(expression, 0)->type),
                               type_name(getchild(expression, 1)->type));
                           semantic_errors++;
                       }
                       //expression->type = undef;
                   }
                   // Propaga o tipo compatível
                   //expression->type = getchild(expression, 0)->type;
               } else {
                   if (expression->category == Lt) {
                           printf("Line %d, column %d: Operator < cannot be applied to types %s, %s\n",
                              expression->token_line,
                                expression->token_column,
                              type_name(getchild(expression, 0)->type),
                              type_name(getchild(expression, 1)->type));
                       }
                       else if (expression->category == Gt) {
                       printf("Line %d, column %d: Operator > cannot be applied to types %s, %s\n",
                                expression->token_line,
                                    expression->token_column,
                              type_name(getchild(expression, 0)->type),
                              type_name(getchild(expression, 1)->type));
                       semantic_errors++;
                       }
                       else if (expression->category == Le) {
                           printf("Line %d, column %d: Operator <= cannot be applied to types %s, %s\n",
                                    expression->token_line,
                                        expression->token_column,
                               type_name(getchild(expression, 0)->type),
                               type_name(getchild(expression, 1)->type));
                           semantic_errors++;
                       }
                       else if (expression->category == Ge) {
                           printf("Line %d, column %d: Operator >= cannot be applied to types %s, %s\n",
                                    expression->token_line,
                                        expression->token_column,
                               type_name(getchild(expression, 0)->type),
                               type_name(getchild(expression, 1)->type));
                           semantic_errors++;
                       }
               }
           } else {
               //expression->type = undef; // Caso um dos lados não tenha tipo definido
           }

           break;

       case Minus:
       case Plus:
        check_expression(getchild(expression, 0), scope);
           if (getchild(expression, 0)->type == string_type || getchild(expression, 0)->type == bool_type || getchild(expression, 0)->type == undef) {
               // Operadores de adição e subtração não são permitidos para strings
               if (expression->category == Plus) {
                   printf("Line %d, column %d: Operator + cannot be applied to type %s\n",
                      expression->token_line,
                      expression->token_column,
                      type_name(getchild(expression, 0)->type));
               }
               else if (expression->category == Minus) {
                    printf("Line %d, column %d: Operator - cannot be applied to type %s\n",
                            expression->token_line,
                            expression->token_column,
                            type_name(getchild(expression, 0)->type));
                }
               semantic_errors++;
               expression->type = undef;
           }
           else {
               //check_expression(getchild(expression, 0), scope);
               expression->type = getchild(expression, 0)->type;
           }
           break;
       case Not:
           expression->type = bool_type;
           check_expression(getchild(expression, 0), scope);
           //printf("Type child %s\n", type_name(getchild(expression, 0)->type));
           if (getchild(expression, 0)->type == string_type || getchild(expression, 0)->type == integer_type || getchild(expression, 0)->type == float32_type || getchild(expression, 0)->type == undef) { // || getchild(expression, 0)->type == undef
               // Operadores de adição e subtração não são permitidos para strings
               printf("Line %d, column %d: Operator ! cannot be applied to type %s\n",
                      expression->token_line,
                      expression->token_column,
                      type_name(getchild(expression, 0)->type));
               semantic_errors++;
               //expression->type = undef;
           }
           else {
               //check_expression(getchild(expression, 0), scope);
               //expression->type = getchild(expression, 0)->type;
           }
           break;
       case Add:
       case Sub:
       case Mul:
       case Div:
           // Analisa recursivamente os filhos
           check_expression(getchild(expression, 0), scope);
           check_expression(getchild(expression, 1), scope);
           //expression->type = getchild(expression, 0)->type;

           // Verifica tipos de ambos os lados
           if (getchild(expression, 0)->type != no_type && getchild(expression, 1)->type != no_type) {
               if (getchild(expression, 0)->type == getchild(expression, 1)->type) {
                   if (getchild(expression, 0)->type == integer_type || getchild(expression, 0)->type == float32_type) {
                       expression->type = getchild(expression, 0)->type;
                   } else {
                       // Incompatibilidade de tipos
                       if (expression->category == Add) {
                       printf("Line %d, column %d: Operator + cannot be applied to types %s, %s\n",
                               expression->token_line,
                               expression->token_column,
                               type_name(getchild(expression, 0)->type),
                               type_name(getchild(expression, 1)->type));
                       }
                       else if (expression->category == Sub) {
                           printf("Line %d, column %d: Operator - cannot be applied to types %s, %s\n",
                               expression->token_line,
                                 expression->token_column,
                               type_name(getchild(expression, 0)->type),
                               type_name(getchild(expression, 1)->type));
                       }
                       else if (expression->category == Mul) {
                           printf("Line %d, column %d: Operator * cannot be applied to types %s, %s\n",
                               expression->token_line,
                                 expression->token_column,
                               type_name(getchild(expression, 0)->type),
                               type_name(getchild(expression, 1)->type));
                       }
                       else if (expression->category == Div) {
                           printf("Line %d, column %d: Operator / cannot be applied to types %s, %s\n",
                               expression->token_line,
                                 expression->token_column,
                               type_name(getchild(expression, 0)->type),
                               type_name(getchild(expression, 1)->type));
                       }
                       semantic_errors++;
                       expression->type = undef;
                   }
                   // Propaga o tipo compatível
               } else {
                   // Incompatibilidade de tipos
                   if (expression->category == Add) {
                    printf("Line %d, column %d: Operator + cannot be applied to types %s, %s\n",
                               expression->token_line,
                               expression->token_column,
                               type_name(getchild(expression, 0)->type),
                               type_name(getchild(expression, 1)->type));
                   }
                   else if (expression->category == Sub) {
                       printf("Line %d, column %d: Operator - cannot be applied to types %s, %s\n",
                           expression->token_line,
                            expression->token_column,
                           type_name(getchild(expression, 0)->type),
                           type_name(getchild(expression, 1)->type));
                   }
                   else if (expression->category == Mul) {
                       printf("Line %d, column %d: Operator * cannot be applied to types %s, %s\n",
                           expression->token_line,
                           expression->token_column,
                           type_name(getchild(expression, 0)->type),
                           type_name(getchild(expression, 1)->type));
                   }
                   else if (expression->category == Div) {
                       printf("Line %d, column %d: Operator / cannot be applied to types %s, %s\n",
                           expression->token_line,
                            expression->token_column,
                           type_name(getchild(expression, 0)->type),
                           type_name(getchild(expression, 1)->type));
                   }
                   semantic_errors++;
                   expression->type = undef;
               }
           } else {
               expression->type = no_type; // Caso um dos lados não tenha tipo definido
           }

           break;
       case Mod:
           check_expression(getchild(expression, 0), scope);
           check_expression(getchild(expression, 1), scope);

           // Verifica tipos de ambos os lados
           if (getchild(expression, 0)->type != no_type && getchild(expression, 1)->type != no_type) {
               if ((getchild(expression, 0)->type == integer_type && getchild(expression, 1)->type == integer_type) || (getchild(expression, 0)->type == float32_type && getchild(expression, 1)->type == float32_type)) {

                   expression->type = getchild(expression, 0)->type;

               } else {
                   // Incompatibilidade de tipos
                   expression->type = undef;
                   
                    printf("Line %d, column %d: Operator %% cannot be applied to types %s, %s\n",
                       expression->token_line,
                       expression->token_column,
                       type_name(getchild(expression, 0)->type),
                       type_name(getchild(expression, 1)->type));
                   semantic_errors++;
               }
           } else {
               expression->type = undef; // Caso um dos lados não tenha tipo definido
           }
           break;
       case Block:{
           struct node *dummy1_node = getchild(expression,0);
           struct node_list *filhos_block = dummy1_node->children;

           while((filhos_block = filhos_block->next) != NULL){
               check_expression(filhos_block->node,scope);
           }

           break;}

       case Dummy1:  //Bloco com menos de 2 statements

           if(getchild(expression,0) !=  NULL){
               check_expression(getchild(expression,0),scope);
           }


           break;
       case Return:
           if (getchild(expression, 0)->category == Dummy1) {
               if (search_symbol(scope, "return")->type != no_type) {
                   if (getchild(expression, 0) == NULL){
                       printf("Line %d, column %d: Incompatible type void in return statement\n",
                       expression->token_line, expression->token_column);
                       semantic_errors++;
                   }
                   else {
                        /*
                       printf("Line %d, column %d: Incompatible type %s in return statement\n",
                       getchild(expression, 0)->token_line, getchild(expression, 0)->token_column, type_name(getchild(expression, 0)->type));
                       semantic_errors++;
                       */
                      printf("Line %d, column %d: Incompatible type void in return statement\n",
                       expression->token_line, expression->token_column);
                      semantic_errors++;
                   }
                   expression->type = no_type;
               }
               break;
           }

           check_expression(getchild(expression, 0), scope);

           if (search_symbol(scope, "return")->type != getchild(expression, 0)->type) {
              if (getchild(expression, 0)->token_line != 0 && getchild(expression, 0)->token_column != 0) {
                  printf("Line %d, column %d: Incompatible type %s in return statement\n",
                      getchild(expression, 0)->token_line, getchild(expression, 0)->token_column, type_name(getchild(expression, 0)->type));
              }
              else {  
               
               printf("Line %d, column %d: Incompatible type %s in return statement\n",
                   expression->token_line, expression->token_column, type_name(getchild(expression, 0)->type));
                }
               semantic_errors++;
           }
           break;

       case Call:
           //printf("Passei   %s\n",getchild(expression, 0)->token);
           //printf("ENTROU na função: %s\n", getchild(expression, 0)->token);
           erro_chamada_var_nexiste = 0;

           if (erro_chamada_var_nexiste) {
               // Tomar alguma ação, como talvez retornar ou fazer outro tipo de verificação.
               //printf("A função não foi encontrada!\n");
           }


           if (search_symbol(symbol_table, getchild(expression, 0)->token) == NULL) {
               //printf("Function %s (%d:%d) undeclared\n", getchild(expression, 0)->token, getchild(expression, 0)->token_line, getchild(expression, 0)->token_column);
               semantic_errors++;
               expression->type = undef;
               erro_call_print(expression, scope);

               erro_chamada_var_nexiste = 1;


               getchild(expression, 0)->type = undef;

               //getchild(expression, 0)->type = no_type;
           }
           else{
           getchild(expression, 0)->is_var = 0; 
           getchild(expression, 0)->tam_user = 0;//--------------------------------------------------------->NOVO
           getchild(expression, 0)->is_func = 1;

           expression->type = search_symbol(symbol_table, getchild(expression, 0)->token)->type;

           getchild(expression, 0)->type = search_symbol(symbol_table, getchild(expression, 0)->token)->type;

           getchild(expression, 0)->call_aux = 1;

           struct node* funcHeader = search_symbol(symbol_table, getchild(expression, 0)->token)->node;


           //printf("Function category %s\n", category_name[search_symbol(symbol_table, getchild(expression, 0)->token)->node->category]); //FUNCHEADER

           int param_count = 0;
           if (param_count == 0) {

           }
           struct node_list *parameters = NULL;
           enum type type_list[50];
           int i = 0;
           struct node_list *child_funcHeader = funcHeader->children;


           for (int k = 0; k < 50; k++) {
               type_list[k] = no_type;
           }

           int aux_dummy = 0;
           int count_erros_call = 0;
           enum type type_func_user[50];

           for (int k = 0; k < 50; k++) {
               type_func_user[k] = no_type;
           }

           if (child_funcHeader->next->node != NULL) {


           if (getchild(funcHeader, 1)->category == Int || getchild(funcHeader, 1)->category == Float32 || getchild(funcHeader, 1)->category == Bool || getchild(funcHeader, 1)->category == String ) { //O 2º filh do funcHeader pode ou nao ser um Type, entao vamos verificar

               parameters = getchild(funcHeader, 2)->children; //ParamDecl
               while ((parameters = parameters->next) != NULL) {
                   //parameters = parameters->next;
                   param_count++;
                   //printf("Type asdasd %s\n", type_name(category_type(getchild(parameters->node, 0)->category)));
                   type_list[i] = category_type(getchild(parameters->node, 0)->category);
                   //printf("TYPE LIST 1111: %s\n", type_name(type_list[i]));
                   getchild(expression, 0)->type_call[i] = type_list[i];
                   getchild(expression, 0)->tam++;
                   i++;
                   //printf("Type %s\n", type_name(category_type(getchild(parameters->node, 0)->category)));
               }

           }
           else {
               parameters = getchild(funcHeader, 1)->children; //ParamDecl
               while ((parameters = parameters->next) != NULL) {
                   //parameters = parameters->next;
                   param_count++;
                   //printf("Type asdasd %s\n", type_name(category_type(getchild(parameters->node, 0)->category)));
                   type_list[i] = category_type(getchild(parameters->node, 0)->category);
                   getchild(expression, 0)->type_call[i] = type_list[i];
                   getchild(expression, 0)->tam++;
                   i++;
                   //printf("Type %s\n", type_name(category_type(getchild(parameters->node, 0)->category)));
               }
           }

           }

           if (getchild(expression, 0)->tam == 0){
               getchild(expression, 0)->func_0_param = 1;
           }

           //printf("PASSEI 5\n");
           struct node_list *child_expression = expression->children;
           int count_c = 0;
           int j = 0;
           struct node *first_argument = NULL;
           struct node *second_argument = NULL;
           struct node_list *additional_arguments = NULL;


           //printf("PASSEI 6\n");

           for (int k = 0; k < 50; k++) {
               //printf("Type LIST 1111: %s\n", type_name(type_list[k]));
           }

           while ((child_expression = child_expression->next) != NULL) {
               //printf("Tem argumentos %s\n", category_name[child_expression->node->category]);

               if (count_c == 1) { //2º filho ---> 1º argumento

                   first_argument = child_expression->node;
                   enum type first_type = no_type;

                   if (first_argument->token == NULL || strcmp(category_name[first_argument->category],"Natural") == 0 || strcmp(category_name[first_argument->category],"Decimal")==0){
                       //check_expression(first_argument, scope);
                       //printf("--> %s\n",category_name[first_argument->category]);

                       if(strcmp(category_name[first_argument->category],"Call") == 0 ){

                           //printf("CALL\n");
                           //check_expression(first_argument, scope);

                           check_expression(first_argument, scope);

                           if(getchild(first_argument,0)->type!=undef){
                               first_type = search_symbol(symbol_table,getchild(first_argument,0)->token)->type;
                               type_func_user[j-1] = first_type;
                               getchild(expression, 0)->type_call_user[j-1] = first_type;
                               getchild(expression, 0)->tam_user++;
                           }
                           else{

                               first_type = no_type;
                              type_func_user[j-1] = first_type;
                              getchild(expression, 0)->type_call_user[j-1] = first_type;
                              getchild(expression, 0)->tam_user++;
                              //check_expression(getchild(first_argument, 0), scope); //---------------------------------------------------------------->
                               //printf("%s  %s\n",category_name[getchild(first_argument, 0)->category],getchild(first_argument, 0)->token);


                           }



                       }
                       else{

                               if(strcmp(category_name[first_argument->category],"Dummy") == 0 ){

                                   first_type = no_type;
                                   type_list[j-1] =no_type;
                                   aux_dummy = 1;
                                   //printf("AQUI!\n");


                               }
                               else{

                                   //printf("%s  IMPORTANTE!\n", category_name[first_argument->category]);
                                   check_expression(first_argument, scope);

                                   first_type = first_argument->type;
                                   type_func_user[j-1] = first_type;
                                   getchild(expression, 0)->type_call_user[j-1] = first_type;
                                   getchild(expression, 0)->tam_user++;

                               }


                       }


                       //first_type = first_argument->type;
                       //printf("First Argument Category: %s\n", category_name[first_argument->category]);
                       //printf(" %s   :   %s     :    %s\n",type_name(first_type),type_name(type_list[j - 1]),type_name(type_func_user[j-1]));
                       if (first_type != type_list[j - 1]) {
                           count_erros_call = 1;
                           //printf(" %s   :   %s     :    %s\n",type_name(first_type),type_name(type_list[j - 1]),type_name(type_func_user[j-1]));
                           //printf("Argument %s\n", category_name[first_argument->category]);

                           //printf("%s Line %d, Column %d: Invalid type in argument aaa %d\n", category_name[getchild(expression, 0)->category],getchild(expression, 0)->token_line, getchild(expression, 0)->token_column, j);

                           //printf("Type %s\n", type_name(first_type));
                           //printf("Type %s\n", type_name(type_list[j - 1]));
                           semantic_errors++;
                           expression->type = undef;
                           getchild(expression, 0)->type = undef;
                       }

                   }
                   else if (search_symbol(scope, first_argument->token) != NULL || search_symbol(symbol_table, first_argument->token) != NULL){

                       check_expression(first_argument, scope);

                       if (search_symbol(scope, first_argument->token) != NULL) {

                           first_type = search_symbol(scope, first_argument->token)->type;
                           type_func_user[j-1] = first_type;
                           getchild(expression, 0)->type_call_user[j-1] = first_type;
                           getchild(expression, 0)->tam_user++;

                       }
                       else {

                           first_type = search_symbol(symbol_table, first_argument->token)->type;
                           type_func_user[j-1] = first_type;
                           getchild(expression, 0)->type_call_user[j-1] = first_type;
                           getchild(expression, 0)->tam_user++;

                       }
                       //first_type = search_symbol(scope, first_argument->token)->type;
                       //printf(" %s   :   %s     :    %s\n",type_name(first_type),type_name(type_list[j - 1]),type_name(type_func_user[j-1]));
                       if (first_type != type_list[j - 1]) {
                           count_erros_call = 1;

                           //printf(" %s   :   %s     :    %s\n",type_name(first_type),type_name(type_list[j - 1]),type_name(type_func_user[j-1]));
                           //printf("Argument %s\n", category_name[first_argument->category]);

                           //printf("%s Line %d, Column %d: Invalid type in argument aaa %d\n", category_name[getchild(expression, 0)->category],getchild(expression, 0)->token_line, getchild(expression, 0)->token_column, j);

                           //printf("Type %s\n", type_name(first_type));
                           //printf("Type %s\n", type_name(type_list[j - 1]));
                           semantic_errors++;
                           expression->type = undef;
                           getchild(expression, 0)->type = undef;
                       }
                   }
                   else if(search_symbol(scope, first_argument->token) == NULL && search_symbol(symbol_table, first_argument->token) == NULL){ //variavel colocada como argumento nao existe

                       //printf("Variable %s (%d:%d) undeclared\n", first_argument->token, first_argument->token_line, first_argument->token_column);
                       printf("Line %d, column %d: Cannot find symbol %s\n",first_argument->token_line,first_argument->token_column,first_argument->token);
                       type_func_user[j-1] = no_type;
                       getchild(expression, 0)->type_call_user[j-1] = no_type;
                       getchild(expression, 0)->tam_user++;
                       semantic_errors++;
                       expression->type = undef;
                       getchild(expression, 0)->type = undef;
                   }


               }
               if (count_c == 2) { //3º filho ---> 2º argumento
                   second_argument = child_expression->node;
                   enum type second_type = no_type;

                   if (second_argument->token == NULL || strcmp(category_name[second_argument->category],"Natural") == 0 || strcmp(category_name[second_argument->category],"Decimal")==0){
                       //check_expression(second_argument, scope);

                       if(strcmp(category_name[second_argument->category],"Call") == 0 ){

                           //printf("CALL\n");
                           //check_expression(second_argument, scope);
                           check_expression(second_argument, scope);

                           if(getchild(second_argument,0)->type!=undef){
                               second_type = search_symbol(symbol_table,getchild(second_argument,0)->token)->type;
                               type_func_user[j-1] = second_type;
                               getchild(expression, 0)->type_call_user[j-1] = second_type;
                               getchild(expression, 0)->tam_user++;
                           }
                           else{

                                if(strcmp(category_name[second_argument->category],"Dummy") == 0 ){

                                   second_type = no_type;
                                   type_list[j-1] =no_type;
                                   aux_dummy = 1;
                                   //printf("AQUI!\n");


                               }
                                else{
                                second_type = no_type;
                                type_func_user[j-1] = second_type;
                                getchild(expression, 0)->type_call_user[j-1] = second_type;
                                getchild(expression, 0)->tam_user++;
                                }
                               //check_expression(getchild(second_argument, 0), scope); //---------------------------------------------------------->

                           }


                       }
                       else{



                                   check_expression(second_argument, scope);
                                   second_type = second_argument->type;
                                   type_func_user[j-1] = second_type;
                                   getchild(expression, 0)->type_call_user[j-1] = second_type;
                                   getchild(expression, 0)->tam_user++;



                       }


                       //second_type = second_argument->type;
                       //printf("---------------------> %s  %s\n",type_name(second_type),type_name(type_list[j - 1]));
                       if (second_type != type_list[j - 1]) {

                           count_erros_call = 1;
                           //printf("Argument %s\n", category_name[first_argument->category]);

                           //printf("%s Line %d, Column %d: Invalid type in argument aaa %d\n", category_name[getchild(expression, 0)->category],getchild(expression, 0)->token_line, getchild(expression, 0)->token_column, j);

                           //printf("Type %s\n", type_name(second_type));
                           //printf("Type %s\n", type_name(type_list[j - 1]));
                           semantic_errors++;
                           expression->type = undef;
                           getchild(expression, 0)->type = undef;
                       }

                   }
                   else if (search_symbol(scope, second_argument->token) != NULL || search_symbol(symbol_table, second_argument->token) != NULL){
                       //printf("FODASSSEIYCFWIUYFGWIGI\n");

                       check_expression(second_argument, scope);

                       if (search_symbol(scope, second_argument->token) != NULL) {

                           second_type = search_symbol(scope, second_argument->token)->type;
                           type_func_user[j-1] = second_type;
                           getchild(expression, 0)->type_call_user[j-1] = second_type;
                           getchild(expression, 0)->tam_user++;


                       }
                       else {

                           second_type = search_symbol(symbol_table, second_argument->token)->type;
                           type_func_user[j-1] = second_type;
                           getchild(expression, 0)->type_call_user[j-1] = second_type;
                           getchild(expression, 0)->tam_user++;


                       }
                       //second_type = search_symbol(scope, second_argument->token)->type;

                       if (second_type != type_list[j - 1]) {

                           count_erros_call = 1;
                           //printf("Argument %s\n", category_name[second_argument->category]);

                           //printf("Invalid type in argument bbb %d\n", j);

                           //printf("Type %s\n", type_name(second_type));
                           //printf("Type %s\n", type_name(type_list[j - 1]));
                           semantic_errors++;
                           expression->type = undef;
                           getchild(expression, 0)->type = undef;
                       }
                   }
                   else if(search_symbol(scope, second_argument->token) == NULL && search_symbol(symbol_table, second_argument->token) == NULL){

                       //printf("Variable %s (%d:%d) undeclared\n", second_argument->token, second_argument->token_line, second_argument->token_column);
                       printf("Line %d, column %d: Cannot find symbol %s\n",second_argument->token_line,second_argument->token_column,second_argument->token);
                       type_func_user[j-1] = no_type;
                       getchild(expression, 0)->type_call_user[j-1] = no_type;
                       getchild(expression, 0)->tam_user++;
                       semantic_errors++;
                       expression->type = undef;
                       getchild(expression, 0)->type = undef;
                   }


                   additional_arguments = child_expression->node->children; // o additional_arguments é os filhos do segundo paramentro

                   //printf("%s ------>DEVERIA SER ... %s\n",category_name[getchild(second_argument,0)->category], getchild(second_argument,0)->token);

                   while ((additional_arguments = additional_arguments->next) != NULL) {

                       enum type aux_type = no_type;

                       if(strcmp(category_name[additional_arguments->node->category],"Dummy") != 0){

                           //printf("DIFERENTE: %s\n",category_name[additional_arguments->node->category]);
                           //------------------------------------------------------------------------------------------------------------------------>
                       }
                       else{
                           //printf("IGUAL : %s\n",category_name[additional_arguments->node->category]);


                           if (getchild(additional_arguments->node, 0)->token == NULL || strcmp(category_name[getchild(additional_arguments->node, 0)->category],"Natural") == 0 || strcmp(category_name[getchild(additional_arguments->node, 0)->category],"Decimal")==0){

                           //printf("%s ------> \n",category_name[getchild(additional_arguments->node, 0)->category]);
                           //check_expression(getchild(additional_arguments->node, 0), scope);


                               if(strcmp(category_name[getchild(additional_arguments->node,0)->category],"Call") == 0 ){
                                   //printf("CALL\n");


                                   check_expression(getchild(additional_arguments->node, 0), scope);


                                   //printf("%s ------->\n",getchild(getchild(additional_arguments,0),0)->token);

                                   if(getchild(getchild(additional_arguments->node,0),0)->type!=undef){
                                       aux_type = search_symbol(symbol_table,getchild(getchild(additional_arguments->node,0),0)->token)->type;
                                       type_func_user[j] = aux_type;
                                       getchild(expression, 0)->type_call_user[j] = aux_type;
                                       getchild(expression, 0)->tam_user++;

                                   }
                                   else{

                                        if(strcmp(category_name[getchild(additional_arguments->node,0)->category],"Dummy") == 0 ){

                                            aux_type = no_type;
                                            type_list[j-1] =no_type;
                                            aux_dummy = 1;
                                            //printf("AQUI!\n");


                                        }
                                        else{
                                       aux_type = no_type;
                                       type_func_user[j] = aux_type;
                                       getchild(expression, 0)->type_call_user[j] = aux_type;
                                       getchild(expression, 0)->tam_user++;
                                        }
                                       //check_expression(getchild(getchild(additional_arguments->node, 0),0), scope); //---------------------------------------------------------->


                                   }

                               }
                               else{

                                   check_expression(getchild(additional_arguments->node, 0), scope);

                                   aux_type = getchild(additional_arguments->node, 0)->type;
                                   type_func_user[j] = aux_type;
                                   getchild(expression, 0)->type_call_user[j] = aux_type;
                                   getchild(expression, 0)->tam_user++;

                               }


                               //aux_type = getchild(additional_arguments->node, 0)->type;

                               if (aux_type != type_list[j]) {
                                   count_erros_call = 1;
                                   //printf("Argument %s\n", category_name[first_argument->category]);

                                   //printf("%s Line %d, Column %d: Invalid type in argument aaa %d\n", category_name[getchild(expression, 0)->category],getchild(expression, 0)->token_line, getchild(expression, 0)->token_column, j);

                                   //printf("Type %s\n", type_name(getchild(additional_arguments->node, 0)->type));
                                   //printf("Type %s\n", type_name(type_list[j -1 ]));
                                   semantic_errors++;
                                   expression->type = undef;
                                   getchild(expression, 0)->type = undef;
                               }
                           }    
                           else if (search_symbol(scope, getchild(additional_arguments->node, 0)->token) != NULL || search_symbol(symbol_table, getchild(additional_arguments->node, 0)->token) != NULL){

                               check_expression(getchild(additional_arguments->node, 0), scope);

                               if (search_symbol(scope, getchild(additional_arguments->node, 0)->token) != NULL) {
                                   aux_type = search_symbol(scope, getchild(additional_arguments->node, 0)->token)->type;
                                   type_func_user[j] = aux_type;
                                   getchild(expression, 0)->type_call_user[j] = aux_type;
                                   getchild(expression, 0)->tam_user++;


                               }
                               else {

                                   aux_type = search_symbol(symbol_table, getchild(additional_arguments->node, 0)->token)->type;
                                   type_func_user[j] = aux_type;
                                   getchild(expression, 0)->type_call_user[j] = aux_type;
                                   getchild(expression, 0)->tam_user++;

                               }
                               //aux_type = search_symbol(scope, getchild(additional_arguments->node, 0)->token)->type;
                               //printf("Argument %s\n", category_name[additional_arguments->node->category]);

                               if (aux_type != type_list[j]) {
                                   count_erros_call = 1;
                                   //printf("Invalid type in argument cccc %d\n", j);
                                   //printf("Type %s\n", type_name(aux_type));
                                   //printf("Type %s\n", type_name(type_list[j]));
                                   semantic_errors++;
                                   expression->type = undef;
                                   getchild(expression, 0)->type = undef;
                               }

                           }
                           else if(search_symbol(scope, getchild(additional_arguments->node, 0)->token) == NULL){

                               //printf("Variable %s (%d:%d) undeclared\n", getchild(additional_arguments->node, 0)->token, getchild(additional_arguments->node, 0)->token_line, getchild(additional_arguments->node, 0)->token_column);
                               printf("Line %d, column %d: Cannot find symbol %s\n",getchild(additional_arguments->node, 0)->token_line,getchild(additional_arguments->node, 0)->token_column,getchild(additional_arguments->node, 0)->token);
                               type_func_user[j] = no_type;
                               getchild(expression, 0)->type_call_user[j] = no_type;
                               getchild(expression, 0)->tam_user++;
                               semantic_errors++;
                               expression->type = undef;
                               getchild(expression, 0)->type = undef;
                           }
                           j++;

                       }



                   }
               }
               count_c++;
               j++;
           }

           //printf("Param count %d\n", param_count);
           //printf("J %d\n", j);
           //isto esta bem mas tenho de colocar em comentario agora-------------------------------------------------------------------------------------->

           if(aux_dummy == 1){
               aux_dummy = 0;
               j = j-1;
           }

           int contador_argumentos = 0;
           for (int l = 0; l < j-1; l++) {
                if (strcmp(type_name(type_func_user[l]), "none") != 0) {
                    contador_argumentos++;
                }
           }


           if(count_erros_call == 1 || param_count != contador_argumentos){
               expression->type = undef;
               getchild(expression, 0)->type = undef;
               printf("Line %d, column %d: Cannot find symbol %s(",getchild(expression,0)->token_line,getchild(expression,0)->token_column,getchild(expression,0)->token);
               for(int l = 0; l < j-1; l++){
                   if(l!=j-2){
                       printf("%s,",type_name(type_func_user[l]));
                   }
                   else{
                       printf("%s",type_name(type_func_user[l]));
                   }
               }
               printf(")\n");
               
           }




           //printf("VALOR J - 1: %d\n",j-1);
           //printf("%s    -->\n",type_name(type_func_user[0]));
           }


           break;

       case If: // -------------------------------------------------------------------------------------------------------->
           check_expression(getchild(expression, 0), scope); // Condição
           //printf("IF 2\n");
           if (getchild(expression, 0)->type != bool_type) {
                /*
               printf("Line %d, column %d: Incompatible type %s in if statement\n",
                   expression->token_line,
                   expression->token_column,
                   type_name(getchild(expression, 0)->type));
                */
               if (getchild(expression, 0)->parentesis == 0) {
                     printf("Line %d, column %d: Incompatible type %s in if statement\n",
                          getchild(expression, 0)->token_line,
                          getchild(expression, 0)->token_column,
                          type_name(getchild(expression, 0)->type));
               }
               else {
                printf("Line %d, column %d: Incompatible type %s in if statement\n",
                     getchild(expression, 0)->token_line,
                     getchild(expression, 0)->token_column,
                     type_name(getchild(expression, 0)->type));
               }
            
               semantic_errors++;
           }
           //printf("IF 3\n");
           struct node_list *percorre_filhos_if = getchild(getchild(expression,1),0)->children;
           while((percorre_filhos_if = percorre_filhos_if->next) != NULL) { //percorre-se os statements do if
               //printf("%s\n",category_name[percorre_filhos_if->node->category]);
               check_expression(percorre_filhos_if->node, scope);
           }

           struct node_list *percorre_filhos_else = getchild(getchild(expression,2),0)->children;
           while((percorre_filhos_else = percorre_filhos_else->next) != NULL) { //percorre-se os statements do else
               //printf("%s\n",category_name[percorre_filhos_else->node->category]);
               check_expression(percorre_filhos_else->node, scope);
           }

           //check_expression(getchild(expression, 1), scope); // Bloco "then"
           //check_expression(getchild(expression, 2), scope); // Bloco "else"
           break;

       case For: //---------------------------------------------------------------------------------------------------------->
           check_expression(getchild(expression, 0), scope); // Condição
           if (getchild(expression, 0)->category == Dummy1) {
               struct node_list *percorre_statements_for = getchild(getchild(expression,1),0)->children;
               while((percorre_statements_for = percorre_statements_for->next) != NULL) { //percorre-se os statements do if
                   //printf("%s\n",category_name[percorre_statements_for->node->category]);
                   check_expression(percorre_statements_for->node, scope);
               }
           }
           else {
               if (getchild(expression, 0)->type != bool_type) {
                   
                   if (getchild(expression, 0)->parentesis == 0) {
                   printf("Line %d, column %d: Incompatible type %s in for statement\n",
                       getchild(expression, 0)->token_line,
                       getchild(expression, 0)->token_column,
                       type_name(getchild(expression, 0)->type));
                   }
                   else {
                        printf("Line %d, column %d: Incompatible type %s in for statement\n",
                            getchild(expression, 0)->token_line,
                            getchild(expression, 0)->token_column,
                            type_name(getchild(expression, 0)->type));
                   }
                   
                   
                   semantic_errors++;
               }
               //else {
                   struct node_list *percorre_statements_for = getchild(getchild(expression,1),0)->children;
                   while((percorre_statements_for = percorre_statements_for->next) != NULL) { //percorre-se os statements do if
                       //printf("%s\n",category_name[percorre_statements_for->node->category]);
                       check_expression(percorre_statements_for->node, scope);
                   }
               //}
           }
           break;

       case ParseArgs: //------------------------------------------------------------------------------------------------------>
           //printf("TYPE PARSEARGS: %s\n", type_name(expression->type));

           //if (getchild(expression, 0)->type != string_type) {
               //printf("Line %d, column %d: Operator strconv.Atoi cannot be applied to types %s, %s\n",
                      //getchild(expression, 0)->token_line,
                      //getchild(expression, 0)->token_column, type_name(getchild(expression, 0)->type), type_name(getchild(expression, 1)->type));
               //semantic_errors++;
           //}

           //else {
               check_expression(getchild(expression, 0), scope); // Identifier
               check_expression(getchild(expression, 1), scope); //Expr
               expression->type = getchild(expression, 0)->type;

               if (getchild(expression, 0)->type != integer_type || getchild(expression, 1)->type != integer_type) {
                   printf("Line %d, column %d: Operator strconv.Atoi cannot be applied to types %s, %s\n",
                          getchild(expression, 0)->token_line,
                          getchild(expression, 0)->token_column, type_name(getchild(expression, 0)->type), type_name(getchild(expression, 1)->type));
                   semantic_errors++;
                   expression->type = undef;
               }
           //}


           break;

       case Print: //------------------------------------------------------------------------------------------------------------->
            //expression->type = string_type;
           if(strcmp(category_name[getchild(expression, 0)->category], "StrLit")!=0){
               check_expression(getchild(expression, 0), scope);
               //expression->type = getchild(expression, 0)->type;

               if (getchild(expression, 0)->type == undef) {
               printf("Line %d, column %d: Incompatible type %s in fmt.Println statement\n",
                      getchild(expression, 0)->token_line,
                      getchild(expression, 0)->token_column, type_name(getchild(expression, 0)->type));
               semantic_errors++;
               }
           }
           else{
               getchild(expression, 0)->type = string_type;
           }



           break;

       case VarDecl:
           //struct node *type = getchild(expression, 0);
           //struct node *id = getchild(expression, 1); // id é o Identifier
           //struct node *optType = getchild(expression, 2); //

           if(search_symbol(scope, getchild(expression, 1)->token) == NULL) {
               insert_symbol_varDecl(scope, getchild(expression, 1)->token, category_type(getchild(expression, 0)->category), expression, getchild(expression, 1)->token_line, getchild(expression, 1)->token_column);
                //printf("VarDecl: %s\n", getchild(expression, 1)->token);
           } else {
               printf("Line %d, column %d: Symbol %s already defined\n", getchild(expression, 1)->token_line,
               getchild(expression, 1)->token_column, getchild(expression, 1)->token);
               semantic_errors++;
               //expression->type = undef;
           }


           if (getchild(expression, 2) != NULL) {
               struct node_list *vars = getchild(expression, 2)->children; //vars é o varDecl
               //printf("Category 0: %s\n", category_name[vars->node->category]);
               //category_name[getchild(vars->node, 0)->category]

               while((vars = vars->next) != NULL) {
                   struct node* aux = getchild(vars->node,1);

                   if(search_symbol(scope, aux->token) == NULL) {
                       
                       insert_symbol_varDecl(scope, aux->token, category_type(getchild(expression, 0)->category), vars->node, aux->token_line, aux->token_column);
                       
                   }
                   else {
                       printf("Line %d, column %d: Symbol %s already defined\n", aux->token_line, aux->token_column, aux->token);
                       semantic_errors++;
                       //aux->type = undef;
                   }


               }


           }
       break;
       default:
           break;
   }

}



void check_parameters(struct node *parameters, struct symbol_list *scope) { //Parameters é o FuncParams
   struct node_list *parameter = parameters->children;
   struct symbol_list *new;
   while((parameter = parameter->next) != NULL) {
       struct node *id = getchild(parameter->node, 1);
       enum type type = category_type(getchild(parameter->node, 0)->category);
       if(search_symbol(scope, id->token) == NULL) {
           //insert_symbol(symbol_table, id->token, type, parameter->node);
           new = insert_symbol(scope, id->token, type, parameter->node);
           new->node->aux_param = 1; //----------------------------------------------------------------------------------------------->
       } else {
           printf("Line %d, column %d: Symbol %s already defined\n", id->token_line, id->token_column,id->token);
           semantic_errors++;
       }
   }
}

void check_function(struct node *function) { // Function é o funcDecl

   struct node *id = getchild(getchild(function, 0), 0); // O 1º getchild leva nos para funcHeader e o 2º getchild para o identifier (exemplo: main)

   if(search_symbol(symbol_table, id->token) == NULL) {
       if (getchild(getchild(function, 0), 1)->category == Int || getchild(getchild(function, 0), 1)->category == Float32 || getchild(getchild(function, 0), 1)->category == Bool || getchild(getchild(function, 0), 1)->category == String) { //O 2º filh do funcHeader pode ou nao ser um Type, entao vamos verificar
           //insert_symbol(symbol_table, id->token, category_type(getchild(getchild(function, 0), 1)->category), getchild(function, 0));
       } else {
           //insert_symbol(symbol_table, id->token, no_type, getchild(function, 0));
       }
       //insert_symbol(symbol_table, id->token, no_type, function);
   } else {
       //printf("Line %d Column %d: Identifier %s already declared\n", id->token_line, id->token_column, id->token);
       //printf("Line %d, column %d: Symbol %s already defined\n", id->token_line, id->token_column, id->token);
       //semantic_errors++;
   }

   struct symbol_list *scope = (struct symbol_list *) malloc(sizeof(struct symbol_list));
   scope->next = NULL;

   add_scope_to_list(scope, id->token);


   if (getchild(getchild(function, 0), 1)->category == Int || getchild(getchild(function, 0), 1)->category == Float32 || getchild(getchild(function, 0), 1)->category == Bool || getchild(getchild(function, 0), 1)->category == String) { //O 2º filh do funcHeader pode ou nao ser um Type, entao vamos verificar
       insert_symbol(scope, "return", category_type(getchild(getchild(function, 0), 1)->category), function);
       check_parameters(getchild(getchild(function, 0), 2), scope);
           //show_scope(scope, id->token);
   }
   else {
       insert_symbol(scope, "return", category_type(getchild(getchild(function, 0), 1)->category), function);
       check_parameters(getchild(getchild(function, 0), 1), scope);
           //show_scope(scope, id->token);
   }

   struct node *funcBody = getchild(function, 1);

   if (getchild(funcBody, 0) == NULL) {
       return;
   }

   struct node_list *child_funcBody = funcBody->children;

   while ((child_funcBody = child_funcBody->next) != NULL) {
       //printf("Category 0: %s\n", category_name[child_funcBody->node->category]);
       struct node_list *child_expr = child_funcBody->node->children;

       while ((child_expr = child_expr->next) != NULL) {

           check_expression(child_expr->node, scope);
       }
   }

   //show_scope(scope, id->token); //------------------------------------------->coloquei aqui o show_scope no fim de tudo
   //check_expression(getchild(function, 1), scope);
}


void check_function_parse1(struct node *function) { // Function é o funcDecl

   struct node *id = getchild(getchild(function, 0), 0); // O 1º getchild leva nos para funcHeader e o 2º getchild para o identifier (exemplo: main)

   if(search_symbol(symbol_table, id->token) == NULL) {
       if (getchild(getchild(function, 0), 1)->category == Int || getchild(getchild(function, 0), 1)->category == Float32 || getchild(getchild(function, 0), 1)->category == Bool || getchild(getchild(function, 0), 1)->category == String) { //O 2º filh do funcHeader pode ou nao ser um Type, entao vamos verificar
           struct symbol_list *help_func = insert_symbol(symbol_table, id->token, category_type(getchild(getchild(function, 0), 1)->category), getchild(function, 0));
           help_func->node->is_var = 0;
       } else {
           struct symbol_list *help_func = insert_symbol(symbol_table, id->token, no_type, getchild(function, 0));
           help_func->node->is_var = 0;
       }
       //insert_symbol(symbol_table, id->token, no_type, function);
   } else {
       //printf("Line %d Column %d: Identifier %s already declared\n", id->token_line, id->token_column, id->token);
       printf("Line %d, column %d: Symbol %s already defined\n", id->token_line, id->token_column, id->token);
       semantic_errors++;
       function->declared_func = 1;
       
   }
}





void check_var(struct node *var) { // var é o VarDecl
   struct node *type = getchild(var, 0);
   struct node *id = getchild(var, 1); // id é o Identifier
   struct node *optType = getchild(var, 2); //

   if(search_symbol(symbol_table, id->token) == NULL) {
       struct symbol_list *help_is_var = insert_symbol(symbol_table, id->token, category_type(type->category), var);
       //getchild(search_symbol(symbol_table, id->token)->node, 1)->is_var = 1;
       help_is_var->node->is_var = 1;
   } else {
       printf("Line %d, column %d: Symbol %s already defined\n", id->token_line, id->token_column,id->token);
       semantic_errors++;
   }


   if (optType != NULL) {
       struct node_list *vars = optType->children; //vars é o varDecl
       //printf("Category 0: %s\n", category_name[vars->node->category]);
       //category_name[getchild(vars->node, 0)->category]

       while((vars = vars->next) != NULL) {
           struct node* aux = getchild(vars->node,1);

           if(search_symbol(symbol_table, aux->token) == NULL) {
               struct symbol_list *help_is_var1 = insert_symbol(symbol_table, aux->token, category_type(type->category), vars->node);
                help_is_var1->node->is_var = 1;
               //getchild(search_symbol(symbol_table, aux->token)->node, 1)->is_var = 1;
           }
           else {
               //printf("Identifier %s (%d:%d) already declared\n", aux->token, aux->token_line, aux->token_column);
               printf("Line %d, column %d: Symbol %s already defined\n", aux->token_line, aux->token_column,aux->token);
               semantic_errors++;
           }


       }

   }
}

void var_not_used(){
   /*
   struct symbol_list *aux = symbol_table;

   while(aux != NULL){

       if (aux->node != NULL) {
           if (getchild(aux->node, 1) != NULL)
               //printf("USED: %d  Category %s TOKEN %s\n", aux->used, category_name[aux->node->category], getchild(aux->node, 1)->token);
           if(aux->node->category == VarDecl){

               if(aux->used == 0 && getchild(aux->node, 1)->token != NULL){
                   printf("Line %d, column %d: Symbol %s declared but never used\n", aux->line, aux->column, getchild(aux->node, 1)->token);
                   semantic_errors++;
               }
           }
       }

       aux = aux->next;
   }
   */

   struct scope_list *aux_scope= lista_de_scopes;

   while (aux_scope != NULL) {
       struct symbol_list *aux = aux_scope->scope;
       while(aux != NULL){

           if (aux->node != NULL) {
               if (getchild(aux->node, 1) != NULL)
                   //printf("USED: %d  Category %s TOKEN %s LINE %d COLUMN %d\n", aux->used, category_name[aux->node->category], getchild(aux->node, 1)->token, aux->line, aux->column);
               if(aux->node->category == VarDecl){
                   if(aux->used == 0 && getchild(aux->node, 1)->token != NULL){
                       printf("Line %d, column %d: Symbol %s declared but never used\n", aux->line, aux->column, getchild(aux->node, 1)->token);
                       semantic_errors++;
                   }
               }
           }
           aux = aux->next;
       }
       aux_scope = aux_scope->next;
   }

}

// semantic analysis begins here, with the AST root node
int check_program(struct node *program) {

   symbol_table = (struct symbol_list *) malloc(sizeof(struct symbol_list));

   symbol_table->next = NULL;
   struct node_list *child = program->children;

   while((child = child->next) != NULL) {

       //printf("Category: %s\n", category_name[child->node->category]);
       if (child->node->category == VarDecl) {
           check_var(child->node);

       }
       else {
            
            check_function_parse1(child->node);
           
       }


   }

   child = program->children;

   while((child = child->next) != NULL) {

       //printf("Category: %s\n", category_name[child->node->category]);
       if (child->node->category == VarDecl) {
           //check_var(child->node);
       }
       else {
           if (child->node->declared_func != 1)
                check_function(child->node);
       }


   }
   var_not_used();

   return semantic_errors;
}


void erro_call_print(struct node *expression, struct symbol_list *scope){



           //int aux_dummy = 0;
           //int count_erros_call = 0;
           enum type type_func_user[50];
           //printf("OLA 25---->\n");

           for (int k = 0; k < 50; k++) {
               type_func_user[k] = no_type;
           }

           if (getchild(expression, 0)->tam == 0){
               getchild(expression, 0)->func_0_param = 1;
           }

           //printf("PASSEI 5\n");
           struct node_list *child_expression = expression->children;
           int count_c = 0;
           int j = 0;
           struct node *first_argument = NULL;
           struct node *second_argument = NULL;
           struct node_list *additional_arguments = NULL;


           //printf("PASSEI 6\n");


           while ((child_expression = child_expression->next) != NULL) {

               //printf("PASSEI 7\n");
               if (count_c == 1) { //2º filho ---> 1º argumento
                   //printf("PASSEI 8\n");
                   first_argument = child_expression->node;
                   enum type first_type = no_type;

                   if (first_argument->token == NULL || strcmp(category_name[first_argument->category],"Natural") == 0 || strcmp(category_name[first_argument->category],"Decimal")==0){
                       //check_expression(first_argument, scope);
                       //printf("--> %s\n",category_name[first_argument->category]);

                       if(strcmp(category_name[first_argument->category],"Call") == 0 ){
                           //printf("CALL\n");
                           //check_expression(first_argument, scope);
                           check_expression(first_argument, scope);

                           if(getchild(first_argument,0)->type!=undef){
                               first_type = search_symbol(symbol_table,getchild(first_argument,0)->token)->type;
                               type_func_user[j-1] = first_type;
                           }
                           else{

                               first_type = no_type;
                              type_func_user[j-1] = first_type;
                              //check_expression(getchild(first_argument, 0), scope); //---------------------------------------------------------------->
                               //printf("%s  %s\n",category_name[getchild(first_argument, 0)->category],getchild(first_argument, 0)->token);


                           }

                       }
                       else{

                               if(strcmp(category_name[first_argument->category],"Dummy") == 0 ){
                                   first_type = no_type;

                               }
                               else{
                                   //printf("%s  IMPORTANTE!\n", category_name[first_argument->category]);
                                   check_expression(first_argument, scope);
                                   first_type = first_argument->type;
                                   type_func_user[j-1] = first_type;
                               }



                       }

                   }
                   else if (search_symbol(scope, first_argument->token) != NULL || search_symbol(symbol_table, first_argument->token) != NULL){
                       //printf("PASSEI 9\n");

                       check_expression(first_argument, scope);
                       if (search_symbol(scope, first_argument->token) != NULL) {
                           first_type = search_symbol(scope, first_argument->token)->type;
                           type_func_user[j-1] = first_type;


                       }
                       else {
                           first_type = search_symbol(symbol_table, first_argument->token)->type;
                           type_func_user[j-1] = first_type;


                       }

                   }
                   else if(search_symbol(scope, first_argument->token) == NULL && search_symbol(symbol_table, first_argument->token) == NULL){ //variavel colocada como argumento nao existe

                       //printf("Variable %s (%d:%d) undeclared\n", first_argument->token, first_argument->token_line, first_argument->token_column);
                       printf("Line %d, column %d: Cannot find symbol %s\n",first_argument->token_line,first_argument->token_column,first_argument->token);
                       type_func_user[j-1] = no_type;
                       semantic_errors++;
                   }

                   //printf("PASSEI 10\n");
               }
               if (count_c == 2) { //3º filho ---> 2º argumento

                   second_argument = child_expression->node;
                   enum type second_type = no_type;

                   if (second_argument->token == NULL || strcmp(category_name[second_argument->category],"Natural") == 0 || strcmp(category_name[second_argument->category],"Decimal")==0){
                       //check_expression(second_argument, scope);

                       if(strcmp(category_name[second_argument->category],"Call") == 0 ){


                           check_expression(second_argument, scope);


                           if(getchild(second_argument,0)->type!=undef){
                               second_type = search_symbol(symbol_table,getchild(second_argument,0)->token)->type;
                               type_func_user[j-1] = second_type;
                           }
                           else{

                               second_type = no_type;
                               type_func_user[j-1] = second_type;
                               //check_expression(getchild(second_argument, 0), scope); //---------------------------------------------------------->

                           }

                       }
                       else{

                           check_expression(second_argument, scope);
                           second_type = second_argument->type;
                           type_func_user[j-1] = second_type;

                       }

                   }
                   else if (search_symbol(scope, second_argument->token) != NULL || search_symbol(symbol_table, second_argument->token) != NULL){
                       //printf("FODASSSEIYCFWIUYFGWIGI\n");

                       check_expression(second_argument, scope);
                       if (search_symbol(scope, second_argument->token) != NULL) {

                           second_type = search_symbol(scope, second_argument->token)->type;
                           type_func_user[j-1] = second_type;

                       }
                       else {

                           second_type = search_symbol(symbol_table, second_argument->token)->type;
                           type_func_user[j-1] = second_type;

                       }

                   }
                   else if(search_symbol(scope, second_argument->token) == NULL && search_symbol(symbol_table, second_argument->token) == NULL){

                       //printf("Variable %s (%d:%d) undeclared\n", second_argument->token, second_argument->token_line, second_argument->token_column);
                       printf("Line %d, column %d: Cannot find symbol %s\n",second_argument->token_line,second_argument->token_column,second_argument->token);
                       type_func_user[j-1] = no_type;
                       semantic_errors++;
                   }


                   additional_arguments = child_expression->node->children; // o additional_arguments é os filhos do segundo paramentro
                   //printf("PASSEI 21\n");
                   //printf("%s ------>DEVERIA SER ... %s\n",category_name[getchild(second_argument,0)->category], getchild(second_argument,0)->token);

                   while ((additional_arguments = additional_arguments->next) != NULL) {


                       enum type aux_type = no_type;

                       if(strcmp(category_name[additional_arguments->node->category],"Dummy") != 0){

                           //printf("DIFERENTE: %s\n",category_name[additional_arguments->node->category]);
                       }
                       else{
                           //printf("IGUAL : %s\n",category_name[additional_arguments->node->category]);


                           if (getchild(additional_arguments->node, 0)->token == NULL || strcmp(category_name[getchild(additional_arguments->node, 0)->category],"Natural") == 0 || strcmp(category_name[getchild(additional_arguments->node, 0)->category],"Decimal")==0){
                           //printf("PASSEI 23\n");
                           //printf("%s ------> \n",category_name[getchild(additional_arguments->node, 0)->category]);
                           //check_expression(getchild(additional_arguments->node, 0), scope);


                               if(strcmp(category_name[getchild(additional_arguments->node,0)->category],"Call") == 0 ){
                                   //printf("CALL\n");
                                   check_expression(getchild(additional_arguments->node, 0), scope);



                                   if(getchild(getchild(additional_arguments->node,0),0)->type!=undef){
                                       aux_type = search_symbol(symbol_table,getchild(getchild(additional_arguments->node,0),0)->token)->type;
                                       type_func_user[j] = aux_type;

                                   }
                                   else{


                                       aux_type = no_type;
                                       type_func_user[j] = aux_type;
                                       //check_expression(getchild(getchild(additional_arguments->node, 0),0), scope); //---------------------------------------------------------->


                                   }


                               }
                               else{
                                   check_expression(getchild(additional_arguments->node, 0), scope);
                                   aux_type = getchild(additional_arguments->node, 0)->type;
                                   type_func_user[j] = aux_type;


                               }

                           }    
                           else if (search_symbol(scope, getchild(additional_arguments->node, 0)->token) != NULL || search_symbol(symbol_table, getchild(additional_arguments->node, 0)->token) != NULL){

                               check_expression(getchild(additional_arguments->node, 0), scope);
                               if (search_symbol(scope, getchild(additional_arguments->node, 0)->token) != NULL) {
                                   aux_type = search_symbol(scope, getchild(additional_arguments->node, 0)->token)->type;
                                   type_func_user[j] = aux_type;


                               }
                               else {
                                   aux_type = search_symbol(symbol_table, getchild(additional_arguments->node, 0)->token)->type;
                                   type_func_user[j] = aux_type;


                               }
                           }
                           else if(search_symbol(scope, getchild(additional_arguments->node, 0)->token) == NULL){

                               //printf("Variable %s (%d:%d) undeclared\n", getchild(additional_arguments->node, 0)->token, getchild(additional_arguments->node, 0)->token_line, getchild(additional_arguments->node, 0)->token_column);
                               printf("Line %d, column %d: Cannot find symbol %s\n",getchild(additional_arguments->node, 0)->token_line,getchild(additional_arguments->node, 0)->token_column,getchild(additional_arguments->node, 0)->token);
                               semantic_errors++;
                               type_func_user[j] = no_type;
                           }
                           j++;

                       }



                   }
               }
               count_c++;
               j++;
           }


               printf("Line %d, column %d: Cannot find symbol %s(",getchild(expression,0)->token_line,getchild(expression,0)->token_column,getchild(expression,0)->token);
               for(int l = 0; l < j-1; l++){
                   if(l!=j-2){
                       printf("%s,",type_name(type_func_user[l]));
                   }
                   else{
                       printf("%s",type_name(type_func_user[l]));
                   }
               }
               printf(")\n");


}