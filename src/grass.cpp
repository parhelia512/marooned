#include "grass.h"

#include <iostream>

namespace Grass
{
    std::vector<GrassInstanceSource> gGrassInstanceSources;

    void Clear()
    {
        gGrassInstanceSources.clear();
    }

    Matrix BuildGrassTransform(Vector3 position, float yawDeg, float scale)
    {
        Matrix matScale = MatrixScale(scale, scale, scale);
        Matrix matRot   = MatrixRotateY(yawDeg * DEG2RAD);
        Matrix matTrans = MatrixTranslate(position.x, position.y, position.z);

        // Same general order raylib examples use for instancing:
        // scale -> rotate -> translate
        return MatrixMultiply(MatrixMultiply(matScale, matRot), matTrans);
    }

void GenerateFromHeightmap(
    Image& heightmap,
    Vector3 terrainScale,
    float grassSpacing,
    float heightThreshold,
    int maxGrassCards
)
    {
        Clear();
        gGrassInstanceSources.reserve(maxGrassCards);

        if (heightmap.data == nullptr)
        {
            std::cout << "Grass::GenerateFromHeightmap failed: heightmap.data is null\n";
            return;
        }

        Image heightCopy = ImageCopy(heightmap);
        ImageFormat(&heightCopy, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE);

        unsigned char* pixels = (unsigned char*)heightCopy.data;

        int step = (int)grassSpacing;
        if (step < 1) step = 1;

        const float jitterAmount = grassSpacing * 0.45f;

        for (int z = 0; z < heightCopy.height; z += step)
        {
            for (int x = 0; x < heightCopy.width; x += step)
            {
                if ((int)gGrassInstanceSources.size() >= maxGrassCards)
                {
                    UnloadImage(heightCopy);
                    return;
                }

                // Randomly offset the sample so the grass does not form a perfect grid.
                int jitterX = GetRandomValue((int)-jitterAmount, (int)jitterAmount);
                int jitterZ = GetRandomValue((int)-jitterAmount, (int)jitterAmount);

                int sampleX = x + jitterX;
                int sampleZ = z + jitterZ;

                sampleX = Clamp(sampleX, 0, heightCopy.width - 1);
                sampleZ = Clamp(sampleZ, 0, heightCopy.height - 1);

                int index = sampleZ * heightCopy.width + sampleX;

                unsigned char heightValue = pixels[index];
                float height01 = heightValue / 255.0f;

                if (height01 < heightThreshold)
                    continue;

                // Optional: skip some valid spots so it looks more natural.
                // Higher number = less grass.
                if (GetRandomValue(0, 100) > 50)
                    continue;

                float worldX = ((float)sampleX / (float)heightCopy.width)  * terrainScale.x - terrainScale.x * 0.5f;
                float worldZ = ((float)sampleZ / (float)heightCopy.height) * terrainScale.z - terrainScale.z * 0.5f;
                float worldY = height01 * terrainScale.y;

                float sinkAmount = (float)GetRandomValue(5, 25);
                worldY -= sinkAmount;

                GrassInstanceSource grass;
                grass.position = { worldX, worldY, worldZ };

                grass.yawDeg = (float)GetRandomValue(0, 359);

                float randomScale = (float)GetRandomValue(25, 40) / 100.0f;
                grass.scale = randomScale;

                grass.textureIndex = GetRandomValue(0, 3);

                grass.transform = BuildGrassTransform(
                    grass.position,
                    grass.yawDeg,
                    grass.scale
                );

                gGrassInstanceSources.push_back(grass);
            }
        }

        UnloadImage(heightCopy);
    }
}
// #include "grass.h"

// #include "raymath.h"
// #include "ResourceManager.h"
// #include "world.h"
// #include "viewCone.h"
// #include "game_settings.h"
// // Include whatever owns these globals if needed:
// // startPosition
// // dungeonEntrances
// // DungeonEntrance
// // isDungeon / current level checks, etc.

// namespace Grass
// {
//     static std::vector<GrassCard> gGrassCards;

//     float GRASS_DRAW_DISTANCE = 6000.0f;
//     float GRASS_DRAW_DISTANCE_SQR = GRASS_DRAW_DISTANCE * GRASS_DRAW_DISTANCE;

