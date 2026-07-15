#ifndef CONFIG_HPP
#define CONFIG_HPP

struct Config {
    int windowWidth;
    int windowHeight;
    int targetFPS;
    std::string gameDirectory;
    std::string username;
    std::string windowTitle;
    int renderDistance;
    float gamma;
    bool atlasRegeneration;
};

#endif // CONFIG_HPP