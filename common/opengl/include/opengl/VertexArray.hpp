#pragma once

#include "opengl/IndexBuffer.hpp"
#include "opengl/VertexBuffer.hpp"
#include <memory>
#include <vector>

class VertexArray {
public:
    VertexArray();
    virtual ~VertexArray();

    void bind();
    void unbind();

    void addVertexBuffer(const std::shared_ptr<VertexBuffer> &vertexBuffer); // TODO: See if this can be unique ptrs
    void setIndexBuffer(const std::shared_ptr<IndexBuffer> &indexBuffer);

    const std::vector<std::shared_ptr<VertexBuffer>> &getVertexBuffers() const { return m_vertexBuffers; }
    const std::shared_ptr<IndexBuffer> &getIndexBuffer() const { return m_indexBuffer; }

private:
    uint32_t m_id;
    uint32_t m_vertexBufferIndex = 0;
    std::vector<std::shared_ptr<VertexBuffer>>  m_vertexBuffers;
    std::shared_ptr<IndexBuffer> m_indexBuffer;
};
