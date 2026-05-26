#version 330

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexTexCoord;
layout(location = 2) in vec3 vertexNormal;

// raylib DrawMeshInstanced expects this at location 6
layout(location = 6) in mat4 instanceTransform;

uniform mat4 mvp;

out vec2 fragUV;
out vec3 fragPosition;

void main()
{
    vec4 worldPos = instanceTransform * vec4(vertexPosition, 1.0);

    fragPosition = worldPos.xyz;
    fragUV = vertexTexCoord;

    // IMPORTANT:
    // For instancing, mvp should be view-projection-ish here,
    // and instanceTransform supplies the model transform.
    gl_Position = mvp * worldPos;
}