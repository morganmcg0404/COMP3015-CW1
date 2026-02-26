#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexColor;
layout (location = 2) in vec3 VertexNormal;
layout (location = 3) in vec2 VertexUV;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec3 Color;
    vec4 FragPosLightSpace;
    vec2 UV;
} vs_out;

uniform mat4 MVP;
uniform mat4 ModelMatrix;
uniform mat4 LightSpaceMatrix;

void main()
{
    vs_out.FragPos = vec3(ModelMatrix * vec4(VertexPosition, 1.0));
    vs_out.Normal = normalize(mat3(transpose(inverse(ModelMatrix))) * VertexNormal);
    vs_out.Color = VertexColor;
    vs_out.FragPosLightSpace = LightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
    vs_out.UV = VertexUV;
    gl_Position = MVP * vec4(VertexPosition, 1.0);
}
