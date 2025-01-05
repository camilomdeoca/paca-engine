#include <filesystem>
#include <string>
#include <print>

#include <serializers/Reflection.hpp>
#include <serializers/BinarySerialization.hpp>

struct TestStruct
{
    FIELDS(i, f, s, ifv);
    int i;
    float f;
    std::string s;
    std::variant<int, float> ifv;
};

void print(const TestStruct &t)
{
    std::println("i: {}, f: {}, s: {}, ifv: {}", t.i, t.f, t.s, t.ifv.index());
}

int main (int argc, char *argv[]) {
    TestStruct t;
    t.i = -2;
    t.f = 0.2;
    t.s = "awda";
    t.ifv = 0.4f;
    {
        serialization::BinarySerializer s("temp");
        s << t;
    }

    TestStruct t2;
    {
        serialization::BinaryUnserializer u("temp");
        u << t2;
    }

    print(t);
    print(t2);

    std::filesystem::remove("temp");

    if (t.i == t2.i && t.f == t2.f && t.s == t2.s && t.ifv == t2.ifv)
        return 0;
    return 1;
}
