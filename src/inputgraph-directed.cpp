#include <iomanip>

#include "util.hpp"

template <class T>
auto read_value_or_quit(const std::string_view prompt, const T value_max) -> std::optional<T> {
loop:
    auto str = read_line(prompt);
    if(str.empty()) {
        return std::nullopt;
    }
    if(const auto o = string_parse<size_t>(str)) {
        if(o <= value_max) {
            return o.value();
        } else {
            println("out of range");
            goto loop;
        }
    } else {
        println("invalid number");
        goto loop;
    }
}

struct Graph {
    size_t            size;
    std::vector<bool> data;

    auto debug_print() const -> void {
        std::cout << "a=from b=to" << std::endl;
        std::cout << "a\\b";
        for(auto c = size_t(0); c < size; c += 1) {
            std::cout << std::setw(3) << c;
        }
        std::cout << std::endl;

        for(auto r = size_t(0); r < size; r += 1) {
            std::cout << std::setw(3) << r;
            for(auto c = size_t(0); c < size; c += 1) {
                std::cout << std::setw(3) << at(r, c).value();
            }
            std::cout << std::endl;
        }
    }

    auto at(const size_t r, const size_t c) const -> std::optional<bool> {
        if(r >= size || c >= size) {
            return std::nullopt;
        }

        return data[r * size + c];
    }

    static auto from_stdin() -> Graph {
        const auto size = read_stdin<size_t>("size: ");

        auto graph = Graph{size, std::vector<bool>(size * size)};

    loop:
        auto a = size_t();
        auto b = size_t();
        if(const auto o = read_value_or_quit<size_t>("from: ", size - 1)) {
            a = o.value();
        } else {
            return graph;
        }
        if(const auto o = read_value_or_quit<size_t>("to: ", size - 1)) {
            b = o.value();
        } else {
            return graph;
        }
        graph.data[a * size + b] = true;
        goto loop;
    }
};

auto main() -> int {
    auto graph = Graph::from_stdin();
    graph.debug_print();
    return 0;
}
