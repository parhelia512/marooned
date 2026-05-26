#include "vegetation_instanced.h"

#include "raymath.h"
#include "resourceManager.h"
#include "world.h"
#include "game_settings.h"
#include "shadows.h"
#include "utilities.h"
#include "shaderSetup.h"

#include <algorithm>

// These are the CPU-side instance batches.
// Each batch = one model drawn with DrawMeshInstanced().
static VegetationInstanceBatch gPalmTreeBatch;
static VegetationInstanceBatch gBushBatch;
// static VegetationInstanceBatch gPalm2Batch;
// static VegetationInstanceBatch gDeadTreeBatch;


// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------

static void ApplyInstancedShaderToModel(Model& model, Shader shader)
{
    for (int i = 0; i < model.materialCount; ++i)
    {
        model.materials[i].shader = shader;
    }
}

static Matrix MakeInstanceTransform(Vector3 position, float rotationYDeg, float scale)
{
    Matrix matScale = MatrixScale(scale, scale, scale);
    Matrix matRot   = MatrixRotateY(rotationYDeg * DEG2RAD);
    Matrix matTrans = MatrixTranslate(position.x, position.y, position.z);

    // Same order raylib's DrawModelEx effectively uses:
    // scale -> rotate -> translate
    return MatrixMultiply(MatrixMultiply(matScale, matRot), matTrans);
}

static void PushInstance(
    VegetationInstanceBatch& batch,
    Vector3 position,
    float rotationYDeg,
    float scale
)
{
    batch.positions.push_back(position);
    batch.transforms.push_back(MakeInstanceTransform(position, rotationYDeg, scale));
}

static void DrawBatch(VegetationInstanceBatch& batch, Camera& camera)
{
    if (batch.transforms.empty()) return;

    batch.visibleTransforms.clear();

    const float cullDist = GameSettings::maxDrawDist;
    const float cullDistSq = cullDist * cullDist;

    for (int i = 0; i < (int)batch.transforms.size(); ++i)
    {
        float distSq = Vector3DistanceSqr(camera.position, batch.positions[i]);

        if (distSq < cullDistSq)
        {
            batch.visibleTransforms.push_back(batch.transforms[i]);
        }
    }

    if (batch.visibleTransforms.empty()) return;

    Model& model = R.GetModel(batch.modelName.c_str());

    for (int meshIndex = 0; meshIndex < model.meshCount; ++meshIndex)
    {
        int matIndex = model.meshMaterial[meshIndex];

        if (matIndex < 0 || matIndex >= model.materialCount)
        {
            matIndex = 0;
        }

        // TraceLog(LOG_INFO,
        //     "VEG INST DrawBatch %s total=%i visible=%i meshCount=%i materialCount=%i",
        //     batch.modelName.c_str(),
        //     (int)batch.transforms.size(),
        //     (int)batch.visibleTransforms.size(),
        //     model.meshCount,
        //     model.materialCount
        // );
        DrawMeshInstanced(
            model.meshes[meshIndex],
            model.materials[matIndex],
            batch.visibleTransforms.data(),
            (int)batch.visibleTransforms.size()
        );
    }
}

// ------------------------------------------------------------
// Public API
// ------------------------------------------------------------

namespace VegetationInstanced
{
    void Clear()
    {
        gPalmTreeBatch.Clear();
        gBushBatch.Clear();
    }

    void Generate()
    {
        Clear();
        TraceLog(LOG_INFO, "VEG INST Generate() START");

        gPalmTreeBatch.modelName = "palmTreeInstanced";
        gBushBatch.modelName = "bushInstanced";

        generateVegetation();

        TraceLog(LOG_INFO, "VEG INST old trees=%i bushes=%i",
            (int)trees.size(),
            (int)bushes.size());




        for (const TreeInstance& tree : trees)
        {
            float finalScale = tree.scale;

            if (tree.useAltModel)
            {
                finalScale *= 1.15f;   // bigger palms
            }
            else
            {
                finalScale *= 0.85f;   // smaller palms
            }

           
            Vector3 pos = tree.position;
            pos.x += tree.xOffset;
            pos.y += tree.yOffset;
            pos.z += tree.zOffset;

            PushInstance(gPalmTreeBatch, pos, tree.rotationY, finalScale);
        }

        for (const BushInstance& bush : bushes)
        {
            Vector3 pos = bush.position;
            pos.x += bush.xOffset;
            pos.y += bush.yOffset;
            pos.z += bush.zOffset;

            PushInstance(gBushBatch, pos, bush.rotationY, bush.scale);
        }


        TraceLog(LOG_INFO, "INSTANCED VEG: palms=%i bushes=%i",
            (int)gPalmTreeBatch.transforms.size(),
            (int)gBushBatch.transforms.size());
    }

    void Draw(Camera& camera)
    {

        SetShaderValues(camera);
        DrawBatch(gPalmTreeBatch, camera);
        DrawBatch(gBushBatch, camera);

    }

    int GetTotalInstanceCount()
    {
        return
            (int)gPalmTreeBatch.transforms.size() +
            (int)gBushBatch.transforms.size();
    }

    int GetVisibleInstanceCount()
    {
        return
            (int)gPalmTreeBatch.visibleTransforms.size() +
            (int)gBushBatch.visibleTransforms.size();
    }


}

namespace VegetationInstanced
{
    static Shader gShader = { 0 };

