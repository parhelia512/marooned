#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D sceneTexture;
uniform vec2  resolution;

// Bloom + tone mapping
uniform float bloomStrength;       // 0.0 = off, ~0.2–1.0 typical
uniform float uExposure;           // 0.5–2.0 typical
uniform int   uToneMapOperator;    // 0=Off, 1=ACES, 2=Reinhard

// Vignette / status / fade / dungeon
uniform float baseVignetteStrength; // 0.0 = off, ~0.3–0.8 typical

// 0 = normal red damage
// 1 = frozen blue
// 2 = quad damage orange
// 3 = haste yellow
uniform int   vignetteMode;
uniform float vignetteIntensity;    // 0.0–1.0
uniform float fadeToBlack;          // 0.0 = no fade, 1.0 = full black
uniform float dungeonDarkness;      // 0.0 = normal, 1.0 = fully dark
uniform float dungeonContrast;      // 1.0 = normal, >1.0 = more contrast
uniform int   isDungeon;

// --- sRGB <-> linear helpers ---
vec3 toLinear(vec3 c) { return pow(c, vec3(2.2)); }
vec3 toSRGB  (vec3 c) { return pow(c, vec3(1.0 / 2.2)); }

// --- Tone mapping operators, expect linear color ---
vec3 ToneMapACES(vec3 x)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;

    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

vec3 ToneMapReinhard(vec3 x)
{
    return x / (1.0 + x);
}

void main()
{
    vec2 texelSize = 1.0 / resolution;

    // Source scene is assumed to be sRGB.
    vec3 srcSRGB = texture(sceneTexture, fragTexCoord).rgb;
    vec3 srcLin  = toLinear(srcSRGB);

    // ------------------------------------------------------------
    // 1) Bloom in linear space
    // ------------------------------------------------------------
    vec3 bloomLin = vec3(0.0);

    if (bloomStrength > 0.001) //skip bloom iteration if 0.0
    {
        float weightSum = 0.0;
        const float threshold = 0.8;

        for (int y = -2; y <= 2; ++y)
        {
            for (int x = -2; x <= 2; ++x)
            {
                vec2 uv = fragTexCoord + vec2(x, y) * texelSize;
                uv = clamp(uv, vec2(0.0), vec2(1.0));

                vec3 s = toLinear(texture(sceneTexture, uv).rgb);

                float lum = dot(s, vec3(0.2126, 0.7152, 0.0722));
                float w = smoothstep(threshold, threshold + 1.0, lum);

                bloomLin += s * w;
                weightSum += w;
            }
        }

        if (weightSum > 0.0)
        {
            bloomLin /= weightSum;
        }
    }


    vec3 resultLin = srcLin + bloomLin * bloomStrength;

    // ------------------------------------------------------------
    // 2) Exposure + tone mapping in linear space
    // ------------------------------------------------------------
    vec3 mappedLin = resultLin * uExposure;

    if (uToneMapOperator == 1)
    {
        mappedLin = ToneMapACES(mappedLin);
    }
    else if (uToneMapOperator == 2)
    {
        mappedLin = ToneMapReinhard(mappedLin);
    }

    // Convert back to sRGB before doing screen-space UI-ish effects.
    vec3 final = toSRGB(mappedLin);

    // ------------------------------------------------------------
    // 3) Vignette / status / fade / dungeon effects
    // ------------------------------------------------------------
    float dist = distance(fragTexCoord, vec2(0.5));
    float vignette = smoothstep(0.4, 0.8, dist);

    // Base dark corner vignette
    final = mix(final, vec3(0.0), vignette * baseVignetteStrength);

    // Damage / status vignette
    vec3 vignetteColor = vec3(1.0, 0.0, 0.0); // red damage default

    if (vignetteMode == 1)
    {
        vignetteColor = vec3(0.2, 0.6, 1.0); // frozen
    }
    else if (vignetteMode == 2)
    {
        vignetteColor = vec3(1.0, 0.25, 0.0); // quad damage
    }
    else if (vignetteMode == 3)
    {
        vignetteColor = vec3(1.0, 0.9, 0.0); // haste
    }

    final = mix(final, vignetteColor, vignette * vignetteIntensity);

    // Fade to black
    final = mix(final, vec3(0.0), fadeToBlack);

    // Dungeon darkening / contrast / tint
    if (isDungeon == 1)
    {
        final *= 1.0 - dungeonDarkness;

        float midpoint = 0.5;
        final = (final - midpoint) * dungeonContrast + midpoint;

        // Slight blue dungeon tint
        final = mix(final, vec3(0.1, 0.3, 0.7), 0.13);
    }

    finalColor = vec4(clamp(final, 0.0, 1.0), 1.0);
}