//     static int gVisibleGrassCards = 0;
//     static int gVisibleGrassQuads = 0;

//     static float RandomFloat(float minValue, float maxValue)
//     {
//         float t = (float)GetRandomValue(0, 10000) / 10000.0f;
//         return minValue + (maxValue - minValue) * t;
//     }

//     static float DistanceSqrXZ(Vector3 a, Vector3 b)
//     {
//         float dx = a.x - b.x;
//         float dz = a.z - b.z;
//         return dx * dx + dz * dz;
//     }

//     static Color RandomGrassTint()
//     {
//         // Keep this subtle at first. Too much random tint can look noisy.
//         return {
//             (unsigned char)GetRandomValue(190, 240),
//             (unsigned char)GetRandomValue(210, 255),
//             (unsigned char)GetRandomValue(170, 220),
//             255
//         };
//     }

//     static Vector3 HeightmapPixelToWorld(
//         int x,
//         int z,
//         float height,
//         int imageWidth,
//         int imageHeight,
//         Vector3 terrainScale
//     )
//     {
//         return {
//             (float)x / (float)imageWidth  * terrainScale.x - terrainScale.x * 0.5f,
//             height,
//             (float)z / (float)imageHeight * terrainScale.z - terrainScale.z * 0.5f
//         };
//     }

//     static void PushGrassQuad(
//         const GrassCard& grass,
//         const Camera& camera,
//         Texture2D texture,
//         float rotationY
//     )
//     {
//         BillboardDrawRequest req = {};

//         req.type = Billboard_FixedFlat;

//         // DrawFlatWeb treats position as the CENTER of the quad.
//         // GrassCard position is ground position, so raise center by half height.
//         req.position = {
//             grass.position.x,
//             grass.position.y + grass.size.y * 0.5f,
//             grass.position.z
//         };

//         req.texture = texture;

//         req.sourceRect = {
//             0.0f,
//             0.0f,
//             (float)texture.width,
//             (float)texture.height
//         };

//         req.size = grass.size;
//         req.tint = grass.tint;

//         // Use normal distance if the rest of billboardRequests use normal distance.
//         // Do not mix squared and unsquared distances in the same sorted vector.
//         float dist = Vector3DistanceSqr(camera.position, grass.position);
//         req.distanceToCamera = dist;

//         req.rotationY = rotationY; // radians
//         req.flipX = false;

//         req.isPortal = false;
//         req.isOpen = false;
//         req.openAmount = 0.0f;
//         req.pallet = {};

//         billboardRequests.push_back(req);
//     }

//     void Clear()
//     {
//         gGrassCards.clear();
//     }

//     void GenerateFromHeightmap(
//         Image& heightmap,
//         Vector3 terrainScale,
//         float grassSpacing,
//         float heightThreshold,
//         int maxGrassCards
//     )
//     {
//         Clear();
//         gGrassCards.reserve(maxGrassCards);

//         // Tune these.
//         const float startClearRadius = 900.0f;
//         const float startClearRadiusSqr = startClearRadius * startClearRadius;

//         const float entranceClearRadius = grassSpacing * 3.0f;
//         const float entranceClearRadiusSqr = entranceClearRadius * entranceClearRadius;

//         if (heightmap.data == nullptr)
//         {
//             std::cout << "Grass::GenerateFromHeightmap failed: heightmap.data is null\n";
//             return;
//         }

//         unsigned char* pixels = (unsigned char*)heightmap.data;

//         for (int z = 0; z < heightmap.height; z += (int)grassSpacing)
//         {
//             for (int x = 0; x < heightmap.width; x += (int)grassSpacing)
//             {
//                 if ((int)gGrassCards.size() >= maxGrassCards)
//                     return;

//                 // Randomly skip some cells so it is not a perfect grid.
//                 // Lower number = denser grass.
//                 if (GetRandomValue(0, 100) > 65)
//                     continue;

//                 int jitterX = GetRandomValue(-(int)(grassSpacing * 0.45f), (int)(grassSpacing * 0.45f));
//                 int jitterZ = GetRandomValue(-(int)(grassSpacing * 0.45f), (int)(grassSpacing * 0.45f));

