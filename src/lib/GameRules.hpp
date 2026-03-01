#ifndef GAME_RULES_HPP
#define GAME_RULES_HPP

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

        rules["PlayerDefaultJumpHeight"] = 1.0;
        rules["PlayerDefaultSpeed"] = 5.0;
        rules["PlayerDefaultBreakingEfficiency"] = 1.0;
        rules["PlayerDefaultAttackRange"] = 3.0;
        rules["PlayerDefaultInteractionRange"] = 2.0;
    }

    bool getRule(const std::string& rule) const {
        return rules.at(rule);
    }

    void setRule(const std::string& rule, bool value) {
        rules[rule] = value;
    }
};

#endif // GAME_RULES_HPP
