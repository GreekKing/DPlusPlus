#pragma once

#include <string>
#include <unordered_map>
#include <vector>

class Environment {
public:
    void set(
        const std::string& name,
        const std::string& value
    );

    std::string get(
        const std::string& name
    ) const;

    void set_array(
        const std::string& name,
        const std::vector<int>& value
    );

    const std::vector<int>& get_array(
        const std::string& name
    ) const;

private:
    std::unordered_map<std::string, std::string> variables;

    std::unordered_map<
        std::string,
        std::vector<int>
    > arrays;
};