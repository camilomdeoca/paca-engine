#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <type_traits>

namespace paca::fileformats {

template<typename Test, template<typename...> class Ref>
struct is_specialization : std::false_type {};

template<template<typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref>: std::true_type {};

template <template<typename...> class Ref, typename... Args>
constexpr bool is_specialization_v = is_specialization<Ref<Args...>, Ref>::value;

template <typename T>
concept dynamic_array
    = is_specialization<T, std::vector>::value
    || is_specialization<T, std::basic_string>::value;

template <typename T>
concept glm_type = requires(T t) {
    { t.length() } -> std::same_as<int>;
    { t[0] } -> std::same_as<decltype(t[0])&>;
};

} // namespace paca::fileformats
