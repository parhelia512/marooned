#include "debug_overlay.h"

#include <cstdio>
#include "resourceManager.h"


// ------------------------------------------------------------
// Small helpers local to this file
// ------------------------------------------------------------



static const char* FormatTime(float seconds) {
    static char buffer[32];

    int totalSeconds = static_cast<int>(seconds);
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int secs = totalSeconds % 60;

    std::snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", hours, minutes, secs);
    return buffer;
}

static void DrawTextExShadowed(
    Font font,
    const char* text,
    int x,
    int y,
    float fontSize,
    float spacing,
    Color color
) {
    Vector2 pos = { static_cast<float>(x), static_cast<float>(y) };

    DrawTextEx(
        font,
        text,
        { pos.x + 2.0f, pos.y + 2.0f },
        fontSize,
        spacing,
        Fade(BLACK, 0.85f)
    );

    DrawTextEx(
        font,
        text,
        pos,
        fontSize,
        spacing,
        color
    );
}

static void DrawDebugLine(
    Font font,
    const char* label,
    const char* value,
    int x,
    int y,
    int labelWidth,
    float fontSize,
    float spacing,
    Color textColor
) {
    DrawTextExShadowed(font, label, x, y, fontSize, spacing, textColor);
    DrawTextExShadowed(font, value, x + labelWidth, y, fontSize, spacing, textColor);
}

// ------------------------------------------------------------
// Main overlay
// ------------------------------------------------------------

