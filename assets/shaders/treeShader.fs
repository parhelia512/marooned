#version 330
in vec2 fragUV;
in vec3 fragPosition;
out vec4 finalColor;

uniform sampler2D textureDiffuse;   // bound via MATERIAL_MAP_ALBEDO
uniform vec4  colDiffuse;           // raylib material tint (keep WHITE)
uniform float alphaCutoff;          // e.g. 0.3

uniform vec3 cameraPos;

// same fog uniforms you used for terrain
uniform vec3  u_SkyColorTop;
uniform vec3  u_SkyColorHorizon;
uniform float u_FogStart;
uniform float u_FogEnd;
uniform float u_SeaLevel;
uniform float u_FogHeightFalloff;
uniform int   u_UseFog; // 1 = use fog, 0 = skip fog

// Overall model/tree darkness.
// Tie this to gSky.skyTransition from C++.
// 0.0 = day, 1.0 = full night.
uniform float u_ModelNightDarkness;

void main()
{
    vec4 tex = texture(textureDiffuse, fragUV);
    float alpha = tex.a * colDiffuse.a;

    // Alpha-test foliage
    if (alpha < alphaCutoff) discard;

    vec3 color = tex.rgb * colDiffuse.rgb;

    // ---- General night darkening
    // This affects nearby trees/models too, not just distant fog.
    float nightT = clamp(u_ModelNightDarkness, 0.0, 1.0);

    // Tweak this 0.60 value.
    // Lower = darker at night.
    float nightBrightness = mix(1.0, 0.60, nightT);

    color *= nightBrightness;

    // ---- Sky-colored fog
    float dist    = length(fragPosition - cameraPos);
    float fogDist = smoothstep(u_FogStart, u_FogEnd, dist);

    float h = max(fragPosition.y - u_SeaLevel, 0.0);
    float fogHeight = exp(-h * u_FogHeightFalloff);
    float fog = clamp(fogDist * fogHeight, 0.0, 1.0);

    float skyT = clamp((cameraPos.y - u_SeaLevel) / 600.0, 0.0, 1.0);
    vec3 skyColor = mix(u_SkyColorHorizon, u_SkyColorTop, skyT);

    vec3 outRGB = color;

    if (u_UseFog == 1)
    {
        outRGB = mix(color, skyColor, fog);
    }

    finalColor = vec4(outRGB, alpha);
}