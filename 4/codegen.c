#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "gocompiler.y"
#include "semantics.h"
#include "codegen.h"
//#include "semantics.c"
#define MAX 50


int temporary;   // sequence of temporary registers in a function
//extern char *category_name[];
extern const char *category_name[];
extern struct symbol_list *symbol_table;
extern struct scope_list *lista_de_scopes;
struct symbol_list *scope_func;

struct parameter{
 char name_var[100]; // Primeira string, tamanho máximo 100 caracteres
 char type_var[10]; // Segunda string, tamanho máximo 100 caracteres
 int tmp;
};


struct parameter parameters_global[MAX]; //guarda os parametros de cada função
struct parameter variaveis_locais[MAX*5];
int tam_variaveis_locais = 0; //indice do proximo sitio a colocar no vetor variaveis_locais
int contador_for_vazio = 1;//usado quando a expressao do for esta vazia
char tmp_plus_minus[256]; //usado no PLUS E no MINUS visto que o valor da variável da esquerda é irmao do nó PLUS/MINUS-> no assign guardo o valor do temporario para o usar no PLUS e no MINUS


//procura variavel na tabela parameters_global
int search_parameters_global(char * str){
 for(int i = 0; i < MAX; i++){
     if(strcmp(parameters_global[i].name_var,str) == 0){
         return i;
     }
 }
 return -1;
}

// caso em que é defenido um float32 do tipo: .e2
char* corrigirDecimal(char* decimal) {
 // Verifica se o ponteiro é nulo
 if (decimal == NULL) {
     char* vazio = strdup(""); // Retorna uma string vazia
     if (vazio == NULL) {
         fprintf(stderr, "Erro de alocação de memória\n");
         exit(1);
     }
     return vazio;
 }

 // Verifica se a string está vazia
 if (strlen(decimal) == 0) {
     char* vazio = strdup(""); // Retorna uma string vazia
     if (vazio == NULL) {
         fprintf(stderr, "Erro de alocação de memória\n");
         exit(1);
     }
     return vazio;
 }

 // Verifica se o primeiro caractere é '.'
 if (decimal[0] == '.') {
     // Aloca memória para o novo número com "0." adicionado
     size_t tamanho = strlen(decimal) + 2; // Espaço extra para '0' e '\0'
     char* corrigido = (char*)malloc(tamanho);
     if (corrigido == NULL) {
         fprintf(stderr, "Erro de alocação de memória\n");
         exit(1);
     }

     // Constrói a string corrigida
     corrigido[0] = '0';      // Adiciona '0'
     strcpy(corrigido + 1, decimal); // Copia o restante da string original
     return corrigido;
 }

 // Caso contrário, retorna uma cópia da string original
 char* copia = strdup(decimal);
 if (copia == NULL) {
     fprintf(stderr, "Erro de alocação de memória\n");
     exit(1);
 }
 return copia;
}


int search_variaveis_locais(char * str){
 for(int i = 0; i < MAX; i++){
     if(strcmp(variaveis_locais[i].name_var,str) == 0){
         return i;
     }
 }
 return -1;
}

void clear_parameters_global() {
 for (int i = 0; i < MAX; i++) {
     // Configura as strings como vazias
     parameters_global[i].name_var[0] = '\0';
     parameters_global[i].type_var[0] = '\0';

     // Configura o inteiro como zero
     parameters_global[i].tmp = 0;
 }
}

void clear_variaveis_locais() {
 for (int i = 0; i < MAX; i++) {
     // Configura as strings como vazias
     variaveis_locais[i].name_var[0] = '\0';
     variaveis_locais[i].type_var[0] = '\0';

     // Configura o inteiro como zero
     variaveis_locais[i].tmp = 0;
 }
}

/*
int llvm_string_size(const char *str) {
 int size = 0;
 for (const char *p = str; *p != '\0'; p++) {
     if (*p == '\\' || *p == '"') {
         size += 2;  // Escapes requerem dois caracteres no formato LLVM
     } else {
         size += 1;
     }
 }
 return size;
}
*/

/*
int llvm_string_size(const char *str) {
 int size = 0;
 for (const char *p = str; *p != '\0'; p++) {
     size++;  // Conta cada caractere visível
 }
 return size;  // Não adiciona \0 para LLVM (a menos que necessário)
}
*/




int llvm_string_size(const char *str) {
 int size = 0;
 int tam = strlen(str);
 const char *p = str;

 for (int i = 0; i < tam; i++, p++) {
     if (*p == '\\') {
         if (*(p+1) == 't' || *(p+1) == 'n' || *(p+1) == 'r' || *(p+1) == 'f') {
             size += 1;  // Contam como um caractere para LLVM
             p++;  // Pula o próximo caractere
             i++;
         }
         else if (*(p+1) == '%') {
             size += 2;  // Conta como dois caracteres
             p+=2;  // Pula o próximo caractere
             i+=2;    
         }
         else if (*(p+1) == '\\') {
             size += 1;  // Conta uma barra invertida como um caractere
             p++;  // Pula o próximo caractere
             i++;
         }
         else if (*(p+1) == '2' && *(p+2) == '2') {  // Aspas duplas escapadas como \22
             size += 1;  // Conta como um caractere
             p += 2;  // Pula os dois próximos caracteres
             i += 2;
         }

         else if (*(p+1) == '0' &&
                 ((*(p+2) >= '0' && *(p+2) <= '9') || (*(p+2) >= 'A' && *(p+2) <= 'F'))) {
             size += 1;  // Conta uma sequência hexadecimal \0X como um caractere
             p += 2;  // Pula os dois próximos caracteres
             i += 2;
         }
         else if (*(p+1) == '5' && *(p+2) == 'C') {
             size += 1;  // Conta uma sequência \00 como um caractere
             p += 2;  // Pula os dois próximos caracteres
             i += 2;
         }
         else {
             size += 1;  // Conta a barra como caractere comum
         }
     }
     else {
         //printf("Caracter normal somou 1 %c\n",*p);
         size += 1;  // Conta caracteres normais
     }
 }

 return size;
}




void process_escape_sequences(const char* input, char* output) {
 int i = 0, j = 0;
 int len = strlen(input);

 while (i < len) {
     if (input[i] == '\\' && i < len - 1) {
         if (input[i + 1] == 'n') {  // Nova linha
             output[j++] = '\\';
             output[j++] = '0';
             output[j++] = 'A';
             i += 2;
         }
         else if (input[i + 1] == 't') {  // Tabulação
             output[j++] = '\\';
             output[j++] = '0';
             output[j++] = '9';
             i += 2;
         }
         else if (input[i + 1] == 'f') {
             output[j++] = '\\';
             output[j++] = '0';
             output[j++] = 'C';
             i += 2;
         }
         else if (input[i + 1] == 'r') {
             output[j++] = '\\';
             output[j++] = '0';
             output[j++] = 'D';
             i += 2;
         }
         else if (input[i + 1] == '"') {  // Aspas duplas
             output[j++] = '\\';
             output[j++] = '2';
             output[j++] = '2';
             i += 2;
         }
         else if (input[i + 1] == '\\') {  // Barra invertida
             output[j++] = '\\';
             output[j++] = '5';
             output[j++] = 'C';
             i += 2;
         }
         else {
             // Copia qualquer sequência desconhecida
             output[j++] = input[i++];
         }
     }
     else if (input[i] == '"' && i != 0 && i != len - 1) {
         // Transforma aspas não iniciais e não finais
         output[j++] = '\\';
         output[j++] = '2';
         output[j++] = '2';
         i++;
     }
     else if (i == len - 1 && input[i] == '"')  {
         // Finaliza corretamente a string
         output[j++] = '\\';
         output[j++] = '0';
         output[j++] = 'A';
         output[j++] = '\\';
         output[j++] = '0';
         output[j++] = '0';
         output[j++] = input[i++];
     }
     else if (input[i] == '%') {
         output[j++] = '%';
         output[j++] = '%';
         i++;
     }
     else {
         output[j++] = input[i++];
     }
 }
 output[j] = '\0';  // Finaliza a string
}


/*
int llvm_string_size(const char *str) {  // pelo que percebi \t,\r,\n,\f contam todos como 2 caracteres so \\ é que conta como 1
 int size = 0;
 int aux = 0;
 for (const char *p = str; *p != '\0'; p++) {
     if(*p == '\\' && *(p+1) == '\\' && aux == 0){
         //size = size;
         aux = 1;
     }
     //else if(*p == '\%' && *(p+1) != ' ' && aux == 0){
         //size = size;
         //aux = 1;
     //}
     else{
         size++;
         aux = 0;
     }

 }
 return size;  // Não adiciona \0 para LLVM (a menos que necessário)
}
*/



char* transform_string(const char* input) {
 // Calcula o tamanho da nova string: tamanho da entrada + "\n" + "\0"
 size_t input_len = strlen(input);
 size_t new_len = input_len + 1; // +1  para '\0'

 // Aloca memória para a nova string
 char* transformed = (char*)malloc(new_len * sizeof(char));
 if (!transformed) {
     fprintf(stderr, "Erro ao alocar memória.\n");
     exit(1);
 }

 // Copia a string original
 strcpy(transformed, input);

 // Adiciona '\n' e '\0' ao final
 //transformed[input_len] = '\n';
 transformed[input_len ] = '\0';

 return transformed;
}


