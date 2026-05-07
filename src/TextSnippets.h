#pragma once
#include <string>
#include <vector>

class TextSnippets {
public:
    static std::vector<std::string> normal();
    static std::vector<std::string> boss();
    static std::string getRandom(const std::vector<std::string>& pool);
};