    static int locAlphaCutoff = -1;
    static int locCameraPos = -1;
    static int locSkyTop = -1;
    static int locSkyHorizon = -1;
    static int locFogStart = -1;
    static int locFogEnd = -1;
    static int locSeaLevel = -1;
    static int locFogHeightFalloff = -1;
    static int locUseFog = -1;
    static int locModelNightDarkness = -1;

    void InitShader()
    {
        gShader = R.GetShader("tree_instanced");

        TraceLog(LOG_INFO, "VEG INST shader id=%u", gShader.id);

        gShader.locs[SHADER_LOC_MATRIX_MVP] =
            GetShaderLocation(gShader, "mvp");

        // IMPORTANT:
        // In your project, DrawMeshInstanced is using SHADER_LOC_MATRIX_MODEL
        // as the instanceTransform attribute location.
        gShader.locs[SHADER_LOC_MATRIX_MODEL] =
            GetShaderLocationAttrib(gShader, "instanceTransform");

        if (gShader.locs[SHADER_LOC_MATRIX_MODEL] < 0)
        {
            TraceLog(LOG_ERROR, "VEG INST missing instanceTransform");
            return;
        }

        gShader.locs[SHADER_LOC_MAP_DIFFUSE] =
            GetShaderLocation(gShader, "texture0");

        gShader.locs[SHADER_LOC_COLOR_DIFFUSE] =
            GetShaderLocation(gShader, "colDiffuse");

        TraceLog(LOG_INFO,
            "VEG INST loc mvp=%i instanceTransform/MODEL=%i texture0=%i colDiffuse=%i",
            gShader.locs[SHADER_LOC_MATRIX_MVP],
            gShader.locs[SHADER_LOC_MATRIX_MODEL],
            gShader.locs[SHADER_LOC_MAP_DIFFUSE],
            gShader.locs[SHADER_LOC_COLOR_DIFFUSE]
        );

        locAlphaCutoff        = GetShaderLocation(gShader, "alphaCutoff");
        locCameraPos          = GetShaderLocation(gShader, "cameraPos");
        locSkyTop             = GetShaderLocation(gShader, "u_SkyColorTop");
        locSkyHorizon         = GetShaderLocation(gShader, "u_SkyColorHorizon");
        locFogStart           = GetShaderLocation(gShader, "u_FogStart");
        locFogEnd             = GetShaderLocation(gShader, "u_FogEnd");
        locSeaLevel           = GetShaderLocation(gShader, "u_SeaLevel");
        locFogHeightFalloff   = GetShaderLocation(gShader, "u_FogHeightFalloff");
        locUseFog             = GetShaderLocation(gShader, "u_UseFog");
        locModelNightDarkness = GetShaderLocation(gShader, "u_ModelNightDarkness");

        TraceLog(LOG_INFO,
            "VEG INST uniforms alpha=%i cam=%i skyTop=%i skyHor=%i fogStart=%i fogEnd=%i sea=%i falloff=%i useFog=%i night=%i",
            locAlphaCutoff,
            locCameraPos,
            locSkyTop,
            locSkyHorizon,
            locFogStart,
            locFogEnd,
            locSeaLevel,
            locFogHeightFalloff,
            locUseFog,
            locModelNightDarkness
        );

        ApplyInstancedShaderToModel(R.GetModel("palmTreeInstanced"), gShader);
        ApplyInstancedShaderToModel(R.GetModel("bushInstanced"), gShader);

        TraceLog(LOG_INFO, "VEG INST shader applied to palmTreeInstanced and bushInstanced");
    }
}

void VegetationInstanced::SetShaderValues(Camera& camera)
{
    Vector3 camPos = camera.position;
    Vector3 topFogColor = ShaderSetup::GetCurrentSkyTopFogColor();
    Vector3 fogHor = ShaderSetup::GetCurrentSkyFogColor();
    float fogStart = (currentGameState == GameState::Menu) ? 10000 : 100;
    float fogEnd    = 16000.0f;
    float seaLevel  = 400.0f;
    float falloff   = 0.002f;

    float alphaCutoff = 0.50f;

    SetShaderValue(gShader, locCameraPos, &camPos, SHADER_UNIFORM_VEC3);

    SetShaderValue(gShader, locAlphaCutoff, &alphaCutoff, SHADER_UNIFORM_FLOAT);

    SetShaderValue(gShader, locSkyTop, &topFogColor, SHADER_UNIFORM_VEC3);
    SetShaderValue(gShader, locSkyHorizon, &fogHor, SHADER_UNIFORM_VEC3);

    SetShaderValue(gShader, locFogStart, &fogStart, SHADER_UNIFORM_FLOAT);
    SetShaderValue(gShader, locFogEnd, &fogEnd, SHADER_UNIFORM_FLOAT);
    SetShaderValue(gShader, locSeaLevel, &seaLevel, SHADER_UNIFORM_FLOAT);
    SetShaderValue(gShader, locFogHeightFalloff, &falloff, SHADER_UNIFORM_FLOAT);

    int useFog = 1;
    SetShaderValue(gShader, locUseFog, &useFog, SHADER_UNIFORM_INT);

    float nightDarkness = ShaderSetup::gSky.skyTransition;
    SetShaderValue(gShader, locModelNightDarkness, &nightDarkness, SHADER_UNIFORM_FLOAT);
}