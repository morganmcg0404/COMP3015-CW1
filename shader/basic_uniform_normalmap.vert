#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexColor;
layout (location = 2) in vec3 VertexNormal;
layout (location = 3) in vec2 VertexUV;
layout (location = 4) in vec3 VertexTangent;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec3 Color;
    vec4 FragPosLightSpace;
    vec2 UV;
    vec3 LightDirection;
    vec3 ViewDir;
    mat3 TBN;
} vs_out;

uniform mat4 MVP;
uniform mat4 ModelMatrix;
uniform mat4 NormalMatrix;
uniform mat4 LightSpaceMatrix;
uniform vec3 sunDirection;
uniform vec3 viewPos;

void main()
{
    vs_out.FragPos = vec3(ModelMatrix * vec4(VertexPosition, 1.0));
    vs_out.Normal = normalize(mat3(transpose(inverse(ModelMatrix))) * VertexNormal);
    vs_out.Color = VertexColor;
    vs_out.FragPosLightSpace = LightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
    vs_out.UV = VertexUV;
    
    // Transform normal and tangent to eye/world space
    vec3 norm = normalize(mat3(transpose(inverse(ModelMatrix))) * VertexNormal);
    vec3 tangent = normalize(mat3(transpose(inverse(ModelMatrix))) * VertexTangent);
    
    // Compute binormal
    vec3 binormal = cross(norm, tangent);
    
    // Create TBN matrix for transformation to tangent space
    vs_out.TBN = mat3(tangent, binormal, norm);
    
    // Transform light direction to tangent space
    vec3 lightDir = normalize(-sunDirection);
    vs_out.LightDirection = vs_out.TBN * lightDir;
    
    // Transform view direction to tangent space
    vec3 viewDir = normalize(viewPos - vs_out.FragPos);
    vs_out.ViewDir = vs_out.TBN * viewDir;
    
    gl_Position = MVP * vec4(VertexPosition, 1.0);
}
