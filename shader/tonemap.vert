#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 2) in vec2 TexCoords;

out VS_OUT {
    vec2 TexCoord;
} vs_out;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;

void main()
{
    vs_out.TexCoord = TexCoords;
    gl_Position = vec4(VertexPosition, 1.0);
}
