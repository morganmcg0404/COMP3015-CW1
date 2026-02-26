#version 460

// Vignette framebuffer effect - darkens edges of screen
in VS_OUT {
    vec2 texCoord;
} fs_in;

uniform sampler2D srcTexture;
uniform float vignetteIntensity;

layout(location = 0) out vec4 FragColor;

void main()
{
    vec4 color = texture(srcTexture, fs_in.texCoord);
    
    // Create vignette effect - darker at edges
    vec2 centerDist = fs_in.texCoord - vec2(0.5);
    float dist = length(centerDist) * 2.0;
    
    // Smooth falloff for vignette
    float vignette = smoothstep(1.2, 0.3, dist);
    
    // Blend vignette intensity
    vignette = mix(1.0, vignette, vignetteIntensity);
    
    // Apply vignette to color
    color.rgb *= vignette;
    
    FragColor = color;
}
