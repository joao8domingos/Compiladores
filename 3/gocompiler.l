/* João Domingos - 2022215704 
  Miguel Domingos - 2022216083 */


%{
    #include "y.tab.h"
    #include <stdio.h>
    #include <string.h>
    #include <ctype.h>
    #include "semantics.h"

    extern int yychar; // Current token

    int lex_line = 1, lex_column = 1;
    int syn_line = -1, syn_column = -1;
    int line1 = 1, col1 = 1;

    extern struct scope_list *lista_de_scopes;

    int eof = 0;

    // macro YY_USER_ACTION, which updates 'lex_column', is executed before every action
    //#define YY_USER_ACTION { syn_line = lex_line; syn_column = lex_column; lex_column += yyleng; }
    #define YY_USER_ACTION do { \
        syn_line = line1; syn_column = col1; \
        yylloc.first_line = syn_line; yylloc.first_column = syn_column; \
        for (int i = 0; i < yyleng; i++) { \
            if (yytext[i] == '\n') { \
                line1++; col1 = 1; \
            } else \
                { col1++; } \
        } \
    } while(0);

    #define PASS_TOKEN() yylval.lexeme = strdup(yytext);

    /*
    enum category {
    Program, Function, Parameters, Parameter, Arguments, Identifier, Natural, Decimal, Call, If,
    Declarations, VarDeclaration, VarSpec, Type, FuncDeclaration, FuncBody, VarsAndStatements, Statement, For, Return, Print, ParseArgs, FuncInvocation, Expr, FuncDecl, FuncHeader, FuncParams, VarDecl, Block, Assign, StrLit,ParamDecl,
    Or, And, Lt, Gt, Eq, Ne, Le, Ge, Plus, Sub, Mul, Mod, Not, Div, String, 
    Int, Float32, Bool, Add, Dummy, Minus, 
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
    */

    int yyparse(void);
    void yyerror(const char *s);
    void show(struct node *node, int depth);
    void show_s(struct node *node, int depth);
    void freenode(struct node *root);

    int errors = 0;

    extern struct node *program;

    int line = 1;
    int col = 1;
    char aux[2048];
    char aux1[2048];
    int ant = 0;
    int print = 0;
    int print_tree = 0;
    int meta3 = 0;
    int erros = 0;
    int pos = 1; 
    int pos_c = 0;
    int line_ant = 0;
    int nn = 0;

    /* Função para transformar a string em letras maiúsculas */
    void to_uppercase(char *str) {
        for (int i = 0; str[i]; i++) {
            str[i] = toupper(str[i]);
        }
    }

    /* Função para imprimir tokens caso a opção -l esteja ativada */
    void print_token(const char *token, const char *value) {
        if (print) {
            if (value)
                printf("%s(%s)\n", token, value);  // Tokens com valor (IDENTIFIER, STRLIT, etc.)
            else
                printf("%s\n", token);             // Tokens simples (SEMICOLON, FUNC, etc.)
        }
    }

%}

%option noyywrap
%option noinput
%option nounput
%option yylineno

digit    [0-9]
letter   [a-zA-Z]
hexa     [0-9a-fA-F]
operator [=*/+-]
octal [0-7]

%X COMMENT
%X COMMENTLINE
%X STR
%%

"/*"            { BEGIN(COMMENT); pos_c = col; line_ant = line; col += yyleng; nn = 0; } 
<COMMENT>"*/"   { BEGIN(INITIAL); col += yyleng; if (nn > 0 && ant == 1){ print_token("SEMICOLON", NULL); ant = 0; return SEMICOLON; } }
<COMMENT>\n     { line += 1; col = 1; nn++; lex_line++; lex_column = 1;} 
<COMMENT><<EOF>> { printf("Line %d, column %d: unterminated comment\n", line_ant, pos_c); ant = 0; BEGIN(INITIAL); return 1;  }
<COMMENT>.      { col += yyleng; }

"//"            { BEGIN(COMMENTLINE); col += yyleng;}  
<COMMENTLINE>\n     {lex_line++; lex_column = 1; line += 1; col = 1; BEGIN(INITIAL);  if (ant == 1){print_token("SEMICOLON", NULL);ant = 0; return SEMICOLON;}}  
<COMMENTLINE>.      { col += yyleng; }

