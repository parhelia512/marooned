#pragma once

#include "raylib.h"
#include "raymath.h"
#include <vector>

struct GrassInstanceSource
{
    Vector3 position{};
    float yawDeg = 0.0f;
    float scale = 1.0f;

    // 0 = grassCard, 1 = grassCard2, etc.
    int textureIndex = 0;

    Matrix transform{};
};

namespace Grass
{
    extern std::vector<GrassInstanceSource> gGrassInstanceSources;

    void Clear();

    Matrix BuildGrassTransform(Vector3 position, float yawDeg, float scale);

    // For now, this can use your existing grass generation logic.
    // The goal is just to fill gGrassInstanceSources.
    void GenerateFromHeightmap(
        Image& heightmap,
        Vector3 terrainScale,
        float grassSpacing,
        float heightThreshold,
        int maxGrassCards
    );
}