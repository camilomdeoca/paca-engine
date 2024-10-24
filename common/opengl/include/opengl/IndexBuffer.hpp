#pragma once

#include <cstdint>

class IndexBuffer {
public:
    IndexBuffer(const uint32_t *indices, uint32_t count);
    virtual ~IndexBuffer();

    void bind();
    void unbind();

    uint32_t getCount() const { return m_count; }

private:
    uint32_t m_id;
    uint32_t m_count;
};
