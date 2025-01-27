#pragma once

#include "utils/Assert.hpp"
#include "utils/Log.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

// TODO: add all shaderdatatypes
enum class ShaderDataType {
    none,
    float1,
    float2,
    float3,
    float4,
    mat4,
    int1,
    int2,
    int3,
    int4,
    uint1,
    uint2,
    uint3,
    uint4,
};

static uint32_t getSizeOfDataType(ShaderDataType type)
{
    switch (type) {
        case ShaderDataType::float1: return 4;
        case ShaderDataType::float2: return 4 * 2;
        case ShaderDataType::float3: return 4 * 3;
        case ShaderDataType::float4: return 4 * 4;
        case ShaderDataType::mat4:   return 4 * 4 * 4;
        case ShaderDataType::int1:   return 4;
        case ShaderDataType::int2:   return 4 * 2;
        case ShaderDataType::int3:   return 4 * 3;
        case ShaderDataType::int4:   return 4 * 4;
        case ShaderDataType::uint1:  return 4;
        case ShaderDataType::uint2:  return 4 * 2;
        case ShaderDataType::uint3:  return 4 * 3;
        case ShaderDataType::uint4:  return 4 * 4;
        default: break;
    }

    ERROR("Invalid ShaderDatatype!");
    ASSERT(false);
    return 0;
}

struct BufferElement {
    std::string name;
    ShaderDataType type;
    uint32_t size;
    size_t offset;
    bool normalized;

    BufferElement(ShaderDataType type, std::string_view name, bool normalized = false)
        : name(name), type(type), size(getSizeOfDataType(type)), offset(0), normalized(normalized)
    {}

    uint32_t getComponentCount() const
    {
        switch (type) {
            case ShaderDataType::float1: return 1;
            case ShaderDataType::float2: return 2;
            case ShaderDataType::float3: return 3;
            case ShaderDataType::float4: return 4;
            case ShaderDataType::mat4:   return 4;
            case ShaderDataType::int1:   return 1;
            case ShaderDataType::int2:   return 2;
            case ShaderDataType::int3:   return 3;
            case ShaderDataType::int4:   return 4;
            case ShaderDataType::uint1:  return 1;
            case ShaderDataType::uint2:  return 2;
            case ShaderDataType::uint3:  return 3;
            case ShaderDataType::uint4:  return 4;
            default: break;
        }

        ERROR("Invalid ShaderDataType!");
        ASSERT(false);
        return 0;
    }
};

class BufferLayout {
public:
    BufferLayout() {}

    BufferLayout(std::initializer_list<BufferElement> elements)
        : m_elements(elements)
    {
        calculateOffsetAndStride();
    }

    uint32_t getStride() const { return m_stride; }
    const std::vector<BufferElement> &getElements() const { return m_elements; }

    std::vector<BufferElement>::iterator begin() { return m_elements.begin(); }
    std::vector<BufferElement>::iterator end() { return m_elements.end(); }
    std::vector<BufferElement>::const_iterator begin() const { return m_elements.begin(); }
    std::vector<BufferElement>::const_iterator end() const { return m_elements.end(); }

private:
    void calculateOffsetAndStride()
    {
        size_t offset = 0;
        m_stride = 0;
        for (BufferElement &element : m_elements) {
            element.offset = offset;
            offset += element.size;
            m_stride += element.size;
        }
    }

    std::vector<BufferElement> m_elements;
    uint32_t m_stride;
};

class VertexBuffer {
public:
    VertexBuffer(const void *vertices, uint32_t size);
    VertexBuffer(uint32_t size);
    virtual ~VertexBuffer();

    void bind() const;
    void unbind() const;

    void setData(const void *data, uint32_t size);
    void setLayout(const BufferLayout &layout) { m_layout = layout; }
    const BufferLayout &getLayout() const { return m_layout; }

private:
    uint32_t m_id;
    BufferLayout m_layout;
};
