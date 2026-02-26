#version 460

// Simple pass-through vertex shader for full-screen quad
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;

out VS_OUT {
    vec2 texCoord;
} vs_out;

void main()
{
    gl_Position = vec4(position, 0.0, 1.0);
    vs_out.texCoord = texCoord;
}
