#version 460

in VS_OUT {
    vec2 TexCoords;
} fs_in;

uniform sampler2D screenTexture;
uniform float threshold;

layout(location = 0) out vec4 FragColor;

void main()
{
    vec4 color = texture(screenTexture, fs_in.TexCoords);
    
    // Calculate luminance
    float luminance = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    
    // If luminance exceeds threshold, output the color, else black
    if (luminance > threshold) {
        FragColor = color;
    } else {
        FragColor = vec4(0.0);
    }
}
