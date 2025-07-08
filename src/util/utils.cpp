#include "utils.hpp"

int mapNoiseToRange(float noiseValue, int min, int max) {
    float normalized = (noiseValue + 1.0f) / 2.0f;
    return static_cast<int>(normalized * (max - min)) + min;
}

unsigned int generateRandomSeed() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned int> dist(0, std::numeric_limits<unsigned int>::max());
    return dist(gen);
}

double findDistance(int x1, int y1, int x2, int y2) {
    double d2 = std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2);
    return std::sqrt(d2);
}

