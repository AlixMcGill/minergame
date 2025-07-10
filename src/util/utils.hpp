#pragma once

#include <random>
#include <cmath>
#include <raylib.h>

int mapNoiseToRange(float noiseValue, int min, int max);
unsigned int generateRandomSeed();
double findDistance(int x1, int y1, int x2, int y2);
Vector2 diffVector(int x1, int y1, int x2, int y2);

