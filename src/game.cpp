#include "game.hpp"

void run(const Config& config) {
    auto gs = initGame(config);

    while (!WindowShouldClose() && !gs->shouldExit) {
        switch (gs->currentScreen) {
            case MENU:
                drawMenu(*gs);
                break;
            case OPTIONS:
                drawOptions(*gs);
                break;
            case GAME:
                updateGame(*gs);
                drawGame(*gs);
                break;
        }
    }

    cleanupGame(*gs);
}