\"          { BEGIN(STR); pos = col; col += yyleng; aux[0] = '\0'; erros = 0; strcat(aux, "\""); } 
<STR>\n            { printf("Line %d, column %d: unterminated string literal\n", line, pos); BEGIN(INITIAL); line++; col = 1; erros++; pos = 1; ant = 0; lex_line++; lex_column = 1;}  /* String não fechada corretamente */ 
<STR>\"      { if (erros == 0) { strcat(aux, "\""); print_token("STRLIT", aux); ant = 1; }  col += yyleng; BEGIN(INITIAL); yylval.lexeme = strdup(aux); return STRLIT; }  /* String fechada corretamente */
<STR>\\(\n)         {printf("Line %d, column %d: invalid escape sequence (\\)\n", line, col); printf("Line %d, column %d: unterminated string literal\n", line, pos); col = 1; line++; erros++;BEGIN(INITIAL);}
<STR>\\[fnrt\"\\]    { strcat(aux, yytext); col += yyleng; }  /* Sequências de escape válidas */
<STR>\\[^fnrt\"\\]   { printf("Line %d, column %d: invalid escape sequence (%s)\n", line, col, yytext); col += yyleng; erros++; }  /* Sequências de escape inválidas */ // \n ------------------------------------>NOVA
<STR>[^\\\n\"]+      { strcat(aux, yytext); col += yyleng; }  /* Qualquer outro caractere dentro da string */
<STR>.               { printf("Line %d, column %d: invalid character in string", line, col); col += yyleng; BEGIN(INITIAL); }  /* Caractere inválido */
<STR><<EOF>>         { printf("Line %d, column %d: unterminated string literal", line, col);  return 1; }  /* Tratamento de string não finalizada no EOF */


\n                           { line++; 
                                col = 1; 

                                lex_line++;
                                lex_column = 1;
                                if (ant == 1){
                                    
                                    print_token("SEMICOLON", NULL);
                                    ant = 0; 
                                    return SEMICOLON;
                                }
                              }

break|case|chan|const|continue|default|defer|fallthrough|go|goto|import|interface|map|range|select|struct|switch|type|"++"|"--"   {print_token("RESERVED", yytext); col += yyleng; ant = 0; return RESERVED;}


";"     { print_token("SEMICOLON", NULL); col += yyleng; ant = 0; return SEMICOLON; }
","     { print_token("COMMA", NULL); col += yyleng; ant = 0; return COMMA; }
"_"     { print_token("BLANKID", NULL); col += yyleng; ant = 0; return BLANKID; }
"="     { print_token("ASSIGN", NULL); col += yyleng; ant = 0; return ASSIGN; }
"*"     { print_token("STAR", NULL); col += yyleng; ant = 0; return STAR; }
"/"     { print_token("DIV", NULL); col += yyleng; ant = 0; return DIV; }
"-"     { print_token("MINUS", NULL); col += yyleng; ant = 0; return MINUS; }
"+"     { print_token("PLUS", NULL); col += yyleng; ant = 0; return PLUS; }
"=="    { print_token("EQ", NULL); col += yyleng; ant = 0; return EQ; }
">="    { print_token("GE", NULL); col += yyleng; ant = 0; return GE; }
">"     { print_token("GT", NULL); col += yyleng; ant = 0; return GT; }
"{"     { print_token("LBRACE", NULL); col += yyleng; ant = 0; return LBRACE; }
"<="    { print_token("LE", NULL); col += yyleng; ant = 0; return LE; }
"("     { print_token("LPAR", NULL); col += yyleng; ant = 0; return LPAR; }
"["     { print_token("LSQ", NULL); col += yyleng; ant = 0; return LSQ; }
"<"     { print_token("LT", NULL); col += yyleng; ant = 0; return LT; }
"%"     { print_token("MOD", NULL); col += yyleng; ant = 0; return MOD; }
"!="    { print_token("NE", NULL); col += yyleng; ant = 0; return NE; }
"!"     { print_token("NOT", NULL); col += yyleng; ant = 0; return NOT; }
"&&"    { print_token("AND", NULL); col += yyleng; ant = 0; return AND; }
"||"    { print_token("OR", NULL); col += yyleng; ant = 0; return OR; }
"}"     { print_token("RBRACE", NULL); col += yyleng; ant = 1; return RBRACE; }
")"     { print_token("RPAR", NULL); col += yyleng; ant = 1; return RPAR; }
"]"     { print_token("RSQ", NULL); col += yyleng; ant = 1; return RSQ; }



"package"         { print_token("PACKAGE", NULL); col += yyleng; ant = 0; return PACKAGE; }
"return"          { print_token("RETURN", NULL); col += yyleng; ant = 1; return RETURN; }
"else"            { print_token("ELSE", NULL); col += yyleng; ant = 0; return ELSE; }
"for"             { print_token("FOR", NULL); col += yyleng; ant = 0; return FOR; }
"if"              { print_token("IF", NULL); col += yyleng; ant = 0; return IF; }
"var"             { print_token("VAR", NULL); col += yyleng; ant = 0; return VAR; }
"int"             { print_token("INT", NULL); col += yyleng; ant = 0; return INT; }
"float32"         { print_token("FLOAT32", NULL); col += yyleng; ant = 0; return FLOAT32; }
"bool"            { print_token("BOOL", NULL); col += yyleng; ant = 0; return BOOL; }
"string"          { print_token("STRING", NULL); col += yyleng; ant = 0; return STRING; }
"fmt.Println"     { print_token("PRINT", NULL); col += yyleng; ant = 0; return PRINT; }
"strconv.Atoi"    { print_token("PARSEINT", NULL); col += yyleng; ant = 0; return PARSEINT; }
"func"            { print_token("FUNC", NULL); col += yyleng; ant = 0; return FUNC; }
"os.Args"         { print_token("CMDARGS", NULL); col += yyleng; ant = 0; return CMDARGS; }

"0"{octal}*                    { print_token("NATURAL", yytext); col += yyleng; ant = 1; PASS_TOKEN(); return NATURAL; }
"0"[0-9]+                    {printf("Line %d, column %d: invalid octal constant (%s)\n", line, col, yytext); col += yyleng; }
"0x"{hexa}+      { print_token("NATURAL", yytext); col += yyleng; ant = 1; PASS_TOKEN(); return NATURAL; }
"0X"{hexa}+     { print_token("NATURAL", yytext); col += yyleng; ant = 1; PASS_TOKEN(); return NATURAL; }
[1-9]([0-9])*                      { print_token("NATURAL", yytext); col += yyleng; ant = 1; PASS_TOKEN(); return NATURAL; }


{digit}+\.                                   { print_token("DECIMAL", yytext); col += yyleng; ant = 1; PASS_TOKEN(); return DECIMAL; }
{digit}*\.{digit}+((e|E)(\+|\-)?{digit}+)?  { print_token("DECIMAL", yytext); col += yyleng; ant = 1; PASS_TOKEN(); return DECIMAL; }
{digit}+\.(e|E)(\+|\-)?{digit}+             { print_token("DECIMAL", yytext); col += yyleng; ant = 1; PASS_TOKEN(); return DECIMAL; }
{digit}+((e|E)(\+|\-)?{digit}+)               { print_token("DECIMAL", yytext); col += yyleng; ant = 1; PASS_TOKEN(); return DECIMAL; }




[a-zA-Z]([a-zA-Z0-9_])* { print_token("IDENTIFIER", yytext); col += yyleng; ant = 1; PASS_TOKEN(); return IDENTIFIER; }
"_"([a-zA-Z0-9_])+ { print_token("IDENTIFIER", yytext); col += yyleng; ant = 1; PASS_TOKEN(); return IDENTIFIER; }


<<EOF>>                       {if (eof == 0) {eof++; YY_USER_ACTION; if (ant > 0){ print_token("SEMICOLON", NULL); return SEMICOLON; }}  return 0; }

" "|\t                           { col+= yyleng; } // ------------------------------------> NOVO
.                             { printf("Line %d, column %d: illegal character (%s)\n", line, col, yytext);col++;   }

%%

extern int yylex();

/*
void yyerror(const char *s) {
    errors++;
    if (strcmp(s, "syntax error") == 0) {
        printf("Line %d, column %d: %s: %s\n", syn_line, syn_column, s, yytext);
    } else {
        printf("Line %d, column %d: %s: %s\n", syn_line, syn_column, s, aux);
    }
    //printf("Line %d, column %d: %s: %s\n", syn_line, syn_column, s, yytext);
}
*/

/* Modifique yyerror para aceitar uma mensagem de erro específica */
void yyerror(const char *s) {
    errors++;
    if (yychar != STRLIT) {
        printf("Line %d, column %d: %s: %s\n", syn_line, syn_column, s, yytext);
    } else {
        printf("Line %d, column %lu: %s: %s\n", line1, col1 - strlen(aux), s, aux);
    }
    //printf("Line %d, column %d: %s: %s\n", syn_line, syn_column, s, yytext);
    //printf("Line %d, column %d: %s: %s\n", syn_line, syn_column, s, yytext);
}




int main(int argc, char **argv) {
    // Itera sobre todos os argumentos passados
    for (int i = 1; i < argc; i++) {
        // Verifica se o argumento atual é "-l"
        if (strcmp(argv[i], "-l") == 0) {
            print = 1;  // Ativar impressão de tokens
            print_tree = 0;  // Desativar impressão da árvore
            break;      // Interrompe o loop, já encontramos o "-l"
        }
        if (strcmp(argv[i],"-t") == 0){
            print_tree = 1;
        }
        if (strcmp(argv[i],"-s") == 0){
            meta3 = 1;
        }
    }

    // Chama a função yylex para iniciar o analisador léxico
    //yylex();
    //printf("erros: %d\n", errors);
    yyparse();
    //printf("erros 1: %d\n", errors);
    if (errors > 0) {
        return 1;
    }

    if (print_tree == 0) {
        errors += check_program(program); 

        if (meta3 == 1){
            show_symbol_table();
            struct scope_list *current = lista_de_scopes; // Começa na cabeça da lista
                // Percorre até o final da lista
            while (current != NULL) {
                    // Acessar o nó do escopo atual
                show_scope(current->scope,current->token);

                    // Move para o próximo nó
                current = current->next;
            }

            show_s(program, 0);
        }
    }
    else {
        show(program, 0);
    }
    
    /*
    if (print_tree == 1 && errors == 0){ //&& errors == 0
        show_symbol_table();
        if (meta3 == 1){
            
            struct scope_list *current = lista_de_scopes; // Começa na cabeça da lista
            // Percorre até o final da lista
            while (current != NULL) {
                // Acessar o nó do escopo atual
                show_scope(current->scope,current->token);

                // Move para o próximo nó
                current = current->next;
            }

            show_s(program, 0);
        }
        else{
            show(program, 0);
        }
        //show(program, 0);
    }
    */


    

    return 0;
}

/*
int yywrap() {  // Called on EOF, return 1 to terminate 
    return 1;
}
*/