#include "TextSnippets.h"
#include <cstdlib>

std::vector<std::string> TextSnippets::normal() {
    return {
        "int main() {\n    return 0;\n}",
        "for (int i = 0; i < 10; i++) {\n    std::cout << i;\n}",
        "std::vector<int> nums = {1, 2, 3, 4, 5};",
        "auto result = std::find(v.begin(), v.end(), key);",
        "struct Node {\n    int val;\n    Node* next;\n};",
        "void swap(int& a, int& b) {\n    int t = a; a = b; b = t;\n}",
        "template<typename T>\nT clamp(T v, T lo, T hi) {\n    return v < lo ? lo : v > hi ? hi : v;\n}",
        "float lerp(float a, float b, float t) {\n    return a + t * (b - a);\n}",
        "class Stack {\n    std::vector<int> data;\npublic:\n    void push(int v) { data.push_back(v); }\n};",
        "bool isPrime(int n) {\n    for (int i = 2; i * i <= n; i++)\n        if (n % i == 0) return false;\n    return n > 1;\n}",
        "std::sort(arr.begin(), arr.end(), [](int a, int b){ return a > b; });",
        "int fib(int n) { return n <= 1 ? n : fib(n-1) + fib(n-2); }",
        "while (!queue.empty()) {\n    auto node = queue.front();\n    queue.pop();\n}",
        "std::map<std::string, int> freq;\nfor (auto& c : text) freq[c]++;",
        "auto it = std::lower_bound(v.begin(), v.end(), target);",
        "#include <algorithm>\n#include <vector>\n#include <iostream>",
        "float dot(vec3 a, vec3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }",
        "for (auto& [key, val] : myMap) {\n    process(key, val);\n}",
        "std::unique_ptr<Base> obj = std::make_unique<Derived>();",
        "constexpr int SIZE = 1024;\nint buffer[SIZE] = {};",
    };
}

std::vector<std::string> TextSnippets::boss() {
    return {
        "template<typename T, typename Alloc = std::allocator<T>>\nclass vector {\n    T* data_;\n    size_t size_, cap_;\npublic:\n    void push_back(const T& val);\n    void reserve(size_t n);\n};",
        "class ThreadPool {\n    std::vector<std::thread> workers;\n    std::queue<std::function<void()>> tasks;\n    std::mutex mtx;\n    std::condition_variable cv;\n    bool stop = false;\npublic:\n    void enqueue(std::function<void()> f);\n};",
        "int partition(int* arr, int lo, int hi) {\n    int pivot = arr[hi], i = lo - 1;\n    for (int j = lo; j < hi; j++)\n        if (arr[j] <= pivot) std::swap(arr[++i], arr[j]);\n    std::swap(arr[i+1], arr[hi]);\n    return i + 1;\n}",
        "struct Matrix4x4 {\n    float m[4][4];\n    Matrix4x4 operator*(const Matrix4x4& o) const {\n        Matrix4x4 r{};\n        for (int i=0;i<4;i++)\n            for (int j=0;j<4;j++)\n                for (int k=0;k<4;k++)\n                    r.m[i][j]+=m[i][k]*o.m[k][j];\n        return r;\n    }\n};",
        "std::optional<int> binarySearch(const std::vector<int>& v, int t) {\n    int lo = 0, hi = (int)v.size() - 1;\n    while (lo <= hi) {\n        int mid = lo + (hi - lo) / 2;\n        if (v[mid] == t) return mid;\n        if (v[mid] < t) lo = mid + 1;\n        else hi = mid - 1;\n    }\n    return std::nullopt;\n}",
    };
}

std::string TextSnippets::getRandom(const std::vector<std::string>& pool) {
    if (pool.empty()) return "Type this text as fast as you can!";
    return pool[std::rand() % pool.size()];
}
