#include "lexer.hpp"

#include <cctype>
#include <stdexcept>

Lexer::Lexer(const std::string& source)
    : source(source) {}

char Lexer::current() const {
    if (position >= source.size())
        return '\0';

    return source[position];
}

char Lexer::peek() const {
    if (position + 1 >= source.size())
        return '\0';

    return source[position + 1];
}

void Lexer::advance() {
    if (position < source.size())
        position++;
}

void Lexer::skip_whitespace() {
    while (
        std::isspace(
            static_cast<unsigned char>(current())
        )
    ) {
        advance();
    }
}

Token Lexer::read_string() {
    advance();

    std::string value;

    while (current() != '"' && current() != '\0') {
        value += current();
        advance();
    }

    if (current() != '"') {
        throw std::runtime_error(
            "Unterminated string"
        );
    }

    advance();

    return {
        TokenType::String,
        value
    };
}

Token Lexer::read_identifier() {
    std::string value;

    while (
        std::isalnum(
            static_cast<unsigned char>(current())
        ) ||
        current() == '_'
    ) {
        value += current();
        advance();
    }

    // Keywords
    if (value == "print")
        return {TokenType::Print, value};

    if (value == "if")
        return {TokenType::If, value};

    if (value == "else")
        return {TokenType::Else, value};

    if (value == "while")
        return {TokenType::While, value};

    if (value == "for")
        return {TokenType::For, value};

    if (value == "in")
        return {TokenType::In, value};

    if (value == "fn")
        return {TokenType::Fn, value};

    if (value == "return")
        return {TokenType::Return, value};

    // Booleans
    if (value == "true")
        return {TokenType::True, value};

    if (value == "false")
        return {TokenType::False, value};

    return {
        TokenType::Identifier,
        value
    };
}

Token Lexer::read_number() {
    std::string value;

    while (
        std::isdigit(
            static_cast<unsigned char>(current())
        )
    ) {
        value += current();
        advance();
    }

    return {
        TokenType::Number,
        value
    };
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (current() != '\0') {

        skip_whitespace();

        if (current() == '\0')
            break;

        // String
        if (current() == '"') {

            tokens.push_back(
                read_string()
            );
        }

        // Identifier / keyword
        else if (
            std::isalpha(
                static_cast<unsigned char>(current())
            ) ||
            current() == '_'
        ) {

            tokens.push_back(
                read_identifier()
            );
        }

        // Number
        else if (
            std::isdigit(
                static_cast<unsigned char>(current())
            )
        ) {

            tokens.push_back(
                read_number()
            );
        }

        // ==
        else if (
            current() == '=' &&
            peek() == '='
        ) {

            tokens.push_back(
                {TokenType::EqualEqual, "=="}
            );

            advance();
            advance();
        }

        // !=
        else if (
            current() == '!' &&
            peek() == '='
        ) {

            tokens.push_back(
                {TokenType::NotEqual, "!="}
            );

            advance();
            advance();
        }

        // >=
        else if (
            current() == '>' &&
            peek() == '='
        ) {

            tokens.push_back(
                {TokenType::GreaterEqual, ">="}
            );

            advance();
            advance();
        }

        // <=
        else if (
            current() == '<' &&
            peek() == '='
        ) {

            tokens.push_back(
                {TokenType::LessEqual, "<="}
            );

            advance();
            advance();
        }

        // =
        else if (current() == '=') {

            tokens.push_back(
                {TokenType::Equals, "="}
            );

            advance();
        }

        // >
        else if (current() == '>') {

            tokens.push_back(
                {TokenType::Greater, ">"}
            );

            advance();
        }

        // <
        else if (current() == '<') {

            tokens.push_back(
                {TokenType::Less, "<"}
            );

            advance();
        }

        // +
        else if (current() == '+') {

            tokens.push_back(
                {TokenType::Plus, "+"}
            );

            advance();
        }

        // -
        else if (current() == '-') {

            tokens.push_back(
                {TokenType::Minus, "-"}
            );

            advance();
        }

        // *
        else if (current() == '*') {

            tokens.push_back(
                {TokenType::Star, "*"}
            );

            advance();
        }

        // /
        else if (current() == '/') {

            tokens.push_back(
                {TokenType::Slash, "/"}
            );

            advance();
        }

        // (
        else if (current() == '(') {

            tokens.push_back(
                {TokenType::LeftParen, "("}
            );

            advance();
        }

        // )
        else if (current() == ')') {

            tokens.push_back(
                {TokenType::RightParen, ")"}
            );

            advance();
        }

        // {
        else if (current() == '{') {

            tokens.push_back(
                {TokenType::LeftBrace, "{"}
            );

            advance();
        }

        // }
        else if (current() == '}') {

            tokens.push_back(
                {TokenType::RightBrace, "}"}
            );

            advance();
        }

        // [
        else if (current() == '[') {

            tokens.push_back(
                {TokenType::LeftBracket, "["}
            );

            advance();
        }

        // ]
        else if (current() == ']') {

            tokens.push_back(
                {TokenType::RightBracket, "]"}
            );

            advance();
        }

        // ,
        else if (current() == ',') {

            tokens.push_back(
                {TokenType::Comma, ","}
            );

            advance();
        }

        else {

            throw std::runtime_error(
                std::string(
                    "Unexpected character: "
                ) + current()
            );
        }
    }

    tokens.push_back(
        {TokenType::End, ""}
    );

    return tokens;
}