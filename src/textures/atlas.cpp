static std::pair<int,int> AtlasCoordsForBlock(int blockId) {
    switch (blockId) {
        case 1: return {0,0};
        case 2: return {1,0};
        case 3: return {0,1};
        default: return {1,1};
    }
};
