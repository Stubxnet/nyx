#ifndef BLOCK
#define BLOCK

#include "raylib.h"

class Block {
public:
    Block(Vector3 pos, int id) : position(pos), id(id) {}

    Vector3 GetPosition() const {
        return position;
    }

    int GetId() const {
        return id;
    }

    void SetId(int newId) {
        id = newId;
    }

private:
    const Vector3 position;
    int id;
};

#endif