char* add_dot_before_e(const char* input) {
    size_t len = strlen(input);

    if (strchr(input, 'e') != NULL || strchr(input, 'E') != NULL) {
        const char* e_pos = strchr(input, 'e');
        if (!e_pos) {
            e_pos = strchr(input, 'E');
        }

        char* result = (char*)malloc(len + 3);
        if (result == NULL) {
            return NULL;
        }

        size_t base_len = e_pos - input;
        strncpy(result, input, base_len);
        result[base_len] = '\0';

        if (strchr(result, '.') == NULL) {
            result[base_len] = '.';
            result[base_len + 1] = '0';
            result[base_len + 2] = '\0';
        }

        strcat(result, e_pos);
        return result;
    }

    return strdup(input);
}




int contains_dot(const char *str) {
 // Percorre a string até encontrar o fim
 while (*str != '\0') {
     if (*str == '.') {
         return 1;  // Retorna 1 se encontrar o ponto
     }
     str++;  // Avança para o próximo caractere
 }
 return 0;  // Retorna 0 se não encontrar o ponto
}

int contains_e(const char *str) {
 // Percorre a string até encontrar o fim
 while (*str != '\0') {
     if (*str == 'e' || *str == 'E') {
         return 1;  // Retorna 1 se encontrar o ponto
     }
     str++;  // Avança para o próximo caractere
 }
 return 0;  // Retorna 0 se não encontrar o ponto
}


//quando chamo o type_name()
char* llvm_type(const char * type) {
 if (strcmp(type, "int") == 0) {
     return "i32";
 } else if (strcmp(type, "float32") == 0) {
     return "double";
 } else if (strcmp(type, "bool") == 0) {
     return "i1";
 } else if (strcmp(type, "string") == 0) {
     return "i8";
 } else {
     return "none";  // Valor padrão.
 }
}


//quando chamo o category_type() -> para o type nos parametros da função
char* llvm_type_category(enum type type){
 switch (type) {
    case integer_type:
        return "i32";
    case float32_type:
        return "double";
    case bool_type:
        return "i1";
    case string_type:
        return "i8";
    default:
        return "none";
}
}


struct parameter *codegen_parameters(struct node *parameters) {
 struct node *parameter;
 //struct parameter aux[50];
 struct parameter *aux = malloc(50 * sizeof(struct parameter));
 int curr = 0;
 while((parameter = getchild(parameters, curr++)) != NULL) {
     if(curr > 1)
         printf(", ");
     if(strcmp(llvm_type_category(category_type(getchild(parameter, 0)->category)),"i8") == 0){
         printf("%s* %%%s", llvm_type_category(category_type(getchild(parameter, 0)->category)),getchild(parameter, 1)->token);
     }
     else{
         printf("%s %%%s", llvm_type_category(category_type(getchild(parameter, 0)->category)),getchild(parameter, 1)->token);
     }

     strcpy(aux[curr-1].name_var,getchild(parameter, 1)->token);
     strcpy(aux[curr-1].type_var,llvm_type_category(category_type(getchild(parameter, 0)->category)));
     strcpy(parameters_global[curr-1].name_var,getchild(parameter, 1)->token);
     strcpy(parameters_global[curr-1].type_var,llvm_type_category(category_type(getchild(parameter, 0)->category)));

 }

 return aux;
}



int codegen_natural(struct node *natural) {
 printf("  %%%d = add %s %s, 0\n", temporary, llvm_type(type_name(natural->type)), natural->token);
 return temporary++;
}


int codegen_identifier(struct node *identifier) {

 if(search_parameters_global(identifier->token) != -1){
     if(identifier->type!=string_type){
         printf("  %%%d = load %s, %s* %%p%d\n", temporary, llvm_type(type_name(identifier->type)),llvm_type(type_name(identifier->type)),parameters_global[search_parameters_global(identifier->token)].tmp);
     }
     else{
         printf("  %%%d = load %s*, %s** %%p%d\n", temporary, llvm_type(type_name(identifier->type)),llvm_type(type_name(identifier->type)),parameters_global[search_parameters_global(identifier->token)].tmp);
     }
 }
 //else if(search_symbol(symbol_table, identifier->token)==NULL){
     //printf("  %%%d = load %s, %s* %%%s\n", temporary, llvm_type(type_name(identifier->type)),llvm_type(type_name(identifier->type)),identifier->token);

 //}  scope_func
 else if(search_variaveis_locais(identifier->token)!=-1){
     //---------------------------------------------------------------------->
     if(identifier->type!=string_type){
         printf("  %%%d = load %s, %s* %%%s\n", temporary, llvm_type(type_name(identifier->type)),llvm_type(type_name(identifier->type)),identifier->token);
     }
     else{
         printf("  %%%d = load %s*, %s** %%%s\n", temporary, llvm_type(type_name(identifier->type)),llvm_type(type_name(identifier->type)),identifier->token);
     }

 }
 else{
     if(identifier->type!=string_type){
         printf("  %%%d = load %s, %s* @%s\n", temporary, llvm_type(type_name(identifier->type)),llvm_type(type_name(identifier->type)),identifier->token);
     }
     else{
         printf("  %%%d = load %s*, %s** @%s\n", temporary, llvm_type(type_name(identifier->type)),llvm_type(type_name(identifier->type)),identifier->token);
     }
 }

 return temporary++;
}

int codegen_decimal(struct node *decimal) {
 int dot = contains_dot(decimal->token);
 int e = contains_e(decimal->token);

 if(e == 1){
     if(dot == 0){
         char *new_str = add_dot_before_e(decimal->token);
         
         //printf("------------------------------------>%s\n",new_str);
         printf("  %%%d = fadd %s %s, 0.0\n", temporary, llvm_type(type_name(decimal->type)), new_str);
     }
     else if(decimal->token[0] == '.'){
         char *new_str = corrigirDecimal(decimal->token); //mete um 0 antes do ponto
         printf("  %%%d = fadd %s %s, 0.0\n", temporary, llvm_type(type_name(decimal->type)), new_str);
     }
     else{
         printf("  %%%d = fadd %s %s, 0.0\n", temporary, llvm_type(type_name(decimal->type)), decimal->token);
     }
 }
 else{
     if(decimal->token[0] == '.'){
         char *new_str = corrigirDecimal(decimal->token); //mete um 0 antes do ponto
         printf("  %%%d = fadd %s %s, 0.0\n", temporary, llvm_type(type_name(decimal->type)), new_str);
     }
     else{
         printf("  %%%d = fadd %s %s, 0.0\n", temporary, llvm_type(type_name(decimal->type)), decimal->token);
     }
 }


 return temporary++;
}

//nos varDecl os identifier não tem tipo
int codegen_identifier_varDecl(struct node *identifier, enum category category){
 printf("  %%%d = load %s, %s* %%%s\n", temporary, llvm_type_category(category_type(category)), llvm_type_category(category_type(category)),identifier->token);
 return temporary++;
}

