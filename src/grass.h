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

// #pragma once

// #include "raylib.h"
// #include "transparentDraw.h" // BillboardDrawRequest, billboardRequests, Billboard_FixedFlat

// #include <vector>

// namespace Grass
// {
//     struct GrassCard
//     {
//         Vector3 position = { 0, 0, 0 }; // ground position
//         Vector2 size = { 100, 140 };
//         float rotationY = 0.0f;         // radians, because DrawFlatWeb uses MatrixRotateY(rotationY)
//         int textureIndex = 0;
//         Color tint = WHITE;
//     };

//     void Clear();

//     void GenerateFromHeightmap(
//         Image& heightmap,
//         Vector3 terrainScale,
//         float grassSpacing,
//         float heightThreshold,
//         int maxGrassCards
//     );

//     void Gather(const Camera& camera);

//     int GetCount();
//     int GetTotalGrassCards();
//     int GetVisibleGrassCards();
//     int GetVisibleGrassQuads();
// }