void DrawDebugOverlay(const DebugOverlayInfo& info) {

    Font jetBrains = R.GetFont("jetBrains");
    Font terminal = R.GetFont("terminal");

    const int screenW = GetScreenWidth();
    const int screenH = GetScreenHeight();

    const float scale = 1.25f;
    const float spacing = 1.0f * scale;

    const int lineHeight = static_cast<int>(24 * scale);

    const int panelX = 20;
    const int panelW = static_cast<int>(360 * scale);

    const int padding = static_cast<int>(16 * scale);
    const float fontSize = 18.0f * scale;
    const float titleFontSize = 20.0f * scale;

    const int labelWidth = static_cast<int>(170 * scale);

    // Easier to adjust than hardcoding/scaling the panel height twice
    const int lineCount = 19;
    const int panelH = padding * 2 + lineHeight * lineCount;
    const int panelY = screenH / 2 - panelH / 2;

    const Color panelColor = Fade(BLACK, 0.55f);
    const Color borderColor = Fade(GREEN, 0.65f);
    const Color textColor = { 120, 255, 120, 255 };
    const Color titleColor = { 180, 255, 180, 255 };

    // Panel background
    DrawRectangle(panelX, panelY, panelW, panelH, panelColor);

    // Border, doubled for a slightly stronger console look
    DrawRectangleLines(panelX, panelY, panelW, panelH, borderColor);
    DrawRectangleLines(panelX + 2, panelY + 2, panelW - 4, panelH - 4, Fade(GREEN, 0.35f));

    int x = panelX + padding;
    int y = panelY + padding/2;

    DrawTextExShadowed(
        terminal,
        "MAROONED DEBUG",
        x,
        y,
        titleFontSize,
        spacing,
        titleColor
    );
    y += lineHeight;

    char buffer[128];

    DrawDebugLine(terminal, "FPS", TextFormat("%d", info.fps),
                  x, y, labelWidth, fontSize, spacing, textColor);
    y += lineHeight;

    DrawDebugLine(terminal, "UPTIME", FormatTime(info.elapsedTime),
                  x, y, labelWidth, fontSize, spacing, textColor);
    y += lineHeight;

    DrawDebugLine(terminal, "VSync", info.useVsync ? "true" : "false",
                  x, y, labelWidth, fontSize, spacing, textColor);
    y += lineHeight;

    DrawDebugLine(terminal, "FreeCam", info.freeCam ? "true" : "false",
                  x, y, labelWidth, fontSize, spacing, textColor);
    y += lineHeight;

    DrawDebugLine(terminal, "Show Ceiling", info.showCeiling ? "true" : "false",
                  x, y, labelWidth, fontSize, spacing, textColor);
    y += lineHeight;

    DrawDebugLine(terminal, "Level Name", info.levelName,
                  x, y, labelWidth, fontSize, spacing, textColor);
    y += lineHeight;

    DrawDebugLine(terminal, "Level Index", TextFormat("%d", info.levelIndex),
                  x, y, labelWidth, fontSize, spacing, textColor);
    y += lineHeight;

    std::snprintf(buffer, sizeof(buffer), "%.0f", info.drawDistance);
    DrawDebugLine(terminal, "DRAW DIST", buffer,
                  x, y, labelWidth, fontSize, spacing, textColor);
    y += lineHeight;

    std::snprintf(buffer, sizeof(buffer), "%.1f", info.fovY);
    DrawDebugLine(terminal, "FOV Y", buffer,
                  x, y, labelWidth, fontSize, spacing, textColor);
    y += lineHeight;

    std::snprintf(
        buffer,
        sizeof(buffer),
        "%4d / %4d",
        info.visibleFloorTiles,
        info.totalFloorTiles
    );

    DrawDebugLine(terminal, "FLOOR TILES", buffer,
                  x, y, labelWidth, fontSize, spacing, textColor);
    y += lineHeight;

    std::snprintf(
        buffer,
        sizeof(buffer),
        "%4d / %4d",
        info.visibleFoliage,
        info.totalFoliage
    );

    DrawDebugLine(terminal, "FOLIAGE", buffer,
                x, y, labelWidth, fontSize, spacing, textColor);
    y += lineHeight;

    std::snprintf(buffer, sizeof(buffer), "%.0f%%", info.skyTransition * 100.0f);
    DrawDebugLine(terminal, "DAY/NIGHT", buffer,
                  x, y, labelWidth, fontSize, spacing, textColor);
    y += lineHeight;

    DrawDebugLine(terminal, "Dungeon Size", TextFormat("%d x %d", info.dungeonWidth, info.dungeonHeight),
                  x, y, labelWidth, fontSize, spacing, textColor);
    y += lineHeight;

    DrawDebugLine(terminal, "Static Lights", TextFormat("%d", info.staticLights),
                  x, y, labelWidth, fontSize, spacing, textColor);
    y += lineHeight;

    DrawDebugLine(terminal, "Dynamic Lights", TextFormat("%d", info.dynamicLights),
                  x, y, labelWidth, fontSize, spacing, textColor);
    y += lineHeight;

    DrawDebugLine(terminal, "Lightmap", TextFormat("%d x %d", info.lightmapWidth, info.lightmapHeight),
                  x, y, labelWidth, fontSize, spacing, textColor);
    y += lineHeight;

    DrawDebugLine(terminal, "Active Enemies", TextFormat("%d", info.activeEnemies),
                  x, y, labelWidth, fontSize, spacing, textColor);
    y += lineHeight;

    DrawDebugLine(terminal, "Active Bullets", TextFormat("%d", info.activeBullets),
                  x, y, labelWidth, fontSize, spacing, textColor);
    y += lineHeight;

    DrawDebugLine(terminal, "Weapon", info.currentWeapon,
                x, y, labelWidth, fontSize, spacing, textColor);
    y += lineHeight;

    if (info.showFreeCameraHint) {
        const char* hint = "TAB: FREE CAMERA";

        const float hintFontSize = 20.0f * scale;
        Vector2 hintSize = MeasureTextEx(terminal, hint, hintFontSize, spacing);

        int hintX = screenW / 2 - static_cast<int>(hintSize.x / 2.0f);
        int hintY = static_cast<int>(15 * scale);

        DrawTextExShadowed(
            terminal,
            hint,
            hintX,
            hintY,
            hintFontSize,
            spacing,
            WHITE
        );
    }
}

// void DrawDebugOverlay(const DebugOverlayInfo& info) {


//     Font jetBrains = R.GetFont("jetBrains");
//     const int screenW = GetScreenWidth();
//     const int screenH = GetScreenHeight();
//     const float scale = 1.25;
//     const float spacing = 1.0f * scale;
//     const int lineHeight = static_cast<int>(24 * scale);

//     const int panelX = 20;//(360 * scale);
  
//     const int panelW = static_cast<int>(360 * scale);
//     const int panelH = static_cast<int>((lineHeight * 16) * scale);
//     const int panelY = screenH/2 - panelH/2;

//     const int padding = static_cast<int>(16 * scale);
//     const int fontSize = static_cast<int>(18 * scale);
//     const int titleFontSize = static_cast<int>(20 * scale);

//     const int labelWidth = static_cast<int>(170 * scale);

//     const Color panelColor = Fade(BLACK, 0.25f);
//     const Color borderColor = Fade(GREEN, 0.55f);
//     const Color textColor = { 120, 255, 120, 200 };
//     const Color titleColor = { 180, 255, 180, 200 };