int codegen_expression(struct node *expression) {
 //printf("category :  %s\n",category_name[expression->category]);
 int tmp = -1;
 //printf("tmp : %d---------> %s\n",tmp,category_name[expression->category]);
 switch(expression->category) {
     case Natural:
         tmp = codegen_natural(expression);
         break;
     case Identifier:
         tmp = codegen_identifier(expression);
         break;
     case Decimal:
         tmp = codegen_decimal(expression);
         break;



     /*
     case StrLit:{

         char processed_output[2048];

         for (int i = 0; i < 2048; i++) {
             processed_output[i] = '\0';
         }
         process_escape_sequences(expression->token, processed_output);

         int size = llvm_string_size(processed_output);
         size = size - 2;


         //size = size +1;
         printf("  %%str_ptr%d = alloca [%d x i8]\n",temporary,size);
         if(search_symbol(symbol_table, expression->token)!= NULL){
             printf("  store [%d x i8] c%s, [%d x i8]* @str_ptr%d\n",size, processed_output,size,temporary);
         }
         else{
             printf("  store [%d x i8] c%s, [%d x i8]* %%str_ptr%d\n",size, processed_output,size,temporary);
         }
         //printf("  %%%d = load [%d x i8], [%d x i8]* %%str_ptr%d\n", temporary,size,size, temporary);
         printf("  %%%d = getelementptr [%d x i8], [%d x i8]* %%str_ptr%d, i32 0, i32 0\n", temporary,size,size,temporary);



         tmp = temporary++;
         //printf("TEMPORARY++ STRLIT\n");
         printf("  call i32 (i8*, ...) @printf(");
         printf("i8* %%%d)\n", tmp);
         //printf("  call void @free(i8* %%str_ptr%d)\n", temporary - 1); //--------------------------------------->novo
         tmp = temporary++;
         break;}
*/

     case StrLit: {
           char processed_output[2048 * 2];

           // Inicializar o buffer
           for (int i = 0; i < 2048 * 2; i++) {
               processed_output[i] = '\0';
           }
           process_escape_sequences(expression->token, processed_output);

           int size = llvm_string_size(processed_output) - 2;  // Ajustar o tamanho

           // Gerar código LLVM para alocação dinâmica (malloc)
           printf("  %%str_ptr%d = call i8* @malloc(i64 %d)\n", temporary, size);

           // Obter o ponteiro para o tipo correto
           printf("  %%str_array%d = bitcast i8* %%str_ptr%d to [%d x i8]*\n", temporary, temporary, size);

           // Gerar código para armazenar a string no espaço alocado
           if (search_symbol(symbol_table, expression->token) != NULL) {
               printf("  store [%d x i8] c%s, [%d x i8]* %%str_array%d\n", size, processed_output, size, temporary);
           } else {
               printf("  store [%d x i8] c%s, [%d x i8]* %%str_array%d\n", size, processed_output, size, temporary);
           }

           // Obter o ponteiro para a string (primeiro elemento do array)
           printf("  %%%d = getelementptr [%d x i8], [%d x i8]* %%str_array%d, i32 0, i32 0\n", temporary, size, size, temporary);

           tmp = temporary++;

           // Imprimir a string com printf
           printf("  call i32 (i8*, ...) @printf(i8* %%%d)\n", tmp);

           // Liberar o espaço alocado com malloc
           printf("  call void @free(i8* %%str_ptr%d)\n", temporary - 1);

           tmp = temporary++;
           break;
       }



     case Eq:{ //falta testar
         int left = codegen_expression(getchild(expression, 0));
         int right = codegen_expression(getchild(expression, 1));
         if(getchild(expression, 0)->type == integer_type || getchild(expression, 0)->type == bool_type){
             printf("  %%%d = icmp eq %s %%%d, %%%d\n", temporary, llvm_type(type_name(getchild(expression, 0)->type)), left ,right);
         }
         else{
             printf("  %%%d = fcmp oeq %s %%%d, %%%d\n", temporary, llvm_type(type_name(getchild(expression, 0)->type)), left ,right);
         }

         tmp = temporary++;
         //printf("TEMPORARY++ EQ\n");
         break;}
     case Ne:{ //falta testar
         int left = codegen_expression(getchild(expression, 0));
         int right = codegen_expression(getchild(expression, 1));
         if(getchild(expression, 0)->type == integer_type || getchild(expression, 0)->type == bool_type){
             printf("  %%%d = icmp ne %s %%%d, %%%d\n", temporary, llvm_type(type_name(getchild(expression, 0)->type)), left ,right);
         }
         else{
             printf("  %%%d = fcmp one %s %%%d, %%%d\n", temporary, llvm_type(type_name(getchild(expression, 0)->type)), left ,right);
         }
         tmp = temporary++;
         //printf("TEMPORARY++ NE\n");
         break;}
     case Add:
     case Sub:
     case Mul:
     case Div: {
         int left = codegen_expression(getchild(expression, 0));
         int right = codegen_expression(getchild(expression, 1));
         switch(expression->category) {
             case Add:

                 if(expression->type == integer_type){
                     printf("  %%%d = add %s %%%d, %%%d\n", temporary, llvm_type(type_name(expression->type)), left, right);
                 }
                 else{

                     printf("  %%%d = fadd %s %%%d, %%%d\n", temporary, llvm_type(type_name(expression->type)), left, right);
                 }


                 break;
             case Sub:
                 if(expression->type == integer_type){
                     printf("  %%%d = sub %s %%%d, %%%d\n", temporary, llvm_type(type_name(expression->type)),left, right);
                 }
                 else{
                     printf("  %%%d = fsub %s %%%d, %%%d\n", temporary, llvm_type(type_name(expression->type)),left, right);
                 }
                 break;
             case Mul:
                 if(expression->type == integer_type){
                     printf("  %%%d = mul %s %%%d, %%%d\n", temporary, llvm_type(type_name(expression->type)),left, right);
                 }
                 else{
                     printf("  %%%d = fmul %s %%%d, %%%d\n", temporary, llvm_type(type_name(expression->type)),left, right);
                 }
                 break;
             case Div:
                 if(expression->type == integer_type){
                     printf("  %%%d = sdiv %s %%%d, %%%d\n", temporary, llvm_type(type_name(expression->type)),left, right);
                 }
                 else{
                     printf("  %%%d = fdiv %s %%%d, %%%d\n", temporary, llvm_type(type_name(expression->type)),left, right);
                 }
                 break;
             default:
                 break;
         }
         tmp = temporary++;
         //printf("TEMPORARY++ ADD MUL DIV SUB\n");
         break; }
     case Assign: {
         // Obter os nós filho: destino (esquerda) e expressão (direita)
         struct node *destination = getchild(expression, 0); // variável de destino
         struct node *value = getchild(expression, 1);       // expressão atribuída

         if(getchild(expression, 1)->category == Plus || getchild(expression, 1)->category == Minus){
             //printf("ENTROU ASSIGN!\n");
             if(search_parameters_global(destination->token) != -1){
                 //printf("1\n");
                 snprintf(tmp_plus_minus, sizeof(tmp_plus_minus), "%%p%d", parameters_global[search_parameters_global(destination->token)].tmp);
                 //fprintf(tmp_plus_minus,"%%p%d", parameters_global[search_parameters_global(destination->token)].tmp);
             }
             else if(search_variaveis_locais(destination->token)!=-1){
                 //printf("2\n");
                 snprintf(tmp_plus_minus, sizeof(tmp_plus_minus), "%%%s", destination->token);
                 //fprintf(tmp_plus_minus,"%%%s", destination->token);
                 //printf("2.5\n");
             }
             else{
                 //printf("3\n");
                 snprintf(tmp_plus_minus, sizeof(tmp_plus_minus), "@%s", destination->token);
                 //fprintf(tmp_plus_minus,"@%s", destination->token);
             }
         }



         // Gerar o código para calcular o valor da expressão
         int value_tmp = codegen_expression(value);

         //printf("------------>%d\n",search_variaveis_locais(destination->token));
         // Gerar a instrução de armazenamento (store) para o destino
         if(search_parameters_global(destination->token) != -1){
                 if(destination->type!=string_type){
                     printf("  store %s %%%d, %s* %%p%d\n",
                     llvm_type(type_name(destination->type)),  // tipo da variável
                     value_tmp,                               // valor calculado
                     llvm_type(type_name(destination->type)), // tipo da variável
                     parameters_global[search_parameters_global(destination->token)].tmp);
                 }
                 else{
                     printf("  store %s* %%%d, %s** %%p%d\n",
                     llvm_type(type_name(destination->type)),  // tipo da variável
                     value_tmp,                               // valor calculado
                     llvm_type(type_name(destination->type)), // tipo da variável
                     parameters_global[search_parameters_global(destination->token)].tmp);
                 }




         }
         else if(search_variaveis_locais(destination->token)!=-1){
             //printf("ENTROU\n");
             if(destination->type!=string_type){
                 printf("  store %s %%%d, %s* %%%s\n",
                 llvm_type(type_name(destination->type)),  // tipo da variável
                 value_tmp,                               // valor calculado
                 llvm_type(type_name(destination->type)), // tipo da variável
                 destination->token);
             }
             else{
                 printf("  store %s* %%%d, %s** %%%s\n",
                 llvm_type(type_name(destination->type)),  // tipo da variável
                 value_tmp,                               // valor calculado
                 llvm_type(type_name(destination->type)), // tipo da variável
                 destination->token);
             }

         }
         else{
             if(destination->type!=string_type){
                 printf("  store %s %%%d, %s* @%s\n",
                     llvm_type(type_name(destination->type)),  // tipo da variável
                     value_tmp,                               // valor calculado
                     llvm_type(type_name(destination->type)), // tipo da variável
                     destination->token);    // nome da variável de destino
             }
             else{
                 printf("  store %s* %%%d, %s** @%s\n",
                     llvm_type(type_name(destination->type)),  // tipo da variável
                     value_tmp,                               // valor calculado
                     llvm_type(type_name(destination->type)), // tipo da variável
                     destination->token);
             }                
         }
         break;
     }
     case Return:
         if(strcmp(category_name[getchild(expression,0)->category],"Dummy1") == 0){
             printf("  ret void\n");
         }
         else{
             //printf("----------------------------------------->%s\n",category_name[getchild(expression,0)->category]);
             int value_tmp = codegen_expression(getchild(expression,0));
             //if(strcmp(category_name[getchild(expression,0)->category], "Identifier") == 0){
                 //printf("  ret %s %%%s\n", llvm_type(type_name(getchild(expression,0)->type)),getchild(expression,0)->token);
             //}
             //else{
                 if(getchild(expression,0)->type == string_type){
                     printf("  ret %s* %%%d\n", llvm_type(type_name(getchild(expression,0)->type)),value_tmp); //NOVO ---------------------------------------->
                 }
                 else{
                     printf("  ret %s %%%d\n", llvm_type(type_name(getchild(expression,0)->type)),value_tmp);
                 }

             //}

         }
         tmp = temporary++;
         //printf("TEMPORARY++ RETURN\n");
     break;

     case Dummy1:
         if(getchild(expression,0) !=  NULL){
             tmp = codegen_expression(getchild(expression,0));
         }
         break;

     case Mod:{
         int left = codegen_expression(getchild(expression, 0));
         int right = codegen_expression(getchild(expression, 1));

         printf("  %%%d  =  srem %s %%%d, %%%d\n", temporary,llvm_type(type_name(expression->type)), left, right);
         tmp = temporary++;
         //printf("TEMPORARY++ MOD\n");
         break;}

     case If:
                 tmp  = codegen_expression(getchild(expression,0));

                 printf("  br %s %%%d, label %%L%dthen, label %%L%delse\n",llvm_type(type_name(getchild(expression,0)->type)),tmp,tmp,tmp);

                 //1º Bloco
                 printf("L%dthen:\n",tmp);
                 /*Codigo*/
                 //printf("%s -->\n", category_name[getchild(expression,1)->category]);
                 codegen_expression(getchild(expression,1));

                 printf("  br label %%L%dend\n",tmp);

                 //2º Bloco
                 printf("L%delse:\n",tmp);
                 /*Codigo*/
                 codegen_expression(getchild(expression,2));

                 printf("  br label %%L%dend\n",tmp);

                 printf("L%dend:\n",tmp);


             //tmp = temporary++;//------------------------------------------------------------------------------------------_>
         break;

     case Block:{
         struct node *dummy1_node = getchild(expression,0);
         struct node_list *filhos_block = dummy1_node->children;

         while((filhos_block = filhos_block->next) != NULL){
             tmp = codegen_expression(filhos_block->node);
         }

         break;}

     case For:
         //printf("--->%s \n", category_name[getchild(expression,0)->category]);
         //printf("--->%s \n", category_name[getchild(expression,1)->category]);
         tmp = codegen_expression(getchild(expression,0));
         if(tmp >= 0){
             printf("  br %s %%%d, label %%loop%d , label %%L%dend\n",llvm_type(type_name(getchild(expression,0)->type)),tmp,tmp,tmp);
             printf("loop%d:\n",tmp);
             codegen_expression(getchild(expression,1));
             int tmp1 = codegen_expression(getchild(expression,0)); //acho que esta a mais
             printf("  br %s %%%d, label %%loop%d , label %%L%dend\n",llvm_type(type_name(getchild(expression,0)->type)),tmp1,tmp,tmp);
             printf("L%dend:\n",tmp);
         }
         else{
             printf("  %%for%d = alloca i1\n",contador_for_vazio);
             printf("  store i1 1, i1* %%for%d\n",contador_for_vazio); // se nao houver expressao no for eu nao entro no for evitando um ciclo infinito, se queiser o ciclo infinito basta colocar a 1 o primeiro valor desta instrução

             //%temp = alloca i1       ; Aloca espaço para uma variável do tipo i1
             //store i1 0, i1* %temp   ; Armazena o valor 0 na memória alocada
             //%cond1 = load i1, i1* %temp ; Carrega o valor 0 de volta para o temporário




             printf("  %%cond%d = load i1, i1* %%for%d\n",contador_for_vazio,contador_for_vazio);
             printf("  br i1 %%cond%d, label %%loopfor%d , label %%L%dforend\n",contador_for_vazio,contador_for_vazio,contador_for_vazio);
             printf("loopfor%d:\n",contador_for_vazio);
             int aux_contador_for_vazio = contador_for_vazio;
             contador_for_vazio++;
             codegen_expression(getchild(expression,1));
             //int tmp1 = codegen_expression(getchild(expression,0));
             printf("  br i1 %%cond%d, label %%loopfor%d , label %%L%dforend\n",aux_contador_for_vazio,aux_contador_for_vazio,aux_contador_for_vazio);
             printf("L%dforend:\n",aux_contador_for_vazio);

         }
         //tmp = temporary++; //------------------------------------------------------->
         //printf("TEMPORARY++ FOR\n");
         break;

     case Lt:{
         int left = codegen_expression(getchild(expression, 0));
         int right = codegen_expression(getchild(expression, 1));
         if(getchild(expression, 0)->type == integer_type){
             printf("  %%%d = icmp slt %s %%%d, %%%d\n", temporary, llvm_type(type_name(getchild(expression, 0)->type)), left ,right);
         }
         else{
             printf("  %%%d = fcmp olt %s %%%d, %%%d\n", temporary, llvm_type(type_name(getchild(expression, 0)->type)), left ,right);
         }

         tmp = temporary++;
         //printf("TEMPORARY++ LT\n");
         break;}
     case Gt:{
         int left = codegen_expression(getchild(expression, 0));
         int right = codegen_expression(getchild(expression, 1));
         if(getchild(expression, 0)->type == integer_type){
             printf("  %%%d = icmp sgt %s %%%d, %%%d\n", temporary, llvm_type(type_name(getchild(expression, 0)->type)), left ,right);
         }
         else{
             printf("  %%%d = fcmp ogt %s %%%d, %%%d\n", temporary, llvm_type(type_name(getchild(expression, 0)->type)), left ,right);
         }
         tmp = temporary++;
         //printf("TEMPORARY++ GT\n");

         break;}
     case Le:{
         int left = codegen_expression(getchild(expression, 0));
         int right = codegen_expression(getchild(expression, 1));
         if(getchild(expression, 0)->type == integer_type){
             printf("  %%%d = icmp sle %s %%%d, %%%d\n", temporary, llvm_type(type_name(getchild(expression, 0)->type)), left ,right);
         }
         else{
             printf("  %%%d = fcmp ole %s %%%d, %%%d\n", temporary, llvm_type(type_name(getchild(expression, 0)->type)), left ,right);
         }
         tmp = temporary++;
         //printf("TEMPORARY++ LE\n");

         break;}
     case Ge:{
         int left = codegen_expression(getchild(expression, 0));
         int right = codegen_expression(getchild(expression, 1));
         if(getchild(expression, 0)->type == integer_type){
             printf("  %%%d = icmp sge %s %%%d, %%%d\n", temporary, llvm_type(type_name(getchild(expression, 0)->type)), left ,right);
         }
         else{
             printf("  %%%d = fcmp oge %s %%%d, %%%d\n", temporary, llvm_type(type_name(getchild(expression, 0)->type)), left ,right);
         }
         tmp = temporary++;
         //printf("TEMPORARY++ GE\n");

         break;}

     case And:{
         int left = codegen_expression(getchild(expression, 0));
         int right = codegen_expression(getchild(expression, 1));

         printf("  %%%d = and %s %%%d, %%%d\n", temporary, llvm_type(type_name(getchild(expression, 0)->type)), left ,right);
         tmp = temporary++;
         //printf("TEMPORARY++ AND\n");

         break;}

     case Or:{
         int left = codegen_expression(getchild(expression, 0));
         int right = codegen_expression(getchild(expression, 1));

         printf("  %%%d = or %s %%%d, %%%d\n", temporary, llvm_type(type_name(getchild(expression, 0)->type)), left ,right);
         tmp = temporary++;
         //printf("TEMPORARY++ OR\n");

         break;}

     case Not: {
         // Obter o valor do filho da expressão
         int operand_tmp = codegen_expression(getchild(expression, 0));

         // Inverter o valor lógico: !x equivale a x == false
         printf("  %%%d = icmp eq %s %%%d, 0\n",temporary,llvm_type(type_name(getchild(expression, 0)->type)),operand_tmp); //isto faz um XOR em que põem-se numa variavel temporaria o valor contraria da variavel que estamos a negar

         // Incrementar o índice de temporários e retornar
         tmp = temporary++;
         //printf("TEMPORARY++ NOT\n");
         break;
     }
     case VarDecl:{
         //codegen_expression(getchild(expression, 0));
         enum category category =  getchild(expression, 0)->category; // vaiu servir para guardar o tipo
         if(strcmp(llvm_type_category(category_type(category)),"i8")!= 0){
             printf("  %%%s = alloca %s\n",getchild(expression,1)->token,llvm_type_category(category_type(category)));
         }
         else{
             printf("  %%%s = alloca %s*\n",getchild(expression,1)->token,llvm_type_category(category_type(category)));
         }
         //codegen_identifier_varDecl(getchild(expression,1),category);  //-----------------------------------------------------> Em principio isto nao é necessario
         strcpy(variaveis_locais[tam_variaveis_locais].name_var,getchild(expression,1)->token);
         tam_variaveis_locais++;
         //printf("----------%d------>\n",tam_variaveis_locais-1);
        if (getchild(expression, 2) != NULL) {
            struct node_list *vars = getchild(expression, 2)->children; //vars é o varDecl

            while((vars = vars->next) != NULL) {
                 struct node* aux = getchild(vars->node,1);
                 if(strcmp(llvm_type_category(category_type(category)),"i8")!= 0){
                     printf("  %%%s = alloca %s\n",aux->token,llvm_type_category(category_type(category)));
                 }
                 else{
                     printf("  %%%s = alloca %s*\n",aux->token,llvm_type_category(category_type(category)));
                 }
                 strcpy(variaveis_locais[tam_variaveis_locais].name_var,aux->token);
                 tam_variaveis_locais++;
                 //printf("----------%d------> v2.0\n",tam_variaveis_locais-1);
                 //codegen_identifier_varDecl(aux,category); //-----------------------------------------------------> Em principio isto nao é necessario
            }


        }

         break;}

     /*
     case Plus: {
         int operand_tmp = codegen_expression(getchild(expression, 0)); // Computar o operando
         if(getchild(expression, 0)->type == integer_type){
             printf("  %%%d = add %s %%%d, 0\n", temporary, llvm_type(type_name(getchild(expression, 0)->type)), operand_tmp);
         }
         else{
             printf("  %%%d = fadd %s %%%d, 0.0\n", temporary, llvm_type(type_name(getchild(expression, 0)->type)), operand_tmp);
         }
         tmp = temporary++;
         //printf("TEMPORARY++ PLUS\n");
         break;}


     case Minus:{
         int operand_tmp = codegen_expression(getchild(expression, 0)); // Computar o operando
         if(getchild(expression, 0)->type == integer_type){
             printf("  %%%d = sub %s %%%d, 0\n", temporary, llvm_type(type_name(getchild(expression, 0)->type)), operand_tmp);
         }
         else{
             printf("  %%%d = fsub %s %%%d, 0.0\n", temporary, llvm_type(type_name(getchild(expression, 0)->type)), operand_tmp);
         }
         tmp = temporary++;
         //printf("TEMPORARY++ MINUS\n");

         break;}

     */


    /*
     case Plus: {
         //printf("ENTROU\n");
         if(tmp_plus_minus[0]!='\0'){
             //printf("----------------------------> %s\n",tmp_plus_minus);
             int operand_tmp = codegen_expression(getchild(expression, 0)); // Computar o operando
             if(getchild(expression, 0)->type == integer_type){
                 //printf("integer\n");
                 //printf("  %%%d = add %s %%%d, 0\n", temporary, llvm_type(type_name(getchild(expression, 0)->type)), operand_tmp);
                 // %temp = load i32, i32* @x
                 printf("  %%%d = load i32, i32* %s\n",temporary,tmp_plus_minus);
                 tmp = temporary++;
                 printf("  %%%d = add %s %%%d, %%%d\n", temporary, llvm_type(type_name(getchild(expression, 0)->type)),operand_tmp, temporary-1);
             }
             else{
                 //printf("float\n");
                 //printf("  %%%d = fadd %s %%%d, 0.0\n", temporary, llvm_type(type_name(getchild(expression, 0)->type)), operand_tmp);
                 printf("  %%%d = load double, double* %s\n",temporary,tmp_plus_minus);
                 tmp = temporary++;
                 printf("  %%%d = fadd %s %%%d, %%%d\n", temporary, llvm_type(type_name(getchild(expression, 0)->type)),operand_tmp, temporary-1);
             }
             tmp = temporary++;
             for(int i = 0; i < 256; i++){
                 tmp_plus_minus[i] ='\0';
             }
         }
         else{
             tmp = codegen_expression(getchild(expression, 0));
         }
         //tmp_plus_minus = NULL;
         //printf("TEMPORARY++ PLUS\n");
         break;}


     case Minus:{
         if(tmp_plus_minus[0]!='\0'){
             //printf("----------------------------> %s\n",tmp_plus_minus);
             int operand_tmp = codegen_expression(getchild(expression, 0)); // Computar o operando
             if(getchild(expression, 0)->type == integer_type){
                 //printf("integer\n");
                 //printf("  %%%d = sub %s %%%d, 0\n", temporary, llvm_type(type_name(getchild(expression, 0)->type)), operand_tmp);
                 printf("  %%%d = load i32, i32* %s\n",temporary,tmp_plus_minus);
                 tmp = temporary++;
                 printf("  %%%d = sub %s %%%d, %%%d\n", temporary, llvm_type(type_name(getchild(expression, 0)->type)),operand_tmp, temporary -1);
             }
             else{
                 //printf("float\n");
                 //printf("  %%%d = fsub %s %%%d, 0.0\n", temporary, llvm_type(type_name(getchild(expression, 0)->type)), operand_tmp);
                 printf("  %%%d = load double, double* %s\n",temporary,tmp_plus_minus);
                 tmp = temporary++;
                 printf("  %%%d = fsub %s %%%d, %%%d\n", temporary, llvm_type(type_name(getchild(expression, 0)->type)),operand_tmp, temporary-1);
             }
             tmp = temporary++;
             for(int i = 0; i < 256; i++){
                 tmp_plus_minus[i] ='\0';
             }
         }else{
             int operand_tmp = codegen_expression(getchild(expression, 0));
             if(expression->type == integer_type){
                     printf("  %%%d = mul %s -1, %%%d\n", temporary, llvm_type(type_name(expression->type)),operand_tmp);
                 }
                 else{
                     printf("  %%%d = fmul %s -1.0, %%%d\n", temporary, llvm_type(type_name(expression->type)),operand_tmp);
                 }
                 tmp = temporary++;


         }
         //tmp_plus_minus = NULL;
         //printf("TEMPORARY++ MINUS\n");

         break;}
         */


     case Plus: {
         //printf("ENTROU\n");
         int operand_tmp = codegen_expression(getchild(expression, 0));
             if(expression->type == integer_type){
                     printf("  %%%d = mul %s 1, %%%d\n", temporary, llvm_type(type_name(expression->type)),operand_tmp);
                 }
                 else{
                     printf("  %%%d = fmul %s 1.0, %%%d\n", temporary, llvm_type(type_name(expression->type)),operand_tmp);
                 }
                 tmp = temporary++;
         //tmp_plus_minus = NULL;
         //printf("TEMPORARY++ PLUS\n");
         break;}


     case Minus:{

             int operand_tmp = codegen_expression(getchild(expression, 0));
             if(expression->type == integer_type){
                     printf("  %%%d = mul %s -1, %%%d\n", temporary, llvm_type(type_name(expression->type)),operand_tmp);
                 }
                 else{
                     printf("  %%%d = fmul %s -1.0, %%%d\n", temporary, llvm_type(type_name(expression->type)),operand_tmp);
                 }
                 tmp = temporary++;



         //tmp_plus_minus = NULL;
         //printf("TEMPORARY++ MINUS\n");

         break;}


     case Print: { //Não tenho a certeza ----> esta mal de cereteza
         struct node *to_print = getchild(expression, 0);  // Obter a expressão que será impressa

         // Gerar o código para a expressão
         int tmp = codegen_expression(to_print);

         // Definir o formato da string para o printf
         const char *format;
         if (to_print->type == integer_type) {
             format = "%d";  // Formato para inteiros
         } else if (to_print->type == float32_type) {
             format = "%.08f";  // Formato para floats
         } else if (to_print->type == bool_type) {
             format = "%d";  // Formato para booleanos (0 ou 1)
         } else if (to_print->type == string_type) {
             format = "%s";  // Formato para strings
         } else {
             format = "%s";  // Formato padrão (se não houver outro)
         }

           if(format!=NULL){}
         // Gerar a chamada para printf
         //printf("  call i32 (i8*, ...) @printf(i8* getelementptr (i8, i8* getelementptr (i8, i8* null, i32 0), i32 0), ");

         if(strcmp(llvm_type(type_name(to_print->type)),"i8") == 0){
             //printf("  call i32 (i8*, ...) @printf(");
             //printf("  call i32 (i8*, ...) @printf(");
             //printf("%s* %%%d)\n", llvm_type(type_name(to_print->type)), tmp);
         }
         else{

           /*
             if (to_print->type == integer_type) {
                 printf("  %%str_ptr%d = alloca [%lu x i8]\n",tmp,strlen(format) + 2);
                 printf("  store [%lu x i8] c\"%%d\\0A\\00\", [%lu x i8]* %%str_ptr%d\n",strlen(format) + 2,strlen(format) + 2,tmp);
                 printf("  %%str%d = getelementptr [%lu x i8], [%lu x i8]* %%str_ptr%d, i32 0, i32 0\n",tmp,strlen(format) + 2,strlen(format) + 2,tmp);

                 printf("  call i32 (i8*, ...) @printf(");
                 printf("i8* %%str%d, %s %%%d)\n", tmp,llvm_type(type_name(to_print->type)),tmp);

                 tmp = temporary++;

                 //printf("%s %%%d)\n", llvm_type(type_name(to_print->type)), tmp);  // Passar o argumento da expressão para inteiros e booleanos
             }

             */




            if (to_print->type == integer_type) {
               const char* format = "%d";
               size_t format_len = strlen(format) + 2;  // +1 para '\0' e +2 para '\0A\00'

               // Alocar memória dinamicamente usando malloc
               printf("  %%str_ptr%d = call i8* @malloc(i64 %lu)\n", tmp, format_len);

               // Fazer cast do ponteiro alocado para o tipo correto
               printf("  %%str_array%d = bitcast i8* %%str_ptr%d to [%lu x i8]*\n", tmp, tmp, format_len);

               // Armazenar a string de formato no espaço alocado
               printf("  store [%lu x i8] c\"%%d\\0A\\00\", [%lu x i8]* %%str_array%d\n",
                   format_len, format_len, tmp);

               // Obter o ponteiro para o primeiro elemento da string
               printf("  %%str%d = getelementptr [%lu x i8], [%lu x i8]* %%str_array%d, i32 0, i32 0\n",
                   tmp, format_len, format_len, tmp);

               // Chamar printf para imprimir o valor inteiro com o formato
               printf("  call i32 (i8*, ...) @printf(i8* %%str%d, %s %%%d)\n",
                   tmp, llvm_type(type_name(to_print->type)), tmp);

               // Liberar a memória alocada
               printf("  call void @free(i8* %%str_ptr%d)\n", tmp);

               tmp = temporary++;
           }

             /*
             else if (to_print->type == float32_type) {
                 const char* format = "%.08f";
                 size_t format_len = strlen(format) + 2;  // +1 para '\0'

                 printf("  %%str_ptr%d = alloca [%lu x i8]\n", tmp, format_len);
                 printf("  store [%lu x i8] c\"%%.08f\\0A\\00\", [%lu x i8]* %%str_ptr%d\n",
                     format_len, format_len, tmp);

                 printf("  %%str%d = getelementptr [%lu x i8], [%lu x i8]* %%str_ptr%d, i32 0, i32 0\n",
                     tmp, format_len, format_len, tmp);

                 printf("  call i32 (i8*, ...) @printf(i8* %%str%d, %s %%%d)\n",
                     tmp, llvm_type(type_name(to_print->type)), tmp);

                 tmp = temporary++;
             }
*/
               else if (to_print->type == float32_type) {
                   const char* format = "%.08f";
                   size_t format_len = strlen(format) + 2;  // +1 para '\0' e +2 para '\0A\00'

                   // Alocar memória dinamicamente usando malloc
                   printf("  %%str_ptr%d = call i8* @malloc(i64 %lu)\n", tmp, format_len);

                   // Fazer cast do ponteiro alocado para o tipo correto
                   printf("  %%str_array%d = bitcast i8* %%str_ptr%d to [%lu x i8]*\n", tmp, tmp, format_len);

                   // Armazenar a string de formato no espaço alocado
                   printf("  store [%lu x i8] c\"%%.08f\\0A\\00\", [%lu x i8]* %%str_array%d\n",
                       format_len, format_len, tmp);

                   // Obter o ponteiro para o primeiro elemento da string
                   printf("  %%str%d = getelementptr [%lu x i8], [%lu x i8]* %%str_array%d, i32 0, i32 0\n",
                       tmp, format_len, format_len, tmp);

                   // Chamar printf para imprimir o valor float com o formato
                   printf("  call i32 (i8*, ...) @printf(i8* %%str%d, %s %%%d)\n",
                       tmp, llvm_type(type_name(to_print->type)), tmp);

                   // Liberar a memória alocada
                   printf("  call void @free(i8* %%str_ptr%d)\n", tmp);

                   tmp = temporary++;
               }

             else if(to_print->type == bool_type) {  

                 // Comparação booleana
                 printf("  br i1 %%%d, label %%true%dthen, label %%false%delse\n", tmp, tmp, tmp+2);

                 // Bloco verdadeiro
                 printf("true%dthen:\n", tmp);
                 printf("  %%true_str%d = getelementptr [6 x i8], [6 x i8]* @.fmtstr_true, i32 0, i32 0\n", tmp);
                 tmp = temporary++;
                 printf("  call i32 (i8*, ...) @printf(i8* %%true_str%d)\n", tmp-1);
                 printf("  br label %%bool%dend\n", tmp);
                tmp = temporary++;

                 // Bloco falso
                 temporary = temporary -2;
                 printf("false%delse:\n", tmp);
                 printf("  %%false_str%d = getelementptr [7 x i8], [7 x i8]* @.fmtstr_false, i32 0, i32 0\n", tmp);
                 tmp = temporary++;
                 printf("  call i32 (i8*, ...) @printf(i8* %%false_str%d)\n", tmp+1);
                 printf("  br label %%bool%dend\n", tmp);

                 // Bloco final
                 printf("bool%dend:\n", tmp);
                 tmp = temporary++;
             }
             //printf("%s %%%d)\n", llvm_type(type_name(to_print->type)), tmp);  // Passar o argumento da expressão //----->ver se type esta bem
         }




         //tmp = temporary++;
         break;
     }

     /*
     case Print:{

         struct node *to_print = getchild(expression, 0);  // Obter a expressão que será impressa




         // Gerar o código para a expressão
         int tmp = codegen_expression(to_print);

         if(to_print->type == integer_type){
             printf("  call i32 @_write_int(i32 %%%d)\n",tmp);
         }
         else if(to_print->type == float32_type){
             printf("  call i32 @_write_float(double %%%d)\n",tmp);
         }
         else if(to_print->type == string_type){
             printf("  call i32 @_write_string(i8* %%%d)\n",tmp);
         }
         else if(to_print->type == bool_type){
             printf("  call i32 @_write_bool(i1 %%%d)\n",tmp);
         }

         tmp = temporary++;
         //printf("TEMPORARY++ PRINT\n");
         break;}
         */

     case Call:{

        struct node_list *child_expression = expression->children;
        int count_c = 0;
        int j = 0;
        struct node *first_argument = NULL;
        struct node *second_argument = NULL;
        struct node_list *additional_arguments = NULL;
        int v[50];


        while ((child_expression = child_expression->next) != NULL) {
            //printf("Tem argumentos %s\n", category_name[child_expression->node->category]);

            if (count_c == 1) { //2º filho ---> 1º argumento

                 first_argument = child_expression->node;
                 int operand_tmp = codegen_expression(first_argument);

                 //printf("  %%callp%d = load %s, %s* %%%d\n", operand_tmp, llvm_type(type_name(first_argument->type)),llvm_type(type_name(first_argument->type)),operand_tmp);
                 v[j-1] = operand_tmp;
                 //tmp = operand_tmp;
                 //printf("%s %%%d",llvm_type(type_name(getchild(expression,0)->type_call[j-1])),operand_tmp);

            }
            if (count_c == 2) { //3º filho ---> 2º argumento
                 second_argument = child_expression->node;
                 int operand_tmp = codegen_expression(second_argument);
                 //printf("  %%callp%d = load %s, %s* %%%d\n", operand_tmp, llvm_type(type_name(second_argument->type)),llvm_type(type_name(second_argument->type)),operand_tmp);
                 //printf("%s %%%d",llvm_type(type_name(getchild(expression,0)->type_call[j-1])),operand_tmp);
                 v[j-1] = operand_tmp;
                 //tmp = operand_tmp;


                additional_arguments = child_expression->node->children; // o additional_arguments é os filhos do segundo paramentro

                //printf("%s ------>DEVERIA SER ... %s\n",category_name[getchild(second_argument,0)->category], getchild(second_argument,0)->token);

                while ((additional_arguments = additional_arguments->next) != NULL) {

                    if(strcmp(category_name[additional_arguments->node->category],"Dummy") != 0){

                        //printf("DIFERENTE: %s\n",category_name[additional_arguments->node->category]);
                        //------------------------------------------------------------------------------------------------------------------------>
                    }
                    else{
                         int operand_tmp = codegen_expression(getchild(additional_arguments->node,0));
                         //printf("  %%callp%d = load %s, %s* %%%d\n", operand_tmp, llvm_type(type_name(getchild(additional_arguments->node,0)->type)),llvm_type(type_name(getchild(additional_arguments->node,0)->type)),operand_tmp);
                         //printf("%s %%%d",llvm_type(type_name(getchild(expression,0)->type_call[j])),operand_tmp);
                         v[j] = operand_tmp;
                         //tmp = operand_tmp;
                         j++;

                    }

                }
            }
            count_c++;
            j++;
        }

         if(strcmp(llvm_type(type_name(expression->type)),"none") == 0){
             printf(  "  call void @_%s(", getchild(expression,0)->token);
         }
         else{
             if(expression->type!=string_type){
                 printf(  "  %%%d = call %s @_%s(", temporary,llvm_type(type_name(expression->type)), getchild(expression,0)->token);
             }
             else{
                 printf(  "  %%%d = call %s* @_%s(", temporary,llvm_type(type_name(expression->type)), getchild(expression,0)->token);
             }
             tmp = temporary++;
             //printf("TEMPORARY++ CALL\n");
         }

         for(int i = 1; i < j; i++){
             if(v[i - 1] >= 0){
                 if(i == 1){
                     if(getchild(expression,0)->type_call_user[i-1] != string_type){
                         printf("%s %%%d",llvm_type(type_name(getchild(expression,0)->type_call_user[i-1])),v[i-1]);
                     }
                     else{
                         printf("%s* %%%d",llvm_type(type_name(getchild(expression,0)->type_call_user[i-1])),v[i-1]);
                     }

                 }
                 else{
                     if(getchild(expression,0)->type_call_user[i-1] != string_type){
                         printf(",%s %%%d",llvm_type(type_name(getchild(expression,0)->type_call_user[i-1])),v[i-1]);
                     }
                     else{
                         printf(",%s* %%%d",llvm_type(type_name(getchild(expression,0)->type_call_user[i-1])),v[i-1]);
                     }
                 }
             }


         }

         printf(")\n");
         //printf("%d     :    %d\n",tmp, temporary);
         //tmp = temporary++;

        break;}

     case ParseArgs: {

         //%arg1_ptr = getelementptr i8*, i8** %argv, i32 1 ; Pegar o segundo argumento (os.Args[1])
         //%arg1 = load i8*, i8** %arg1_ptr          ; Carregar o valor do argumento
         //%arg1_int = call i32 @atoi(i8* %arg1)     ; Converter para inteiro com atoi
         //store i32 %arg1_int, i32* %argument       ; Armazenar o valor convertido em 'argument'


       /*

       if(search_parameters_global(destination->token) != -1){
                 if(destination->type!=string_type){
                     printf("  store %s %%%d, %s* %%p%d\n",
                     llvm_type(type_name(destination->type)),  // tipo da variável
                     value_tmp,                               // valor calculado
                     llvm_type(type_name(destination->type)), // tipo da variável
                     parameters_global[search_parameters_global(destination->token)].tmp);
                 }
                 else{
                     printf("  store %s* %%%d, %s** %%p%d\n",
                     llvm_type(type_name(destination->type)),  // tipo da variável
                     value_tmp,                               // valor calculado
                     llvm_type(type_name(destination->type)), // tipo da variável
                     parameters_global[search_parameters_global(destination->token)].tmp);
                 }




         }
         else if(search_variaveis_locais(destination->token)!=-1){
             //printf("ENTROU\n");
             if(destination->type!=string_type){
                 printf("  store %s %%%d, %s* %%%s\n",
                 llvm_type(type_name(destination->type)),  // tipo da variável
                 value_tmp,                               // valor calculado
                 llvm_type(type_name(destination->type)), // tipo da variável
                 destination->token);
             }
             else{
                 printf("  store %s* %%%d, %s** %%%s\n",
                 llvm_type(type_name(destination->type)),  // tipo da variável
                 value_tmp,                               // valor calculado
                 llvm_type(type_name(destination->type)), // tipo da variável
                 destination->token);
             }

         }
         else{
             if(destination->type!=string_type){
                 printf("  store %s %%%d, %s* @%s\n",
                     llvm_type(type_name(destination->type)),  // tipo da variável
                     value_tmp,                               // valor calculado
                     llvm_type(type_name(destination->type)), // tipo da variável
                     destination->token);    // nome da variável de destino
             }
             else{
                 printf("  store %s* %%%d, %s** @%s\n",
                     llvm_type(type_name(destination->type)),  // tipo da variável
                     value_tmp,                               // valor calculado
                     llvm_type(type_name(destination->type)), // tipo da variável
                     destination->token);
             }                
         }







       */



         int index_tmp = codegen_expression(getchild(expression, 1));
         //printf("%s ---------->\n",category_name[getchild(expression, 1)->category]);
         // Obter o ponteiro para o argumento em argv[index]
         printf("  %%%d = getelementptr i8*, i8** %%argv, i32 %%%d\n", temporary, index_tmp);
         int arg_ptr_tmp = temporary++;

         // Carregar o argumento (string)
         printf("  %%%d = load i8*, i8** %%%d\n", temporary, arg_ptr_tmp);
         int arg_str_tmp = temporary++;

         if(search_parameters_global(getchild(expression, 0)->token) != -1){

           if (expression->type == integer_type) {
             printf("  %%%d = call i32 @atoi(i8* %%%d)\n", temporary, arg_str_tmp);
             printf("  store i32 %%%d, i32* %%p%d\n",temporary,parameters_global[search_parameters_global(getchild(expression, 0)->token)].tmp);
             //printf("  %%%s = call i32 @atoi(i8* %%%d)\n", getchild(expression,0)->token, arg_str_tmp);
           } else if (expression->type == float32_type) {
               printf("  %%%d = call double @atof(i8* %%%d)\n", temporary, arg_str_tmp);
               printf("  store double %%%d, double* %%p%d\n",temporary,parameters_global[search_parameters_global(getchild(expression, 0)->token)].tmp);
               //printf("  %%%s = call double @atof(i8* %%%d)\n", getchild(expression,0)->token, arg_str_tmp);
           }

         }
        else if(search_variaveis_locais(getchild(expression, 0)->token)!=-1){
           if (expression->type == integer_type) {
             printf("  %%%d = call i32 @atoi(i8* %%%d)\n", temporary, arg_str_tmp);
             printf("  store i32 %%%d, i32* %%%s\n",temporary,getchild(expression,0)->token);
             //printf("  %%%s = call i32 @atoi(i8* %%%d)\n", getchild(expression,0)->token, arg_str_tmp);
           } else if (expression->type == float32_type) {
               printf("  %%%d = call double @atof(i8* %%%d)\n", temporary, arg_str_tmp);
               printf("  store double %%%d, double* %%%s\n",temporary,getchild(expression,0)->token);
               //printf("  %%%s = call double @atof(i8* %%%d)\n", getchild(expression,0)->token, arg_str_tmp);
           }

        }
         else{
           if (expression->type == integer_type) {
             printf("  %%%d = call i32 @atoi(i8* %%%d)\n", temporary, arg_str_tmp);
             printf("  store i32 %%%d, i32* @%s\n",temporary,getchild(expression,0)->token);
             //printf("  %%%s = call i32 @atoi(i8* %%%d)\n", getchild(expression,0)->token, arg_str_tmp);
           } else if (expression->type == float32_type) {
               printf("  %%%d = call double @atof(i8* %%%d)\n", temporary, arg_str_tmp);
               printf("  store double %%%d, double* @%s\n",temporary,getchild(expression,0)->token);
               //printf("  %%%s = call double @atof(i8* %%%d)\n", getchild(expression,0)->token, arg_str_tmp);
           }


         }








           /*
         // Converter o argumento para inteiro, se necessário
         if (expression->type == integer_type) {
             printf("  %%%d = call i32 @atoi(i8* %%%d)\n", temporary, arg_str_tmp);
             printf("  store i32 %%%d, i32* %%%s\n",temporary,getchild(expression,0)->token);
             //printf("  %%%s = call i32 @atoi(i8* %%%d)\n", getchild(expression,0)->token, arg_str_tmp);
         } else if (expression->type == float32_type) {
             printf("  %%%d = call double @atof(i8* %%%d)\n", temporary, arg_str_tmp);
             printf("  store double %%%d, double* %%%s\n",temporary,getchild(expression,0)->token);
             //printf("  %%%s = call double @atof(i8* %%%d)\n", getchild(expression,0)->token, arg_str_tmp);
         }
*/


         tmp = temporary++;  
         //printf("TEMPORARY++ PARSEARGS\n");
         break;
     }


     default:
         break;
 }
 return tmp;
}



