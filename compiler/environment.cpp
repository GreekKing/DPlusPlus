#include "environment.hpp"

#include <stdexcept>

void Environment::set(
    const std::string& name,
    const std::string& value
) {
    variables[name] = value;
}

std::string Environment::get(
    const std::string& name
) const {
    auto it = variables.find(name);

    if (it == variables.end()) {
        throw std::runtime_error(
            "Unknown variable: " + name
        );
    }

    return it->second;
}

void Environment::set_array(
    const std::string& name,
    const std::vector<int>& value
) {
    arrays[name] = value;
}

const std::vector<int>& Environment::get_array(
    const std::string& name
) const {
    auto it = arrays.find(name);

    if (it == arrays.end()) {
        throw std::runtime_error(
            "Unknown array: " + name
        );
    }

    return it->second;
}