// #version 330

// in vec2 fragTexCoord;
// out vec4 finalColor;

// uniform sampler2D sceneTexture;
// uniform vec2  resolution;

// // Simple bloom + tone mapping
// uniform float bloomStrength;       // 0.0 = off, ~0.2–1.0 typical
// uniform float uExposure;           // 0.5–2.0 typical (default 1.0)
// uniform int   uToneMapOperator;    // 0=Off, 1=ACES, 2=Reinhard

// // --- sRGB <-> linear helpers ---
// vec3 toLinear(vec3 c) { return pow(c, vec3(2.2)); }
// vec3 toSRGB  (vec3 c) { return pow(c, vec3(1.0/2.2)); }

// // --- Tone mapping operators (expect linear) ---

// // ACES (Hable-ish)
// vec3 ToneMapACES(vec3 x) {
//     float a = 2.51;
//     float b = 0.03;
//     float c = 2.43;
//     float d = 0.59;
//     float e = 0.14;
//     return clamp((x*(a*x + b)) / (x*(c*x + d) + e), 0.0, 1.0);
// }

// // Simple Reinhard
// vec3 ToneMapReinhard(vec3 x) {
//     return x / (1.0 + x);
// }

// void main() {
//     vec2 texelSize = 1.0 / resolution;

//     // Sample the source scene (sRGB) and convert to linear for math
//     vec3 srcSRGB = texture(sceneTexture, fragTexCoord).rgb;
//     vec3 srcLin  = toLinear(srcSRGB);

//     // --- Basic bloom (bright-pass + small blur, all in linear) ---
//     vec3 bloomLin   = vec3(0.0);
//     float weightSum = 0.0;

//     const float threshold = 0.8; // in linear luminance
//     for (int y = -2; y <= 2; ++y) {
//         for (int x = -2; x <= 2; ++x) {
//             vec2 uv = fragTexCoord + vec2(x, y) * texelSize;
//             vec3 s   = toLinear(texture(sceneTexture, uv).rgb);

//             float lum = dot(s, vec3(0.2126, 0.7152, 0.0722));
//             float w   = smoothstep(threshold, threshold + 1.0, lum);

//             bloomLin   += s * w;
//             weightSum  += w;
//         }
//     }

//     if (weightSum > 0.0) {
//         bloomLin /= weightSum;
//     } else {
//         bloomLin = vec3(0.0);
//     }

//     // Combine scene + bloom in linear space
//     vec3 resultLin = srcLin + bloomLin * bloomStrength;

//     // --- Tone mapping (still linear) ---
//     vec3 mappedLin = resultLin * uExposure;   // exposure pre-scale

//     if (uToneMapOperator == 1) {
//         mappedLin = ToneMapACES(mappedLin);
//     } else if (uToneMapOperator == 2) {
//         mappedLin = ToneMapReinhard(mappedLin);
//     }
//     // 0 = off (just exposure)

//     // Convert back to sRGB for display
//     vec3 resultSRGB = toSRGB(mappedLin);

//     finalColor = vec4(clamp(resultSRGB, 0.0, 1.0), 1.0);
// }

