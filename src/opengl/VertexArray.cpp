#include "VertexArray.hpp"
#include "VertexBuffer.hpp"

#include <GL/glew.h>
#include <cstdio>
#include <memory>

static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
{
    switch (type) {
        case ShaderDataType::float1: return GL_FLOAT;
        case ShaderDataType::float2: return GL_FLOAT;
        case ShaderDataType::float3: return GL_FLOAT;
        case ShaderDataType::float4: return GL_FLOAT;
        case ShaderDataType::mat4:   return GL_FLOAT;
        default: break;
    }

    fprintf(stderr, "Invalid ShaderDataType!\n");
    exit(1);
    return 0;
}

VertexArray::VertexArray()
{
    glCreateVertexArrays(1, &m_id);
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &m_id);
}

void VertexArray::bind()
{
    glBindVertexArray(m_id);
}

void VertexArray::unbind()
{
    glBindVertexArray(0);
}

void VertexArray::addVertexBuffer(const std::shared_ptr<VertexBuffer> &vertexBuffer)
{
    glBindVertexArray(m_id);
    vertexBuffer->bind();

    const BufferLayout &layout = vertexBuffer->getLayout();
    for (const BufferElement &element : layout) // BUG: Cant iterate through layout
    {
        glEnableVertexAttribArray(m_vertexBufferIndex);
        glVertexAttribPointer(m_vertexBufferIndex,
                element.getComponentCount(),
                ShaderDataTypeToOpenGLBaseType(element.type),
                element.normalized ? GL_TRUE : GL_FALSE,
                layout.getStride(),
                (const void*)(intptr_t)element.offset);
        m_vertexBufferIndex++;
        //switch (element.type) {
        //case ShaderDataType::float1:
        //case ShaderDataType::float2:
        //case ShaderDataType::float3:
        //case ShaderDataType::float4:
        //    {
        //        glEnableVertexAttribArray(m_vertexBufferIndex);
        //        glVertexAttribPointer(m_vertexBufferIndex,
        //                element.getComponentCount(),
        //                ShaderDataTypeToOpenGLBaseType(element.type),
        //                element.normalized ? GL_TRUE : GL_FALSE,
        //                layout.getStride(),
        //                (const void*)element.offset);
        //        m_vertexBufferIndex++;
        //    }
        //    break;
        //case ShaderDataType::mat4:
        //    {
        //        uint8_t count = element.getComponentCount();
        //        for (uint8_t i = 0; i < count; i++) {
        //            glEnableVertexAttribArray(m_vertexBufferIndex);
        //            glVertexAttribPointer(m_vertexBufferIndex,
        //                    count,
        //                    ShaderDataTypeToOpenGLBaseType(element.type),
        //                    element.normalized ? GL_TRUE : GL_FALSE,
        //                    layout.getStride(),
        //                    (const void*)(element.offset + sizeof(float) * count * i));
        //            glVertexAttribDivisor(m_vertexBufferIndex, 1);
        //            m_vertexBufferIndex++;
        //        }
        //    }
        //    break;
        //default:
        //    {
        //        fprintf(stderr, "invalid ShaderDataType!\n");
        //        exit(1);
        //    }
        //    break;
        //}
    }
    m_vertexBuffers.push_back(vertexBuffer);
}

void VertexArray::setIndexBuffer(const std::shared_ptr<IndexBuffer> &indexBuffer)
{
    glBindVertexArray(m_id);
    indexBuffer->bind();

    m_indexBuffer = indexBuffer;
}

