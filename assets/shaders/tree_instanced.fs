#version 330

in vec2 fragUV;
in vec3 fragPosition;

out vec4 finalColor;

//uniform sampler2D textureDiffuse;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float alphaCutoff;

uniform vec3 cameraPos;

uniform vec3  u_SkyColorTop;
uniform vec3  u_SkyColorHorizon;
uniform float u_FogStart;
uniform float u_FogEnd;
uniform float u_SeaLevel;
uniform float u_FogHeightFalloff;
uniform int   u_UseFog;

uniform float u_ModelNightDarkness;

void main()
{
    //vec4 tex = texture(textureDiffuse, fragUV);
    vec4 tex = texture(texture0, fragUV);
    float alpha = tex.a * colDiffuse.a;

    if (alpha < alphaCutoff) discard;

    vec3 color = tex.rgb * colDiffuse.rgb;

    float nightT = clamp(u_ModelNightDarkness, 0.0, 1.0);
    float nightBrightness = mix(1.0, 0.60, nightT);

    color *= nightBrightness;

    float dist = length(fragPosition - cameraPos);
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