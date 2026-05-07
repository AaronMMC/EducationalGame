#include "Stats.h"
#include <algorithm>

void Stats::start() {
    startTime  = std::chrono::steady_clock::now();
    isFinished = false;
    isStarted  = true;
}

void Stats::finish() {
    endTime    = std::chrono::steady_clock::now();
    isFinished = true;
}

void Stats::reset() {
    totalTyped   = 0;
    errors       = 0;
    correctChars = 0;
    isFinished   = false;
    isStarted    = false;
}

float Stats::getElapsedSeconds() {
    if (!isStarted) return 0.f;
    auto end = isFinished ? endTime : std::chrono::steady_clock::now();
    std::chrono::duration<float> elapsed = end - startTime;
    return elapsed.count();
}

float Stats::getRemainingTime() {
    return std::max(0.0f, timeLimit - getElapsedSeconds());
}

int Stats::getWPM() {
    float minutes = getElapsedSeconds() / 60.0f;
    if (minutes <= 0.0f) return 0;
    return static_cast<int>((correctChars / 5.0f) / minutes);
}

int Stats::getAccuracy() {
    if (totalTyped == 0) return 100;
    return static_cast<int>((static_cast<float>(correctChars) / totalTyped) * 100.0f);
}