//                 int sx = x + jitterX;
//                 int sz = z + jitterZ;

//                 if (sx < 0 || sx >= heightmap.width || sz < 0 || sz >= heightmap.height)
//                     continue;

//                 int i = sz * heightmap.width + sx;

//                 float height = ((float)pixels[i] / 255.0f) * terrainScale.y;

//                 if (height <= heightThreshold)
//                     continue;

//                 Vector3 pos = HeightmapPixelToWorld(
//                     sx,
//                     sz,
//                     height,
//                     heightmap.width,
//                     heightmap.height,
//                     terrainScale
//                 );

//                 // Optional: sink slightly so bottom edge hides in the terrain.
//                 pos.y -= 3.0f;

//                 // Don't spawn at player/start clearing.
//                 if (DistanceSqrXZ(pos, startPosition) < startClearRadiusSqr)
//                     continue;

//                 // Don't spawn on dungeon entrances.
//                 bool tooCloseToEntrance = false;
//                 for (const DungeonEntrance& d : dungeonEntrances)
//                 {
//                     if (DistanceSqrXZ(pos, d.position) < entranceClearRadiusSqr)
//                     {
//                         tooCloseToEntrance = true;
//                         break;
//                     }
//                 }

//                 if (tooCloseToEntrance)
//                     continue;

//                 GrassCard grass;
//                 grass.position = pos;

//                 float scale = RandomFloat(0.75f, 1.35f);

//                 grass.size = {
//                     RandomFloat(70.0f, 120.0f) * scale,
//                     RandomFloat(90.0f, 170.0f) * scale
//                 };

//                 // IMPORTANT: radians, because your DrawFlatWeb uses MatrixRotateY(rotationY).
//                 grass.rotationY = RandomFloat(0.0f, PI * 2.0f);

//                 // Later you can randomize grassCard2/3.
//                 grass.textureIndex = GetRandomValue(0, 3);


//                 grass.tint = RandomGrassTint();

//                 gGrassCards.push_back(grass);
//             }
//         }
//     }

//     void Gather(const Camera& camera)
//     {
//         gVisibleGrassCards = 0;
//         gVisibleGrassQuads = 0;

//         Texture2D grassTexture = R.GetTexture("grassCard");
//         Texture2D grassTexture2 = R.GetTexture("grassCard2");
//         Texture2D grassTexture3 = R.GetTexture("grassCard3");
//         Texture2D grassTexture4 = R.GetTexture("grassCard4");

//         Texture2D grassTextures[4] = {
//             grassTexture,
//             grassTexture2,
//             grassTexture3,
//             grassTexture4
//         };

//         ViewConeParams vp = MakeViewConeParams(
//             camera,
//             55.0f,
//             GameSettings::maxDrawDist,
//             400.0f
//         );

//         GRASS_DRAW_DISTANCE_SQR = (currentGameState == GameState::Menu)
//             ? 15000 * 15000
//             : 6000 * 6000;

//         for (const GrassCard& grass : gGrassCards)
//         {
//             float distSqr = Vector3DistanceSqr(camera.position, grass.position);

//             if (distSqr > GRASS_DRAW_DISTANCE_SQR)
//                 continue;

//             if (!IsInViewCone(vp, grass.position))
//                 continue;

//             int index = grass.textureIndex;

//             if (index < 0 || index >= 4)
//                 index = 0;

//             Texture2D selectedTexture = grassTextures[index];

//             PushGrassQuad(grass, camera, selectedTexture, grass.rotationY);
//             PushGrassQuad(grass, camera, selectedTexture, grass.rotationY + PI * 0.5f);

//             gVisibleGrassCards++;
//             gVisibleGrassQuads += 2;
//         }
//     }

//     int GetCount()
//     {
//         return (int)gGrassCards.size();
//     }

//     int GetTotalGrassCards()
//     {
//         return (int)gGrassCards.size();
//     }

//     int GetVisibleGrassCards()
//     {
//         return gVisibleGrassCards;
//     }

//     int GetVisibleGrassQuads()
//     {
//         return gVisibleGrassQuads;
//     }
// }