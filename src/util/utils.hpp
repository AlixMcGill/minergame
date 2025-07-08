#pragma once

#include <random>
#include <cmath>
#include <limits>

int mapNoiseToRange(float noiseValue, int min, int max);
unsigned int generateRandomSeed();
double findDistance(int x1, int y1, int x2, int y2);

