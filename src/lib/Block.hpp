#pragma once
#include "raylib.h"

class Block {
public:
    Block(int id = 0) : id(id), IsOpaque(false) {}

    int GetId() const { return id; }
    void SetId(int newId) { id = newId; IsOpaque = (newId != 0); }
    bool GetIfIsOpaque() const { return IsOpaque; }

private:
    int id;
    bool IsOpaque;
};