#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "lexer.hpp"
#include "parser.hpp"
#include "environment.hpp"

struct ReturnValue {
    bool returned = false;
    int value = 0;
};

bool get_boolean(
    const Token& token,
    const Environment& environment
) {
    if (token.type == TokenType::True)
        return true;

    if (token.type == TokenType::False)
        return false;

    if (token.type == TokenType::Identifier) {
        std::string value =
            environment.get(token.value);

        if (value == "true")
            return true;

        if (value == "false")
            return false;
    }

    throw std::runtime_error(
        "Expected a boolean"
    );
}

int get_number(
    const Token& token,
    const Environment& environment
) {
    if (token.type == TokenType::Number)
        return std::stoi(token.value);

    if (token.type == TokenType::Identifier)
        return std::stoi(
            environment.get(token.value)
        );

    throw std::runtime_error(
        "Expected a number"
    );
}

bool compare(
    const Token& left_token,
    TokenType operation,
    const Token& right_token,
    const Environment& environment
) {
    bool left_is_boolean =
        left_token.type == TokenType::True ||
        left_token.type == TokenType::False;

    bool right_is_boolean =
        right_token.type == TokenType::True ||
        right_token.type == TokenType::False;

    /*
        Boolean comparison

        true == true
        true != false
        false == false

        Also supports variables:

        alive == true
        finished != true
    */
    if (left_is_boolean || right_is_boolean) {

        bool left =
            get_boolean(
                left_token,
                environment
            );

        bool right =
            get_boolean(
                right_token,
                environment
            );

        switch (operation) {

            case TokenType::EqualEqual:
                return left == right;

            case TokenType::NotEqual:
                return left != right;

            default:
                throw std::runtime_error(
                    "Invalid boolean comparison"
                );
        }
    }

    // Number comparison
    int left =
        get_number(
            left_token,
            environment
        );

    int right =
        get_number(
            right_token,
            environment
        );

    switch (operation) {

        case TokenType::EqualEqual:
            return left == right;

        case TokenType::NotEqual:
            return left != right;

        case TokenType::Greater:
            return left > right;

        case TokenType::GreaterEqual:
            return left >= right;

        case TokenType::Less:
            return left < right;

        case TokenType::LessEqual:
            return left <= right;

        default:
            throw std::runtime_error(
                "Invalid comparison"
            );
    }
}

int get_array_index(
    const Token& token,
    const Environment& environment
) {
    return get_number(
        token,
        environment
    );
}

int evaluate_expression(
    const Expression& expression,
    const Environment& environment
) {
    int left =
        get_number(
            expression.left,
            environment
        );

    if (expression.operation.type == TokenType::End) {
        return left;
    }

    int right =
        get_number(
            expression.right,
            environment
        );

    switch (expression.operation.type) {

        case TokenType::Plus:
            return left + right;

        case TokenType::Minus:
            return left - right;

        case TokenType::Star:
            return left * right;

        case TokenType::Slash:

            if (right == 0) {
                throw std::runtime_error(
                    "Cannot divide by zero"
                );
            }

            return left / right;

        default:
            throw std::runtime_error(
                "Invalid expression"
            );
    }
}

void execute_print(
    const PrintStatement& statement,
    const Environment& environment
) {
    if (statement.is_array_access) {

        const auto& array =
            environment.get_array(
                statement.array_access.name
            );

        int index =
            get_array_index(
                statement.array_access.index,
                environment
            );

        if (index < 0 ||
            index >= static_cast<int>(array.size())) {

            throw std::runtime_error(
                "Array index out of bounds"
            );
        }

        std::cout
            << array[index]
            << '\n';

        return;
    }

    if (statement.value.type == TokenType::Identifier) {

        std::cout
            << environment.get(
                statement.value.value
            )
            << '\n';
    }
    else {

        std::cout
            << statement.value.value
            << '\n';
    }
}

ReturnValue execute_program(
    const std::vector<Statement>& program,
    Environment& environment,
    const std::unordered_map<
        std::string,
        FunctionStatement
    >& functions
);

ReturnValue call_function(
    const FunctionStatement& function,
    const std::vector<Token>& arguments,
    Environment& caller_environment,
    const std::unordered_map<
        std::string,
        FunctionStatement
    >& functions
) {
    if (arguments.size() != function.parameters.size()) {

        throw std::runtime_error(
            "Wrong number of arguments for function: " +
            function.name
        );
    }

    Environment local_environment;

    for (size_t i = 0; i < arguments.size(); i++) {

        const Token& argument =
            arguments[i];

        // Boolean argument
        if (argument.type == TokenType::True ||
            argument.type == TokenType::False) {

            bool value =
                get_boolean(
                    argument,
                    caller_environment
                );

            local_environment.set(
                function.parameters[i],
                value ? "true" : "false"
            );
        }

        // Boolean variable
        else if (argument.type == TokenType::Identifier) {

            std::string value =
                caller_environment.get(
                    argument.value
                );

            if (value == "true" ||
                value == "false") {

                local_environment.set(
                    function.parameters[i],
                    value
                );
            }
            else {

                int number =
                    std::stoi(value);

                local_environment.set(
                    function.parameters[i],
                    std::to_string(number)
                );
            }
        }

        // Number
        else {

            int value =
                get_number(
                    argument,
                    caller_environment
                );

            local_environment.set(
                function.parameters[i],
                std::to_string(value)
            );
        }
    }

    return execute_program(
        function.body,
        local_environment,
        functions
    );
}

