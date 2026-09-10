#include "parser.hpp"

#include <stdexcept>

Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens) {}

const Token& Parser::current() const {
    return tokens[position];
}

void Parser::advance() {
    if (position < tokens.size() - 1)
        position++;
}

void Parser::expect(TokenType type) {
    if (current().type != type) {
        throw std::runtime_error(
            "Unexpected token: " + current().value
        );
    }

    advance();
}

Token Parser::parse_value() {
    Token value = current();

    if (value.type != TokenType::String &&
        value.type != TokenType::Number &&
        value.type != TokenType::Identifier &&
        value.type != TokenType::True &&
        value.type != TokenType::False) {

        throw std::runtime_error(
            "Expected a value"
        );
    }

    advance();

    return value;
}

VariableStatement Parser::parse_variable() {
    std::string name = current().value;

    expect(TokenType::Identifier);
    expect(TokenType::Equals);

    // =========================
    // ARRAY
    // =========================

    if (current().type == TokenType::LeftBracket) {

        advance();

        std::vector<Token> elements;

        if (current().type != TokenType::RightBracket) {

            while (true) {

                if (current().type != TokenType::Number) {
                    throw std::runtime_error(
                        "Arrays currently only support numbers"
                    );
                }

                elements.push_back(current());
                advance();

                if (current().type != TokenType::Comma)
                    break;

                advance();
            }
        }

        expect(TokenType::RightBracket);

        return {
            name,
            {},
            false,
            {},
            false,
            {},
            true,
            {elements},
            false,
            {}
        };
    }

    // =========================
    // FUNCTION CALL
    // =========================

    if (current().type == TokenType::Identifier &&
        position + 1 < tokens.size() &&
        tokens[position + 1].type == TokenType::LeftParen) {

        std::string function_name =
            current().value;

        advance();

        expect(TokenType::LeftParen);

        std::vector<Token> arguments;

        if (current().type != TokenType::RightParen) {

            while (true) {

                arguments.push_back(
                    parse_value()
                );

                if (current().type != TokenType::Comma)
                    break;

                advance();
            }
        }

        expect(TokenType::RightParen);

        return {
            name,
            {},
            false,
            {},
            true,
            {function_name, arguments},
            false,
            {},
            false,
            {}
        };
    }

    // =========================
    // ARRAY ACCESS
    // =========================

    if (current().type == TokenType::Identifier &&
        position + 1 < tokens.size() &&
        tokens[position + 1].type == TokenType::LeftBracket) {

        std::string array_name =
            current().value;

        advance();

        expect(TokenType::LeftBracket);

        Token index =
            parse_value();

        expect(TokenType::RightBracket);

        return {
            name,
            {},
            false,
            {},
            false,
            {},
            false,
            {},
            true,
            {array_name, index}
        };
    }

    // =========================
    // NORMAL VALUE
    // =========================

    Token left =
        parse_value();

    // =========================
    // MATH
    // =========================

    if (current().type == TokenType::Plus ||
        current().type == TokenType::Minus ||
        current().type == TokenType::Star ||
        current().type == TokenType::Slash) {

        Token operation =
            current();

        advance();

        Token right =
            parse_value();

        return {
            name,
            {},
            true,
            {left, operation, right},
            false,
            {},
            false,
            {},
            false,
            {}
        };
    }

    // =========================
    // NORMAL VARIABLE
    // =========================

    return {
        name,
        left,
        false,
        {},
        false,
        {},
        false,
        {},
        false,
        {}
    };
}

PrintStatement Parser::parse_print() {
    expect(TokenType::Print);

    expect(TokenType::LeftParen);

    Token value =
        parse_value();

    bool is_array_access = false;

    ArrayAccess array_access{};

    if (value.type == TokenType::Identifier &&
        current().type == TokenType::LeftBracket) {

        array_access.name =
            value.value;

        advance();

        array_access.index =
            parse_value();

        expect(TokenType::RightBracket);

        is_array_access = true;
    }

    expect(TokenType::RightParen);

    return {
        value,
        is_array_access,
        array_access
    };
}

ReturnStatement Parser::parse_return() {
    expect(TokenType::Return);

    Token left =
        parse_value();

    // return x
    if (current().type != TokenType::Plus &&
        current().type != TokenType::Minus &&
        current().type != TokenType::Star &&
        current().type != TokenType::Slash) {

        return {
            {left, {}, {}}
        };
    }

    // return x + y
    Token operation =
        current();

    advance();

    Token right =
        parse_value();

    return {
        {left, operation, right}
    };
}

FunctionStatement Parser::parse_function() {
    expect(TokenType::Fn);

    std::string name =
        current().value;

    expect(TokenType::Identifier);

    expect(TokenType::LeftParen);

    std::vector<std::string> parameters;

    if (current().type != TokenType::RightParen) {

        while (true) {

            parameters.push_back(
                current().value
            );

            expect(TokenType::Identifier);

            if (current().type != TokenType::Comma)
                break;

            advance();
        }
    }

    expect(TokenType::RightParen);

    expect(TokenType::LeftBrace);

    auto body =
        parse_block();

    expect(TokenType::RightBrace);

    return {
        name,
        parameters,
        body
    };
}

