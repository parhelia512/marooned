#pragma once

namespace GameSettings
{
    inline float mouseSensitivity = 0.05f;

    inline constexpr float minMouseSensitivity = 0.01f;
    inline constexpr float maxMouseSensitivity = 0.10f;

    inline float fovY = 45.0f;

    inline constexpr float minFovY = 30.0f;
    inline constexpr float maxFovY = 80.0f;

    inline float maxDrawDist = 30000.0f;
    inline constexpr float minDrawDist = 5000.0f;
    inline constexpr float maxDrawDistLimit = 50000.0f;
}