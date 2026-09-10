#pragma once

#include <string>
#include <vector>

enum class TokenType {
    Print,
    If,
    Else,
    While,
    For,
    In,
    Fn,
    Return,
    True,
    False,

    Identifier,
    Number,
    String,

    Equals,
    Plus,
    Minus,
    Star,
    Slash,

    EqualEqual,
    NotEqual,
    Greater,
    GreaterEqual,
    Less,
    LessEqual,

    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    LeftBracket,
    RightBracket,
    Comma,

    End
};

struct Token {
    TokenType type;
    std::string value;
};

class Lexer {
public:
    explicit Lexer(const std::string& source);

    std::vector<Token> tokenize();

private:
    std::string source;
    size_t position = 0;

    char current() const;
    char peek() const;
    void advance();
    void skip_whitespace();

    Token read_string();
    Token read_identifier();
    Token read_number();
};