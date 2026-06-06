#include "dungeon_props.h"
#include "transparentDraw.h"
#include "resourceManager.h"
#include "world.h"

#include <cmath>

std::vector<DungeonProp> gDungeonProps;

static BillboardDrawRequest MakePropBillboardRequest(
    const DungeonProp& prop,
    const Vector3& cameraPos,
    float rotationY,
    BillboardType billboardType
)
{
    BillboardDrawRequest req{}; // important: zero initializes isPortal, isOpen, openAmount, etc.

    req.type = billboardType;
    req.position = prop.position;


    req.texture = ResourceManager::Get().GetTexture(prop.textureName);
    req.sourceRect = {
        0.0f,
        0.0f,
        (float)req.texture.width,
        (float)req.texture.height
    };

    req.size = prop.size;
    req.tint = prop.tint;
    req.distanceToCamera = DistSq(prop.position, cameraPos);
    req.rotationY = rotationY;
    req.flipX = false;

    // These are portal/door-specific, but zero-init should already handle them.
    req.isPortal = false;
    req.isOpen = false;
    req.openAmount = 0.0f;

    return req;
}

static void PushFixedFlatProp(
    const DungeonProp& prop,
    const Vector3& cameraPos,
    std::vector<BillboardDrawRequest>& requests,
    float rotationY
)
{
    BillboardDrawRequest req = MakePropBillboardRequest(
        prop,
        cameraPos,
        rotationY,
        Billboard_FixedFlat
    );

    requests.push_back(req); //push to main drawRequest pipe line. 
}

static void PushCrossQuadProp(
    const DungeonProp& prop,
    const Vector3& cameraPos,
    std::vector<BillboardDrawRequest>& requests
)
{
    // First sheet.
    PushFixedFlatProp(prop, cameraPos, requests, prop.rotationY);

    // Second sheet crossed through it.
    PushFixedFlatProp(prop, cameraPos, requests, prop.rotationY + 90.0f);
}

void GatherDungeonPropDrawRequests(
    const Vector3& cameraPos,
    std::vector<BillboardDrawRequest>& requests
)
{

    for (const DungeonProp& prop : gDungeonProps)
    {
        if (!prop.active) continue;

        switch (prop.renderMode)
        {
            case DungeonPropRenderMode::Billboard:
            {
                BillboardDrawRequest req = MakePropBillboardRequest(
                    prop,
                    cameraPos,
                    prop.rotationY,
                    Billboard_FacingCamera
                );
            
                requests.push_back(req);
            } break;

            case DungeonPropRenderMode::FixedFlat:
            case DungeonPropRenderMode::WallQuad:
            {
                PushFixedFlatProp(prop, cameraPos, requests, prop.rotationY);
            } break;

            case DungeonPropRenderMode::CrossQuads:
            {
                PushCrossQuadProp(prop, cameraPos, requests);
            } break;

            case DungeonPropRenderMode::FloorQuad:
            {
                // Later. Your current Billboard_FixedFlat probably assumes vertical quads.
                // Don't solve this yet.
            } break;

            case DungeonPropRenderMode::Model:
            {
                // Later. Models should probably be drawn in a separate DrawDungeonPropModels().
            } break;
        }
    }
}


void ClearDungeonProps()
{
    gDungeonProps.clear();
}

static DungeonProp MakeDefaultProp(DungeonPropType type, Vector3 position, float rotationY)
{

    DungeonProp prop;
    prop.type = type;
    prop.position = position;
    prop.rotationY = rotationY;
    prop.active = true;
    prop.hasCollision = false;

    switch (type)
    {
        case DungeonPropType::SpiderWebCorner:
        {
            prop.renderMode = DungeonPropRenderMode::CrossQuads;
            prop.textureName = "spiderWebTexture"; // whatever key you add to ResourceManager
            prop.size = { 180.0f, 180.0f };
            prop.tint = WHITE;
        } break;

    

        case DungeonPropType::HangingWeb:
        {
            prop.renderMode = DungeonPropRenderMode::FixedFlat;
            prop.textureName = "hangingWeb";
            prop.size = { 160.0f, 220.0f };
            prop.tint = WHITE;
        } break;

        case DungeonPropType::WallBanner:
        {
            prop.renderMode = DungeonPropRenderMode::WallQuad;
            prop.textureName = "wallBanner";
            prop.size = { 120.0f, 180.0f };
            prop.tint = WHITE;
        } break;


        default:
        {
            prop.renderMode = DungeonPropRenderMode::CrossQuads;
            prop.textureName = "spiderWebCorner";
            prop.size = { 180.0f, 180.0f };
            prop.tint = WHITE;
        } break;
    }

    return prop;
}

void SpawnDungeonProp(DungeonPropType type, Vector3 position, float rotationY)
{
    //Test prop. Replace this with tile-based spawning later.


}

void GenerateDungeonPropsForCurrentLevel()
{
    // Vector3(6175, 220, 4225)
    // Player Tile: 1, 10


    Vector3 spawnPos = {6175.0f, floorHeight+50.0f, 4225.0f};

    DungeonProp prop = MakeDefaultProp(DungeonPropType::SpiderWebCorner, spawnPos, 0.0f);

    // DungeonProp prop;
    // prop.type = DungeonPropType::SpiderWebCorner;
    // prop.renderMode = DungeonPropRenderMode::CrossQuads;
    // prop.textureName = "spiderWebTexture";
    // prop.position = spawnPos;
    // prop.rotationY = 0.0f;
    // prop.size = { 100.0f, 100.0f };
    // prop.tint = WHITE;
    // prop.hasCollision = false;

    gDungeonProps.push_back(prop);

}