void codegen_var(struct node *expression) {
 //codegen_expression(getchild(expression, 0));
 enum category category =  getchild(expression, 0)->category; // vaiu servir para guardar o tipo
 //printf("  %%%s = alloca %s\n",getchild(expression,1)->token,llvm_type_category(category_type(category)));
 //codegen_identifier_varDecl(getchild(expression,1),category);  //-----------------------------------------------------> Em principio isto nao é necessario
 //@x = global i32 0  ;
 if(strcmp(llvm_type_category(category_type(category)),"i8") != 0){
     printf("@%s = global %s ",getchild(expression,1)->token,llvm_type_category(category_type(category)));
 }
 else{
     printf("@%s = global %s* ",getchild(expression,1)->token,llvm_type_category(category_type(category)));
 }

 if(strcmp(llvm_type_category(category_type(category)),"i32") == 0 ||strcmp(llvm_type_category(category_type(category)),"i1") == 0 ){
     printf("0\n");
 }
 else if(strcmp(llvm_type_category(category_type(category)),"i8") == 0){
     printf(" null\n");
 }
 else{
     printf("0.0\n");
 }

 if (getchild(expression, 2) != NULL) {
     struct node_list *vars = getchild(expression, 2)->children; //vars é o varDecl

     while((vars = vars->next) != NULL) {
         //struct node* aux = getchild(vars->node,1);
         //printf("  %%%s = alloca %s\n",aux->token,llvm_type_category(category_type(category)));
         //codegen_identifier_varDecl(aux,category); //-----------------------------------------------------> Em principio isto nao é necessario
         //printf("------------->%s\n", getchild(vars->node,1)->token);
         if(strcmp(llvm_type_category(category_type(category)),"i8") != 0){
             printf("@%s = global %s ",getchild(vars->node,1)->token,llvm_type_category(category_type(category))); //onde esta getchild(vars->node,1)->token estava getchild(expression,1)->token
         }
         else{
             printf("@%s = global %s* ",getchild(vars->node,1)->token,llvm_type_category(category_type(category)));
         }


         if(strcmp(llvm_type_category(category_type(category)),"i32") == 0 ||strcmp(llvm_type_category(category_type(category)),"i1") == 0 ){
             printf("0\n");
         }
         else if(strcmp(llvm_type_category(category_type(category)),"i8") == 0){
             printf(" null\n");
         }
         else{
             printf("0.0\n");
         }
     }


 }

}




