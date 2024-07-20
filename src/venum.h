#pragma once
#include <variant>
#include <optional>
#include <expected>

// helper type for the visitor #4
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

/// Starts the std::visit function
#define vmatch std::visit(overloaded
/// The first part of a lambda statement
#define vcase(T) [&](T& var)
/// Closes the std::visit function passing in the matched value
#define vmatch_value(value) , value)

#define venum(type_name, ...) using type_name = std::variant<__VA_ARGS__>

template<typename T>
using Option = std::optional<T>;

constexpr std::nullopt_t None = std::nullopt;

template<typename T, typename E>
using Result = std::expected<T, E>;

template<typename E>
std::unexpected<E> Error(E);

template<typename E>
inline std::unexpected<E> Error(E error) {
	return std::unexpected(error);
}
