#include "TextSnippets.h"
#include <cstdlib>

std::vector<std::string> TextSnippets::normal() {
    return {
        "int main() {\n    std::cout << \"Hello World\";\n    return 0;\n}",
        "for (int i = 0; i < 10; i++) {\n    numbers.push_back(i);\n}",
        "class Player {\npublic:\n    int health = 100;\n    void move();\n};",
        "void update(float dt) {\n    position += velocity * dt;\n    velocity -= drag * dt;\n}",
        "auto it = std::find_if(v.begin(), v.end(),\n    [](int x) { return x > 5; });",
        "template<typename T>\nT clamp(T val, T lo, T hi) {\n    return std::max(lo, std::min(val, hi));\n}",
        "struct Node {\n    int val;\n    Node* next;\n    Node(int v) : val(v), next(nullptr) {}\n};",
        "std::vector<int> result;\nstd::copy_if(src.begin(), src.end(),\n    std::back_inserter(result), pred);",
        "if (auto it = map.find(key); it != map.end()) {\n    return it->second;\n}\nreturn std::nullopt;",
        "void swap(int& a, int& b) {\n    int tmp = a;\n    a = b;\n    b = tmp;\n}",
    };
}

std::vector<std::string> TextSnippets::boss() {
    return {
        "namespace engine {\n    class Renderer {\n    public:\n        void draw(Scene& s) override;\n        void flush() { glFlush(); }\n    };\n}",
        "template<class T, class U>\nauto zip(T a, U b) -> decltype(auto) {\n    return std::make_pair(\n        a.begin(), b.begin());\n}",
        "class EventBus {\n    std::unordered_map<std::type_index,\n        std::vector<std::function<void(Event&)>>> handlers;\npublic:\n    void emit(Event& e);\n};",
    };
}

std::string TextSnippets::getRandom(const std::vector<std::string>& pool) {
    return pool[std::rand() % pool.size()];
}