void codegen_function(struct node *function) {  //function == FuncDecl

 temporary = 1;
 //int tmp = 0;
 struct node *funcHeader = getchild(function,0);
 struct symbol_list *symbol = search_symbol(symbol_table, getchild(funcHeader, 0)->token);

 if(strcmp(llvm_type(type_name(symbol->type)),"none")!= 0 && strcmp(getchild(funcHeader, 0)->token,"main") != 0){
     if(symbol->type == string_type){
         printf("define %s* @_%s(",llvm_type(type_name(symbol->type)), getchild(funcHeader, 0)->token);
     }
     else{
         printf("define %s @_%s(",llvm_type(type_name(symbol->type)), getchild(funcHeader, 0)->token);
     }

 }
 else if(strcmp(getchild(funcHeader, 0)->token,"main") != 0){
     printf("define void @_%s(", getchild(funcHeader, 0)->token);
 }
 else if(strcmp(getchild(funcHeader, 0)->token,"main") == 0 && strcmp(llvm_type(type_name(symbol->type)),"none")!= 0){
     if(symbol->type == string_type){
         printf("define %s* @_%s(",llvm_type(type_name(symbol->type)), getchild(funcHeader, 0)->token);
     }
     else{
         printf("define %s @_%s(",llvm_type(type_name(symbol->type)), getchild(funcHeader, 0)->token);
     }
 }
 else if(strcmp(getchild(funcHeader, 0)->token,"main") == 0){
     printf("define void @_%s(i32 %%argc, i8** %%argv", getchild(funcHeader, 0)->token);
 }

 //struct parameter aux[50];

 //struct parameter *aux = malloc(50 * sizeof(struct parameter));
 if(getchild(funcHeader, 1)!= NULL && strcmp(llvm_type_category(category_type(getchild(funcHeader, 1)->category)),"none")== 0){
     codegen_parameters(getchild(funcHeader, 1));
 }
 else if(getchild(funcHeader, 2)!= NULL && strcmp(llvm_type_category(category_type(getchild(funcHeader, 1)->category)),"none")!= 0){
     codegen_parameters(getchild(funcHeader, 2));
 }

 printf(") {\n");



 //guardar o scope desta função na variavel scope_func

 struct scope_list *current = lista_de_scopes; // Começa na cabeça da lista
 scope_func = NULL;
 // Percorre até o final da lista
 while (current != NULL) {
     // Acessar o nó do escopo atual
     //show_scope(current->scope,current->token);
     if(strcmp(current->token,getchild(funcHeader, 0)->token) == 0){
         scope_func = current->scope;
     }
     // Move para o próximo nó
     current = current->next;
 }
 //GUARDAR PARAMETROS E VARIAVEIS GLOBAIS NO VETOR parameters_global de modo a colocar estes valores todos em variáveis temporarias do tipo exemplo : %p1
 //parametros
 int tmp_aux = 1;
 for(int i = 0; i < 50; i++){
     if(parameters_global[i].name_var[0]!='\0'){
         //printf("  %%%s = alloca %s\n",parameters_global[i].name_var,parameters_global[i].type_var);
         if(strcmp(parameters_global[i].type_var,"i8") != 0){
             printf("  %%p%d = alloca %s\n",tmp_aux,parameters_global[i].type_var);
         }
         else{
             //printf("---------->ENTROU\n");
             printf("  %%p%d = alloca %s*\n",tmp_aux,parameters_global[i].type_var);
         }
         if(strcmp(parameters_global[i].type_var,"i8") != 0){
             printf("  store %s %%%s, %s* %%p%d\n",parameters_global[i].type_var,parameters_global[i].name_var,parameters_global[i].type_var,tmp_aux);
         }
         else{
             printf("  store %s* %%%s, %s** %%p%d\n",parameters_global[i].type_var,parameters_global[i].name_var,parameters_global[i].type_var,tmp_aux);
         }

         parameters_global[i].tmp = tmp_aux;
         tmp_aux = tmp_aux + 1;
     }
 }

 //variaveis globais ------>Nao devo alocar espaço para as globais e troca las por temporarios
 /*
 struct symbol_list *symbol1;

 for(symbol1 = symbol_table->next; symbol1 != NULL; symbol1 = symbol1->next) {
     if (symbol1->node != NULL && symbol1->node->category != FuncHeader && search_parameters_global(symbol1->identifier) == 0) {
         //printf("%s\t", symbol->identifier);
         //printf("%s\n",type_name(symbol->type));
         strcpy(parameters_global[tmp_aux-1].name_var,symbol1->identifier);
         strcpy(parameters_global[tmp_aux-1].type_var,llvm_type(type_name(symbol1->type)));
         parameters_global[tmp_aux-1].tmp = tmp_aux;
         printf("  %%p%d = alloca %s\n",tmp_aux,parameters_global[tmp_aux-1].type_var);
         printf("  store %s* @%s, %s* %%p%d\n",parameters_global[tmp_aux-1].type_var,parameters_global[tmp_aux-1].name_var,parameters_global[tmp_aux-1].type_var,tmp_aux);
         tmp_aux = tmp_aux + 1;


     }
 }
 */


 struct node *funcBody = getchild(function, 1);
 //printf("----> %s \n",category_name[funcBody->category]);

 if (getchild(funcBody, 0) == NULL) {
    return;
}
// printf("6\n");
struct node_list *child_funcBody = funcBody->children;
 //printf("----> %s \n",category_name[function->category]);
while ((child_funcBody = child_funcBody->next) != NULL) {
    //printf("Category 0: %s\n", category_name[child_funcBody->node->category]);
    struct node_list *child_expr = child_funcBody->node->children;

    while ((child_expr = child_expr->next) != NULL) {
         //printf("----> %s \n",category_name[child_expr->node->category]);
        //check_expression(child_expr->node, scope);
        //tmp = codegen_expression(child_expr->node);
        codegen_expression(child_expr->node);
    }
}

 if(strcmp(getchild(funcHeader, 0)->token,"main") == 0){
     //printf("  ret i32 0\n");
     printf("  ret void\n");
 }
 else{
     if(strcmp(llvm_type(type_name(symbol->type)),"none")!= 0){
         //printf("  ret %s %%%d\n", llvm_type(type_name(symbol->type)),tmp);

         //NOVO NAO EXISTIA ANTES COLOCO SEMPRE UM RETURN E CASO HA JA RETURN EU DEPOIS COLOCO RETURN CERTO ANTES DESTE RETURN- ISTO É SO PARA GARANTIR QUE HA SEMPRE RETURN NAS FUNÇÕES EM QUE É NECESSARIO
         if(strcmp(llvm_type(type_name(symbol->type)),"i32")== 0){
             printf("  ret %s 0\n", llvm_type(type_name(symbol->type)));//--------------------------------------->tirei de comentario
         }
         else if(strcmp(llvm_type(type_name(symbol->type)),"double")== 0){
             printf("  ret %s 0.0\n", llvm_type(type_name(symbol->type)));//--------------------------------------->tirei de comentario
         }
         else if(strcmp(llvm_type(type_name(symbol->type)),"i8")== 0){
             printf("  ret %s* null\n", llvm_type(type_name(symbol->type)));//--------------------------------------->tirei de comentario
         }
         else if(strcmp(llvm_type(type_name(symbol->type)),"i1")== 0){
             printf("  ret %s 0\n", llvm_type(type_name(symbol->type)));//--------------------------------------->tirei de comentario
         }

     }
     else{
         //printf("ENTROU\n");
         printf("  ret void\n");
     }
 }

 printf("}\n\n");
 clear_parameters_global();
 tam_variaveis_locais = 0;
 clear_variaveis_locais();

}