//     // Panel background
//     DrawRectangle(panelX, panelY, panelW, panelH, panelColor);

//     // Border, doubled for a slightly stronger console look
//     DrawRectangleLines(panelX, panelY, panelW, panelH, borderColor);
//     DrawRectangleLines(panelX + 2, panelY + 2, panelW - 4, panelH - 4, Fade(GREEN, 0.35f));

//     int x = panelX + padding;
//     int y = panelY + padding;

//     DrawTextShadowed("MAROONED DEBUG", x, y, 15 * scale, titleColor);
//     y += lineHeight;

//     char buffer[128];

//     DrawDebugLine("FPS", TextFormat("%d", info.fps), x, y, labelWidth, fontSize, textColor);
//     y += lineHeight;

//     DrawDebugLine("UPTIME", FormatTime(info.elapsedTime), x, y, labelWidth, fontSize, textColor);
//     y += lineHeight;

//     DrawDebugLine("VSync", info.useVsync ? "true" : "false",
//                 x, y, labelWidth, fontSize, textColor);
//     y += lineHeight;

//     DrawDebugLine("FreeCam", info.freeCam ? "true" : "false", x, y, labelWidth, fontSize, textColor);
//     y += lineHeight;

//     DrawDebugLine("Show Ceiling", info.showCeiling ? "true" : "false", x, y, labelWidth, fontSize, textColor);
//     y += lineHeight;



//     DrawDebugLine("Level Name", info.levelName.c_str(), x, y, labelWidth, fontSize, textColor);
//     y += lineHeight;

//     DrawDebugLine("Level Index", TextFormat("%d", info.levelIndex), x, y, labelWidth, fontSize, textColor);
//     y += lineHeight;


//     std::snprintf(buffer, sizeof(buffer), "%.0f", info.drawDistance);
//     DrawDebugLine("DRAW DIST", buffer, x, y, labelWidth, fontSize, textColor);
//     y += lineHeight;

//     std::snprintf(buffer, sizeof(buffer), "%.1f", info.fovY);
//     DrawDebugLine("FOV Y", buffer, x, y, labelWidth, fontSize, textColor);
//     y += lineHeight;

//     std::snprintf(
//         buffer,
//         sizeof(buffer),
//         "%4d / %4d",
//         info.visibleFloorTiles,
//         info.totalFloorTiles
//     );
    
//     DrawDebugLine("FLOOR TILES", buffer, x, y, labelWidth, fontSize, textColor);
//     y += lineHeight;

//     std::snprintf(buffer, sizeof(buffer), "%.2f", info.skyTransition);
//     DrawDebugLine("SKY CYCLE", buffer, x, y, labelWidth, fontSize, textColor);
//     y += lineHeight;

//     std::snprintf(buffer, sizeof(buffer), "%.0f%%", info.skyTransition * 100.0f);
//     DrawDebugLine("DAY/NIGHT", buffer, x, y, labelWidth, fontSize, textColor);
//     y += lineHeight;

//     DrawDebugLine("Dungeon Size", TextFormat("%d x %d", info.dungeonWidth, info.dungeonHeight), x, y, labelWidth, fontSize, textColor);
//     y += lineHeight;

//     DrawDebugLine("Static Lights", TextFormat("%d", info.staticLights), x, y, labelWidth, fontSize, textColor);
//     y += lineHeight;

//     DrawDebugLine("Dynamic Lights", TextFormat("%d", info.dynamicLights), x, y, labelWidth, fontSize, textColor);
//     y += lineHeight;

//     DrawDebugLine("Lightmap", TextFormat("%d x %d", info.lightmapWidth, info.lightmapHeight), x, y, labelWidth, fontSize, textColor);
//     y += lineHeight;

//     DrawDebugLine("Active Enemies", TextFormat("%d", info.activeEnemies), x, y, labelWidth, fontSize, textColor);
//     y += lineHeight;

//     DrawDebugLine("Active Bullets", TextFormat("%d", info.activeBullets), x, y, labelWidth, fontSize, textColor);
//     y += lineHeight;

    

//     if (info.showFreeCameraHint) {
//         const char* hint = "TAB: FREE CAMERA";

//         int hintFontSize = 20;
//         int hintX = screenW / 2 - MeasureText(hint, hintFontSize) / 2;
//         int hintY = 15;

//         DrawTextShadowed(hint, hintX, hintY, hintFontSize, WHITE);
//     }
// }