#ifndef _CODEGEN_H
#define _CODEGEN_H


#define names { "Program", "Function", "Parameters", "Parameter", "Arguments", "Identifier", "Natural", "Decimal", "Call", "If", "Declarations", "VarDeclaration", "VarSpec", "Type", "FuncDeclaration", "FuncBody", "VarsAndStatements", "Statement", "For", "Return", "Print", "ParseArgs", "FuncInvocation", "Expr", "FuncDecl", "FuncHeader", "FuncParams", "VarDecl", "Block", "Assign", "StrLit","ParamDecl", "Or", "And", "Lt", "Gt", "Eq", "Ne", "Le", "Ge", "Plus", "Sub", "Mul", "Mod", "Not", "Div", "String", "Int", "Float32", "Bool", "Add", "Dummy", "Minus", "Dummy1"}


void codegen_program(struct node *program);
int codegen_expression(struct node *expression);

#endif