// geração de codigo inicio
void codegen_program(struct node *program) {
 printf("@.fmtstr_true = private unnamed_addr constant [6 x i8] c\"true\\0A\\00\"\n");
 printf("@.fmtstr_false = private unnamed_addr constant [7 x i8] c\"false\\0A\\00\"\n");
 printf("declare void @free(i8*)");
 printf("declare i32 @printf(i8*, ...)\n");
 printf("declare i8* @malloc(i64)\n");
 //printf("declare i32 @_write_int(i32)\n");
 //printf("declare i32 @_write_string(i8*)\n");
 //printf("declare i32 @_write_bool(i1)\n");
 //printf("declare i32 @_write_float(double)\n");
 printf("declare i32 @atoi(i8*)\n");
 //printf("declare double @atof(i8*)\n");

 struct node_list *child = program->children;

 while((child = child->next) != NULL)
     if(child->node->category == VarDecl){
         //
         codegen_var(child->node);
     }
     else{
         //codegen_function(getchild(child->node,0));
         codegen_function(child->node);
     }



 struct symbol_list *entry = search_symbol(symbol_table, "main");
 if(entry != NULL )
     printf("define i32 @main(i32 %%argc, i8** %%argv) {\n"
            "  call void @_main(i32 %%argc, i8** %%argv)\n"
            "  ret i32 0\n"
            "}\n");

}