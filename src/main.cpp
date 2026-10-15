#include "main.hpp"

void run(const Config& config) {
    auto gs = initGame(config);

    while (!WindowShouldClose() && !gs->shouldExit) {
        handleWindowEvents(*gs);
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

int main(int argc, char* argv[]) {
    try {
        const ConfigResult loaded = loadConfig(argc, argv);

        if (loaded.showHelp) {
            return 0;
        }

        const Config& config = loaded.config;

        SetTraceLogLevel(config.raylibLogLevel);

        unsigned int flags = FLAG_WINDOW_RESIZABLE;
        if (config.vsync) flags |= FLAG_VSYNC_HINT;
        if (config.msaa4x) flags |= FLAG_MSAA_4X_HINT;
        SetConfigFlags(flags);

        run(config);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Erreur : " << e.what() << '\n';
        return 2;
    }
}
