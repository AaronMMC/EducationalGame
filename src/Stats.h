#pragma once
#include <chrono>

class Stats {
public:
    int totalTyped    = 0;
    int errors        = 0;
    int correctChars  = 0;
    float timeLimit   = 60.0f;
    bool isFinished   = false;

    std::chrono::time_point<std::chrono::steady_clock> startTime;
    std::chrono::time_point<std::chrono::steady_clock> endTime;

    void start();
    void finish();
    float getElapsedSeconds();
    float getRemainingTime();
    int getWPM();
    int getAccuracy();
    void reset();
};
