#pragma once

// Those are not used in this file but are needed in any file that uses the macro
#include <array>
#include <type_traits>

namespace detail {

template <typename VisitorType>
class VisitorConverter {
public:
    VisitorConverter(VisitorType &visitor)
        : m_visitor(visitor)
    {}

    template<typename ...Args>
    void operator()(Args &...values)
    {
        ((*this) << ... << values);
    }

private:
    VisitorConverter &operator<<(auto &value)
    {
        m_visitor(value);
        return *this;
    }

    VisitorType &m_visitor;
};

template <typename T, typename VisitorType>
class VisitorConverterWithName {
public:
    VisitorConverterWithName(VisitorType &visitor)
        : m_visitor(visitor)
    {}

    template<typename ...Args>
    void operator()(Args &...values)
    {
        ((*this) << ... << values);
    }

private:
    VisitorConverterWithName &operator<<(auto &value)
    {
        auto name = T::getFieldNames()[m_currentFieldIndex++];
        m_visitor(value, name);
        return *this;
    }

    int m_currentFieldIndex = 0;
    VisitorType &m_visitor;
};

} // namespace detail

#define NAME(name) \
static constexpr const char* getClassName() { return name; }

#define FIELDS(...) \
template <typename Visitor> \
void forEachField(Visitor &&visitor) \
{ \
    (detail::VisitorConverter<Visitor>(visitor))(__VA_ARGS__); \
} \
template <typename Visitor> \
void forEachField(Visitor &&visitor) const \
{ \
    (detail::VisitorConverter<Visitor>(visitor))(__VA_ARGS__); \
} \
template <typename Visitor> \
void forEachFieldWithName(Visitor &&visitor) \
{ \
    (detail::VisitorConverterWithName< \
        std::remove_reference_t<decltype(*this)>, \
        Visitor \
    >(visitor))(__VA_ARGS__); \
} \
template <typename Visitor> \
void forEachFieldWithName(Visitor &&visitor) const \
{ \
    (detail::VisitorConverterWithName< \
        std::remove_reference_t<decltype(*this)>, \
        Visitor \
    >(visitor))(__VA_ARGS__); \
}

#define FIELD_NAMES(...) \
static constexpr auto getFieldNames() \
{ \
    return std::to_array<const char*>({ __VA_ARGS__ }); \
}

// Describe enums with consecutive values that also start on 0
#define ENUM_DESCRIPTION(type, enumName, valuesNames) \
template<typename T> \
const char *getEnumName(); \
template<> \
constexpr const char *getEnumName<type>() \
{ \
    return enumName; \
} \
constexpr const char *getEnumValueName(type value ) \
{ \
    constexpr const char *names[] = valuesNames; \
    return names[std::to_underlying(value)]; \
}
