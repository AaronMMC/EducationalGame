#include "Avatar.h"

std::vector<Avatar> Avatar::all() {
    return {
        {
            0, "PHOENIX", "[PX]", "COMEBACK",
            "Weak early. Round 3 triggers x3 multiplier burst.",
            AvatarAbility::Comeback,
            sf::Color(240, 192, 0),
            0.2f, 80.0f, 8.0f
        },
        {
            1, "BLAZE", "[BZ]", "VOLATILE",
            "Fast multiplier gain but resets hard below 90% acc.",
            AvatarAbility::Volatile,
            sf::Color(255, 145, 0),
            0.5f, 90.0f, 8.0f
        },
        {
            2, "VIRUS", "[VX]", "SABOTEUR",
            "Every 2s obfuscates 5 chars on opponent screen.",
            AvatarAbility::Saboteur,
            sf::Color(0, 240, 255),
            0.2f, 80.0f, 8.0f
        },
        {
            3, "SENTINEL", "[SN]", "HEAD START",
            "Opponent frozen for 4s before they can type.",
            AvatarAbility::Starter,
            sf::Color(0, 230, 118),
            0.2f, 80.0f, 6.0f
        }
    };
}
