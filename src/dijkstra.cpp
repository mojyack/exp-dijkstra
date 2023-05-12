#include <cfloat>
#include <fstream>
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
    if(const auto o = from_chars<size_t>(str)) {
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

template <size_t index, class... Args>
constexpr auto parse_multi_strings_impl(const std::array<std::string_view, sizeof...(Args)> args, std::tuple<Args...>& ret) -> bool {
    using T = std::tuple_element_t<index, std::tuple<Args...>>;

    if(const auto r = from_chars<size_t>(args[index]); r) {
        std::get<index>(ret) = r.value();
    } else {
        return false;
    }

    if constexpr(index + 1 < sizeof...(Args)) {
        return parse_multi_strings_impl<index + 1, Args...>(args, ret);
    } else {
        return true;
    }
}

template <class... Args>
auto parse_multi_strings(const std::array<std::string_view, sizeof...(Args)> args) -> std::optional<std::tuple<Args...>> {
    auto ret = std::tuple<Args...>();
    if(parse_multi_strings_impl<0, Args...>(args, ret)) {
        return ret;
    } else {
        return std::nullopt;
    }
}

constexpr auto invalid_data = std::numeric_limits<double>::quiet_NaN();

struct Graph {
    size_t              size;
    std::vector<double> data;

    auto debug_print() const -> void {
        constexpr auto cell_width = 3;
        std::cout << "a=from b=to" << std::endl;
        std::cout << "a\\b";
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

    static auto from_file(const std::string_view path) -> Graph {
        struct Edge {
            size_t from;
            size_t to;
            double weight;
        };

        auto size     = size_t(0);
        auto directed = false;
        auto edges    = std::vector<Edge>();

        auto file     = std::fstream(path);
        auto line     = std::string();
        auto line_num = size_t(0);
        while(std::getline(file, line)) {
            line_num += 1;
            if(line.empty() || line[0] == '#') {
                continue;
            }
            const auto elms = split_strip(line);
            if(elms[0] == "size") {
                if(elms.size() != 2) {
                    println(line_num, ": invalid input");
                    continue;
                }
                if(const auto r = from_chars<size_t>(elms[1]); !r) {
                    println(line_num, ": failed to parse string ", elms[1]);
                    continue;
                } else {
                    size = r.value();
                }
            } else if(elms[0] == "directed") {
                if(elms.size() != 2) {
                    println(line_num, ": invalid input");
                    continue;
                }
                if(elms[1] == "true") {
                    directed = true;
                } else if(elms[1] == "false") {
                    directed = false;
                } else {
                    println(line_num, ": operand to 'directed' must be true or false");
                    continue;
                }
            } else if(elms[0] == "edge") {
                if(elms.size() != 4) {
                    println(line_num, ": invalid input");
                    continue;
                }
                if(const auto r = parse_multi_strings<size_t, size_t, double>({elms[1], elms[2], elms[3]}); !r) {
                    println(line_num, ": failed to parse line");
                    continue;
                } else {
                    auto& v = r.value();
                    edges.emplace_back(Edge{std::get<0>(v), std::get<1>(v), std::get<2>(v)});
                }
            }
        }

        auto graph = Graph{size, std::vector<double>(size * size)};
        std::fill(graph.data.begin(), graph.data.end(), invalid_data);
        for(auto& edge : edges) {
            graph.data[edge.from * size + edge.to] = edge.weight;
            if(!directed) {
                graph.data[edge.to * size + edge.from] = edge.weight;
            }
        }

        return graph;
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

auto solve_dijkstra(const size_t start, const Graph& graph) -> void {
    constexpr auto double_max = std::numeric_limits<double>::max();

    struct Label {
        size_t previous     = size_t(-1);
        double total_weight = double_max;
        bool   visited      = false;
    };

    auto labels = std::vector<Label>(graph.size);

    const auto print_route = [&labels, start](const size_t current) {
        auto c = current;
        while(c != start) {
            const auto p = labels[c].previous;
            if(p == size_t(-1)) {
                break;
            }
            print(" <- ", p + 1);
            c = p;
        }
    };

    auto next                  = start;
    labels[start].total_weight = 0;
loop:
    auto  min             = double_max;
    auto  current         = next;
    auto& current_label   = labels[current];
    current_label.visited = true;

    print("current=", current + 1, "(", current_label.total_weight, ")");
    print_route(current);
    print("\n");

    for(auto dest = size_t(0); dest < graph.size; dest += 1) {
        auto& dest_label = labels[dest];
        if(dest_label.visited) {
            continue;
        }
        const auto weight = graph.at(current, dest).value();
        if(!isnan(weight)) {
            println("  dest=", dest + 1);
            println("    total ", current_label.total_weight, "+", weight, " dest ", dest_label.total_weight);

            const auto new_total_weight = current_label.total_weight + weight;
            if(!isnan(weight) && new_total_weight < dest_label.total_weight) {
                dest_label.total_weight = new_total_weight;
                dest_label.previous     = current;
                println("    previous = ", current + 1);
            }
        }
        if(dest_label.total_weight < min) {
            min  = dest_label.total_weight;
            next = dest;
        }
    }
    if(min < double_max) {
        goto loop;
    }

    println("result:");
    for(auto i = size_t(0); i < graph.size; i += 1) {
        auto& label = labels[i];
        print(i + 1, "(", label.total_weight, ")");
        print_route(i);
        print("\n");
    }
}

auto main(const int argc, const char* const argv[]) -> int {
    if(argc != 1 && argc != 2) {
        println("usage: ", argc >= 1 ? argv[0] : "dijkstra", " ", "{graphfile}");
        return 1;
    }
    auto graph = argc == 1 ? Graph::from_stdin() : Graph::from_file(argv[1]);
    graph.debug_print();
    solve_dijkstra(read_stdin<size_t>("start from: "), graph);
    return 0;
}
