#pragma once
#include <string>
#include <vector>
#include <set>
#include <chrono>
#include "Avatar.h"
#include "Stats.h"

class NetworkSim {
public:
    int   avatarId    = 1;
    float progressPct = 0.f;
    int   wpm         = 0;
    int   accuracy    = 95;
    float hp          = 100.f;
    float multiplier  = 1.f;
    bool  frozen      = false;

    std::set<int> sabotageIndices;

    void  reset(int avId, bool frozenStart);
    float tick(float dt, int textLen, bool localPlayerFrozen);

    bool isConnected() const { return connected; }
    void connect() { connected = true; }

private:
    bool  connected     = false;
    float charsTyped    = 0.f;
    float virusTimer    = 0.f;
    float glitchTimer   = 0.f;
    float oppSpeed      = 0.f;
    int   lastMilestone = 0;
    std::chrono::time_point<std::chrono::steady_clock> startTime;
    bool started = false;
};
