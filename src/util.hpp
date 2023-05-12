#pragma once
#include <charconv>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

template <class... Args>
auto build_string(Args... args) -> std::string {
    auto ss = std::stringstream();
    (ss << ... << args);
    return ss.str();
}

template <class... Args>
[[noreturn]] auto panic(Args... args) -> void {
    auto ss = std::stringstream();
    (ss << ... << args) << std::endl;
    throw std::runtime_error(ss.str());
}

template <class... Args>
auto dynamic_assert(const bool cond, Args... args) -> void {
    if(!cond) {
        panic(args...);
    }
}

template <class... Args>
auto print(Args&&... args) -> void {
    (std::cout << ... << args);
}

template <class... Args>
auto println(Args&&... args) -> void {
    print(args..., '\n');
}

inline auto read_line(const std::optional<std::string_view> prompt = std::nullopt) -> std::string {
    if(prompt) {
        print(*prompt);
    }
    auto line = std::string();
    std::getline(std::cin, line);
    return line;
}

template <class T>
constexpr auto false_v = false;

template <class T>
auto from_chars(const std::string_view str) -> std::optional<T> {
    // libc++15 has not support to std::from_chars<double>
    if constexpr(std::is_same_v<T, double>) {
        try {
            return std::stod(std::string(str));
        } catch(const std::invalid_argument&) {
            return std::nullopt;
        }
    } else {
        auto r = T();
        if(auto [ptr, ec] = std::from_chars(std::begin(str), std::end(str), r); ec == std::errc{}) {
            return r;
        } else {
            return std::nullopt;
        }
    }
}

template <class T>
inline auto read_stdin(const std::optional<std::string_view> prompt = std::nullopt) -> T {
    while(true) {
        const auto line = read_line(prompt);
        if(const auto o = from_chars<T>(line)) {
            return o.value();
        } else {
            println("invalid input");
            continue;
        }
    }
}

inline auto split(const std::string_view str, const std::string_view sep) -> std::vector<std::string_view> {
    auto ret = std::vector<std::string_view>();
    auto pos = std::string_view::size_type(0);
    while(true) {
        if(pos >= str.size()) {
            break;
        }
        const auto prev = pos;
        pos             = str.find(sep, pos);
        if(pos == std::string_view::npos) {
            if(prev != str.size()) {
                ret.emplace_back(str.substr(prev));
            }
            break;
        }

        ret.emplace_back(str.substr(prev, pos - prev));

        pos += sep.size();
    }
    return ret;
}

inline auto split_strip(const std::string_view str, const char* const delm = " ") -> std::vector<std::string_view> {
    auto vec = split(str, delm);
    std::erase(vec, "");
    return vec;
}
