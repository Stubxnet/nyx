#ifndef GAME_RULES
#define GAME_RULES

#include <map>
#include <string>

class GameRules {
private:
    std::map<std::string, bool> rules;

public:
    GameRules() {
        // i added some random game rules :)
        rules["DoDayLightCycle"] = true;
        rules["LiquidFlows"] = true;

        rules["PlayerDefaultJumpHeight"] = 1.4;
        rules["PlayerDefaultSpeed"] = 5.0;
        rules["PlayerDefaultBreakingEfficiency"] = 1.0;
        rules["PlayerDefaultRange"] = 3.0;
    }

    bool getRule(const std::string& rule) const {
        return rules.at(rule);
    }

    void setRule(const std::string& rule, bool value) {
        rules[rule] = value;
    }
};

#endif // GAME_RULES
