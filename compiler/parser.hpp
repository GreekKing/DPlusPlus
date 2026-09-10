#pragma once

#include "lexer.hpp"

#include <string>
#include <vector>

struct Expression {
    Token left;
    Token operation;
    Token right;
};

struct FunctionCall {
    std::string name;
    std::vector<Token> arguments;
};

struct ArrayLiteral {
    std::vector<Token> elements;
};

struct ArrayAccess {
    std::string name;
    Token index;
};

struct VariableStatement {
    std::string name;
    Token value;

    bool is_expression;
    Expression expression;

    bool is_function_call;
    FunctionCall function_call;

    bool is_array;
    ArrayLiteral array;

    bool is_array_access;
    ArrayAccess array_access;
};

struct PrintStatement {
    Token value;

    bool is_array_access;
    ArrayAccess array_access;
};

struct ReturnStatement {
    Expression expression;
};

struct FunctionStatement {
    std::string name;
    std::vector<std::string> parameters;
    std::vector<struct Statement> body;
};

struct IfStatement {
    Token left;
    Token operation;
    Token right;

    std::vector<struct Statement> body;
    std::vector<struct Statement> else_body;

    bool has_else;
};

struct WhileStatement {
    Token left;
    Token operation;
    Token right;

    std::vector<struct Statement> body;
};

struct ForStatement {
    std::string variable;
    std::string array_name;

    std::vector<struct Statement> body;
};

struct Statement {
    enum class Type {
        Variable,
        Print,
        If,
        While,
        For,
        Function,
        Return
    };

    Type type;

    VariableStatement variable;
    PrintStatement print;
    IfStatement if_statement;
    WhileStatement while_statement;
    ForStatement for_statement;
    FunctionStatement function;
    ReturnStatement return_statement;
};

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    std::vector<Statement> parse_program();

private:
    const std::vector<Token>& tokens;
    size_t position = 0;

    const Token& current() const;
    void advance();
    void expect(TokenType type);

    Token parse_value();

    VariableStatement parse_variable();
    PrintStatement parse_print();
    IfStatement parse_if();
    WhileStatement parse_while();
    ForStatement parse_for();
    FunctionStatement parse_function();
    ReturnStatement parse_return();

    std::vector<Statement> parse_block();
    Statement parse_statement();
};