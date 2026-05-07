#pragma once
#include <string>
#include <SFML/Graphics.hpp>

enum class AvatarAbility {
    Comeback,   // PHOENIX  - x3 burst on round 3
    Volatile,   // BLAZE    - fast multi gain, harsh threshold
    Saboteur,   // VIRUS    - obfuscates opponent chars every 2s
    Starter     // SENTINEL - 4s head start before opponent can type
};

struct Avatar {
    int id;
    std::string name;
    std::string symbol;       // single char/emoji fallback label
    std::string tagline;
    std::string description;
    AvatarAbility ability;
    sf::Color accentColor;

    // Multiplier tuning per avatar
    float multiGainRate;      // how fast multiplier climbs per correct streak step
    float accuracyThreshold;  // accuracy% below which multiplier resets/drops
    float maxMultiplier;

    static std::vector<Avatar> all();
};