ReturnValue execute_program(
    const std::vector<Statement>& program,
    Environment& environment,
    const std::unordered_map<
        std::string,
        FunctionStatement
    >& functions
) {
    for (const auto& statement : program) {

        // =========================
        // VARIABLE
        // =========================

        if (statement.type == Statement::Type::Variable) {

            const auto& variable =
                statement.variable;

            // Array
            if (variable.is_array) {

                std::vector<int> values;

                for (const auto& element :
                     variable.array.elements) {

                    values.push_back(
                        std::stoi(
                            element.value
                        )
                    );
                }

                environment.set_array(
                    variable.name,
                    values
                );
            }

            // Array access
            else if (variable.is_array_access) {

                const auto& array =
                    environment.get_array(
                        variable.array_access.name
                    );

                int index =
                    get_array_index(
                        variable.array_access.index,
                        environment
                    );

                if (index < 0 ||
                    index >= static_cast<int>(
                        array.size()
                    )) {

                    throw std::runtime_error(
                        "Array index out of bounds"
                    );
                }

                environment.set(
                    variable.name,
                    std::to_string(
                        array[index]
                    )
                );
            }

            // Function call
            else if (variable.is_function_call) {

                auto it =
                    functions.find(
                        variable.function_call.name
                    );

                if (it == functions.end()) {

                    throw std::runtime_error(
                        "Unknown function: " +
                        variable.function_call.name
                    );
                }

                ReturnValue result =
                    call_function(
                        it->second,
                        variable.function_call.arguments,
                        environment,
                        functions
                    );

                environment.set(
                    variable.name,
                    std::to_string(
                        result.value
                    )
                );
            }

            // Math expression
            else if (variable.is_expression) {

                int result =
                    evaluate_expression(
                        variable.expression,
                        environment
                    );

                environment.set(
                    variable.name,
                    std::to_string(result)
                );
            }

            // Normal variable / boolean / string
            else {

                environment.set(
                    variable.name,
                    variable.value.value
                );
            }
        }

        // =========================
        // PRINT
        // =========================

        else if (
            statement.type ==
            Statement::Type::Print
        ) {

            execute_print(
                statement.print,
                environment
            );
        }

        // =========================
        // IF
        // =========================

        else if (
            statement.type ==
            Statement::Type::If
        ) {

            const auto& condition =
                statement.if_statement;

            bool result = false;

            // if alive
            // if true
            // if false
            if (
                condition.operation.type ==
                TokenType::End
            ) {

                result =
                    get_boolean(
                        condition.left,
                        environment
                    );
            }

            // if x == 5
            // if x > 5
            // if finished == false
            else {

                result =
                    compare(
                        condition.left,
                        condition.operation.type,
                        condition.right,
                        environment
                    );
            }

            if (result) {

                ReturnValue returned =
                    execute_program(
                        condition.body,
                        environment,
                        functions
                    );

                if (returned.returned)
                    return returned;
            }

            else if (condition.has_else) {

                ReturnValue returned =
                    execute_program(
                        condition.else_body,
                        environment,
                        functions
                    );

                if (returned.returned)
                    return returned;
            }
        }

        // =========================
        // WHILE
        // =========================

        else if (
            statement.type ==
            Statement::Type::While
        ) {

            const auto& loop =
                statement.while_statement;

            while (true) {

                if (!compare(
                    loop.left,
                    loop.operation.type,
                    loop.right,
                    environment
                )) {
                    break;
                }

                ReturnValue returned =
                    execute_program(
                        loop.body,
                        environment,
                        functions
                    );

                if (returned.returned)
                    return returned;
            }
        }

        // =========================
        // FOR
        // =========================

        else if (
            statement.type ==
            Statement::Type::For
        ) {

            const auto& loop =
                statement.for_statement;

            const auto& array =
                environment.get_array(
                    loop.array_name
                );

            for (int value : array) {

                environment.set(
                    loop.variable,
                    std::to_string(value)
                );

                ReturnValue returned =
                    execute_program(
                        loop.body,
                        environment,
                        functions
                    );

                if (returned.returned)
                    return returned;
            }
        }

        // =========================
        // RETURN
        // =========================

        else if (
            statement.type ==
            Statement::Type::Return
        ) {

            ReturnValue result;

            result.returned = true;

            result.value =
                evaluate_expression(
                    statement.return_statement.expression,
                    environment
                );

            return result;
        }

        // =========================
        // FUNCTION
        // =========================

        else if (
            statement.type ==
            Statement::Type::Function
        ) {
            // Already registered above.
        }
    }

    return {};
}

int main(
    int argc,
    char* argv[]
) {
    if (argc < 2) {

        std::cout
            << "Usage: dpp <file.dpp>\n";

        return 1;
    }

    std::ifstream file(
        argv[1]
    );

    if (!file) {

        std::cerr
            << "Could not open file: "
            << argv[1]
            << '\n';

        return 1;
    }

    std::stringstream buffer;

    buffer << file.rdbuf();

    Lexer lexer(
        buffer.str()
    );

    auto tokens =
        lexer.tokenize();

    Parser parser(tokens);

    auto program =
        parser.parse_program();

    std::unordered_map<
        std::string,
        FunctionStatement
    > functions;

    // Register all functions first
    for (const auto& statement : program) {

        if (
            statement.type ==
            Statement::Type::Function
        ) {

            functions[
                statement.function.name
            ] = statement.function;
        }
    }

    Environment environment;

    execute_program(
        program,
        environment,
        functions
    );

    return 0;
}