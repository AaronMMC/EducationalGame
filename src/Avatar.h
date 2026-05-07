#pragma once
#include <string>
#include <vector>
#include <SFML/Graphics.hpp>

enum class AvatarAbility {
    Comeback,   // PHOENIX  - R3 forces multiplier to minimum x3
    Volatile,   // BLAZE    - gains multi fastest, full reset below acc threshold
    Saboteur,   // VIRUS    - scrambles 5 chars on opponent screen every 2s
    Starter     // SENTINEL - opponent cannot type for first 4 seconds
};

struct Avatar {
    std::string     name;
    std::string     symbol;
    std::string     tagline;
    AvatarAbility   ability;
    sf::Color       accentColor;

    float maxMultiplier     = 4.f;
    float multiGainRate     = 0.1f;
    float accuracyThreshold = 80.f;  // drop below => lose multiplier

    static std::vector<Avatar> all();
};
