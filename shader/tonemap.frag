#version 460

in VS_OUT {
    vec2 TexCoord;
} fs_in;

uniform sampler2D hdrTexture;
uniform float exposure;
uniform float aveLum;
uniform float white;

layout(location = 0) out vec4 FragColor;

void main()
{
    vec3 hdrColor = texture(hdrTexture, fs_in.TexCoord).rgb;
    
    // Calculate luminance (standard formula)
    float luminance = dot(hdrColor, vec3(0.299, 0.587, 0.114));
    
    // Apply tone mapping only to luminance (Lab 5 method)
    // L = (Exposure * L) / AveLum
    float L = (exposure * luminance) / aveLum;
    // L = (L * (1 + L / (White * White))) / (1 + L)
    L = (L * (1.0 + L / (white * white))) / (1.0 + L);
    
    // Preserve the original color by scaling it by the luminance ratio
    // This keeps hue and saturation while tone-mapping brightness
    vec3 toneMapped = hdrColor * (L / max(luminance, 0.001));
    
    // Gamma correction
    toneMapped = pow(toneMapped, vec3(1.0 / 2.2));
    
    FragColor = vec4(toneMapped, 1.0);
}
