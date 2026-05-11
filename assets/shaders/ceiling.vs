#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

out vec2 vUV;
out vec3 vWorldPos;
out vec4 vColor;
out vec3 vNormal;

uniform mat4 mvp;
uniform mat4 matModel;

void main()
{
    vec4 wp = matModel * vec4(vertexPosition, 1.0);
    vWorldPos = wp.xyz;

    vUV     = vertexTexCoord;
    vColor  = vertexColor;
    vNormal = mat3(matModel) * vertexNormal; // fine for uniform scale

    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
