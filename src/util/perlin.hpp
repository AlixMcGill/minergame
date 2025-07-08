#pragma once

#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <random>

class PerlinNoise {
private:
    std::vector<int> p;

    static double fade(double t);
    static double lerp(double a, double b, double t);
    static double grad(int hash, double x, double y);

public:
    explicit PerlinNoise(unsigned int seed = 2023);
    double noise(double x, double y) const;
};

