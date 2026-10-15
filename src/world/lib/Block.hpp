#pragma once
#include <cstdint>

class Block {
public:
    using BlockId = uint16_t;

    Block(BlockId id = 0) : id(id) {}

    BlockId GetId() const { return id; }
    void SetId(BlockId newId) { id = newId; }

    bool IsOpaque() const { return id != 0; }

private:
    BlockId id;
};
