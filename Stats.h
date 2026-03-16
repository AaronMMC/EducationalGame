#pragma once
#include <chrono>

class Stats {
public:
    int totalTyped = 0;
    int errors = 0;
    int correctChars = 0;
    float timeLimit = 30.0f;
    std::chrono::time_point<std::chrono::steady_clock> startTime;
    std::chrono::time_point<std::chrono::steady_clock> endTime;
    bool isFinished = false;

    void start();
    void finish();
    float getElapsedSeconds();
    float getRemainingTime();
    int getWPM();
    int getAccuracy();
};