IfStatement Parser::parse_if() {
    expect(TokenType::If);

    Token left =
        parse_value();

    /*
        Bare boolean:

        if alive {
        }

        if true {
        }

        if false {
        }

        IMPORTANT:
        An identifier is only a bare boolean
        if the next token is "{".

        Therefore:

        if finished == false

        correctly goes to comparison.
    */

    if (current().type == TokenType::LeftBrace) {

        if (left.type != TokenType::True &&
            left.type != TokenType::False &&
            left.type != TokenType::Identifier) {

            throw std::runtime_error(
                "Expected a boolean condition"
            );
        }

        expect(TokenType::LeftBrace);

        auto body =
            parse_block();

        expect(TokenType::RightBrace);

        std::vector<Statement> else_body;

        bool has_else = false;

        if (current().type == TokenType::Else) {

            has_else = true;

            advance();

            expect(TokenType::LeftBrace);

            else_body =
                parse_block();

            expect(TokenType::RightBrace);
        }

        return {
            left,
            {TokenType::End, ""},
            {},
            body,
            else_body,
            has_else
        };
    }

    // =========================
    // COMPARISON
    // =========================

    Token operation =
        current();

    if (operation.type != TokenType::EqualEqual &&
        operation.type != TokenType::NotEqual &&
        operation.type != TokenType::Greater &&
        operation.type != TokenType::GreaterEqual &&
        operation.type != TokenType::Less &&
        operation.type != TokenType::LessEqual) {

        throw std::runtime_error(
            "Expected comparison operator"
        );
    }

    advance();

    Token right =
        parse_value();

    expect(TokenType::LeftBrace);

    auto body =
        parse_block();

    expect(TokenType::RightBrace);

    std::vector<Statement> else_body;

    bool has_else = false;

    if (current().type == TokenType::Else) {

        has_else = true;

        advance();

        expect(TokenType::LeftBrace);

        else_body =
            parse_block();

        expect(TokenType::RightBrace);
    }

    return {
        left,
        operation,
        right,
        body,
        else_body,
        has_else
    };
}

WhileStatement Parser::parse_while() {
    expect(TokenType::While);

    Token left =
        parse_value();

    Token operation =
        current();

    if (operation.type != TokenType::EqualEqual &&
        operation.type != TokenType::NotEqual &&
        operation.type != TokenType::Greater &&
        operation.type != TokenType::GreaterEqual &&
        operation.type != TokenType::Less &&
        operation.type != TokenType::LessEqual) {

        throw std::runtime_error(
            "Expected comparison operator"
        );
    }

    advance();

    Token right =
        parse_value();

    expect(TokenType::LeftBrace);

    auto body =
        parse_block();

    expect(TokenType::RightBrace);

    return {
        left,
        operation,
        right,
        body
    };
}

ForStatement Parser::parse_for() {
    expect(TokenType::For);

    std::string variable =
        current().value;

    expect(TokenType::Identifier);

    expect(TokenType::In);

    std::string array_name =
        current().value;

    expect(TokenType::Identifier);

    expect(TokenType::LeftBrace);

    auto body =
        parse_block();

    expect(TokenType::RightBrace);

    return {
        variable,
        array_name,
        body
    };
}

Statement Parser::parse_statement() {

    if (current().type == TokenType::Identifier) {

        Statement statement{};

        statement.type =
            Statement::Type::Variable;

        statement.variable =
            parse_variable();

        return statement;
    }

    if (current().type == TokenType::Print) {

        Statement statement{};

        statement.type =
            Statement::Type::Print;

        statement.print =
            parse_print();

        return statement;
    }

    if (current().type == TokenType::If) {

        Statement statement{};

        statement.type =
            Statement::Type::If;

        statement.if_statement =
            parse_if();

        return statement;
    }

    if (current().type == TokenType::While) {

        Statement statement{};

        statement.type =
            Statement::Type::While;

        statement.while_statement =
            parse_while();

        return statement;
    }

    if (current().type == TokenType::For) {

        Statement statement{};

        statement.type =
            Statement::Type::For;

        statement.for_statement =
            parse_for();

        return statement;
    }

    if (current().type == TokenType::Fn) {

        Statement statement{};

        statement.type =
            Statement::Type::Function;

        statement.function =
            parse_function();

        return statement;
    }

    if (current().type == TokenType::Return) {

        Statement statement{};

        statement.type =
            Statement::Type::Return;

        statement.return_statement =
            parse_return();

        return statement;
    }

    throw std::runtime_error(
        "Unexpected token: " +
        current().value
    );
}

std::vector<Statement> Parser::parse_block() {
    std::vector<Statement> statements;

    while (
        current().type != TokenType::RightBrace &&
        current().type != TokenType::End
    ) {
        statements.push_back(
            parse_statement()
        );
    }

    return statements;
}

std::vector<Statement> Parser::parse_program() {
    std::vector<Statement> statements;

    while (current().type != TokenType::End) {

        statements.push_back(
            parse_statement()
        );
    }

    return statements;
}