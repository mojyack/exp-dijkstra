#include <cfloat>
#include <iomanip>
#include <limits>

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

constexpr auto invalid_data = std::numeric_limits<double>::quiet_NaN();

struct Graph {
    size_t              size;
    std::vector<double> data;

    auto debug_print() const -> void {
        constexpr auto cell_width = 4;
        std::cout << "a=from b=to" << std::endl;
        std::cout << " a\\b";
        for(auto c = size_t(0); c < size; c += 1) {
            std::cout << std::setw(cell_width) << c;
        }
        std::cout << std::endl;

        for(auto r = size_t(0); r < size; r += 1) {
            std::cout << std::setw(cell_width) << r;
            for(auto c = size_t(0); c < size; c += 1) {
                const auto v = at(r, c).value();
                if(std::isnan(v)) {
                    for(auto i = 0; i < cell_width - 1; i += 1) {
                        std::cout << " ";
                    }
                    std::cout << "X";
                } else {
                    std::cout << std::setw(cell_width) << std::setprecision(cell_width - 1) << v;
                }
            }
            std::cout << std::endl;
        }
    }

    auto at(const size_t r, const size_t c) const -> std::optional<double> {
        if(r >= size || c >= size) {
            return std::nullopt;
        }

        return data[r * size + c];
    }

    static auto from_stdin() -> Graph {
        const auto size = read_stdin<size_t>("size: ");

        auto graph = Graph{size, std::vector<double>(size * size)};
        std::fill(graph.data.begin(), graph.data.end(), invalid_data);

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
        const auto value = read_stdin<double>("weight: ");

        graph.data[a * size + b] = value;
        goto loop;
    }
};

auto main() -> int {
    auto graph = Graph::from_stdin();
    graph.debug_print();
    return 0;
}
