#include "gamestate.hpp"

void cleanupGame(GameState& gs) {
    unloadAtlas(gs.resources.blocksDefaults);

    if (gs.resources.atlas.id) UnloadTexture(gs.resources.atlas);
    if (gs.resources.background.id) UnloadTexture(gs.resources.background);

    CloseWindow();
}
