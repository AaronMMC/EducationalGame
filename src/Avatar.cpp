#include "Avatar.h"

std::vector<Avatar> Avatar::all() {
    std::vector<Avatar> avs;

    // PHOENIX - Comeback king. Weak early, lethal in Round 3.
    {
        Avatar a;
        a.name              = "PHOENIX";
        a.symbol            = "[PH]";
        a.tagline           = "COMEBACK";
        a.ability           = AvatarAbility::Comeback;
        a.accentColor       = {255, 145, 0};
        a.maxMultiplier     = 5.f;
        a.multiGainRate     = 0.08f;
        a.accuracyThreshold = 75.f;
        avs.push_back(a);
    }

    // BLAZE - High risk, high reward. Fastest multi gain but volatile.
    {
        Avatar a;
        a.name              = "BLAZE";
        a.symbol            = "[BZ]";
        a.tagline           = "VOLATILE";
        a.ability           = AvatarAbility::Volatile;
        a.accentColor       = {255, 60, 110};
        a.maxMultiplier     = 6.f;
        a.multiGainRate     = 0.25f;
        a.accuracyThreshold = 90.f;  // Hard threshold - full reset if dropped
        avs.push_back(a);
    }

    // VIRUS - Saboteur. Scrambles opponent text every 2 seconds.
    {
        Avatar a;
        a.name              = "VIRUS";
        a.symbol            = "[VR]";
        a.tagline           = "SABOTEUR";
        a.ability           = AvatarAbility::Saboteur;
        a.accentColor       = {0, 230, 118};
        a.maxMultiplier     = 4.f;
        a.multiGainRate     = 0.1f;
        a.accuracyThreshold = 78.f;
        avs.push_back(a);
    }

    // SENTINEL - Head start. Opponent frozen for 4s at round start.
    {
        Avatar a;
        a.name              = "SENTINEL";
        a.symbol            = "[SN]";
        a.tagline           = "HEAD START";
        a.ability           = AvatarAbility::Starter;
        a.accentColor       = {0, 240, 255};
        a.maxMultiplier     = 4.f;
        a.multiGainRate     = 0.09f;
        a.accuracyThreshold = 80.f;
        avs.push_back(a);
    }

    return avs;
}
