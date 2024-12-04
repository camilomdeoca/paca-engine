#pragma once

#define NAME(name) \
constexpr std::string_view getClassName() { return name; }

#define FIELDS(...) \
template<typename T> \
void forEachField(T &val) { val(__VA_ARGS__); } \
template<typename T> \
void forEachField(T &val) const { val(__VA_ARGS__); }

#define FIELD_NAMES(...) \
constexpr std::vector<std::string_view> getFieldNames() { return { __VA_ARGS__ }; }


