#pragma once

static std::pair<int,int> AtlasCoordsForBlock(int blockId) {
    switch (blockId) {
        case 1: return {0,0};
        case 2: return {1,0};
        case 3: return {0,1};
        case 4: return {1,1};
        case 5: return {0,3};
        case 6: return {2,2};
        case 7: return {1,3};
        case 8: return {2,3};
        case 9: return {1,2};
        default: return {2,1};
    }
};
