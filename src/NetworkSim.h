#pragma once
#include <string>
#include <vector>
#include <set>
#include <chrono>
#include "Avatar.h"
#include "Stats.h"

// Simulates a remote/CPU opponent.
// In a real network build, replace tick() with incoming UDP/TCP packet parsing.
class NetworkSim {
public:
    int  avatarId      = 1;
    float progressPct  = 0.f;   // 0..1
    int  wpm           = 0;
    int  accuracy      = 95;
    float hp           = 100.f;
    float multiplier   = 1.f;
    bool frozen        = false;  // Sentinel ability freezes them

    // Obfuscation indices the opponent is injecting onto our screen
    std::set<int> sabotageIndices;

    void reset(int avId, bool frozenStart);
    // Call every frame with deltaTime in seconds.
    // textLen is the target text length.
    // Returns damage dealt to local player this tick (0 or small value).
    float tick(float dt, int textLen, bool localPlayerFrozen);

    bool isConnected() const { return connected; }
    void connect() { connected = true; }

private:
    bool connected = false;
    float charsTyped = 0.f;
    float virusTimer = 0.f;
    float oppSpeed   = 0.f;     // chars per second
    std::chrono::time_point<std::chrono::steady_clock> startTime;
    bool started = false;
};
