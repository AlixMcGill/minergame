#pragma once

#include "perlin.hpp"
#include <raylib.h>

double PerlinNoise::fade(double t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

double PerlinNoise::lerp(double a, double b, double t) {
    return a + t * (b - a);
}

double PerlinNoise::grad(int hash, double x, double y) {
    int h = hash & 7;  // Use 3 bits
    double u = h < 4 ? x : y;
    double v = h < 4 ? y : x;
    return ((h & 1) ? -u : u) + ((h & 2) ? -2.0 * v : 2.0 * v);
}

PerlinNoise::PerlinNoise(unsigned int seed) {
    p.resize(256);
    std::iota(p.begin(), p.end(), 0);

    std::default_random_engine engine(seed);
    std::shuffle(p.begin(), p.end(), engine);

    p.insert(p.end(), p.begin(), p.end());
}

double PerlinNoise::noise(double x, double y) const {
    int X = (int)std::floor(x) & 255;
    int Y = (int)std::floor(y) & 255;

    double xf = x - std::floor(x);
    double yf = y - std::floor(y);

    double u = fade(xf);
    double v = fade(yf);

    int aa = p[p[X] + Y];
    int ab = p[p[X] + Y + 1];
    int ba = p[p[X + 1] + Y];
    int bb = p[p[X + 1] + Y + 1];

    double x1 = lerp(grad(aa, xf, yf), grad(ba, xf - 1, yf), u);
    double x2 = lerp(grad(ab, xf, yf - 1), grad(bb, xf - 1, yf - 1), u);

    return lerp(x1, x2, v);
}

