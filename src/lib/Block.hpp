#pragma once
#include "raylib.h"

class Block {
public:
    Block(Vector3 pos, int id = 0) : position(pos), id(id), IsOpaque(false) {}

    Vector3 GetPosition() const { return position; }
    int GetId() const { return id; }
    void SetId(int newId) { id = newId; IsOpaque = (newId != 0); }
    bool GetIfIsOpaque() const { return IsOpaque; }

private:
    const Vector3 position;
    int id;
    bool IsOpaque;
};