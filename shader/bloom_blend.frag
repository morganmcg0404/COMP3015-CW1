#version 460

in VS_OUT {
    vec2 TexCoords;
} fs_in;

uniform sampler2D scene;
uniform sampler2D bloom;
uniform float bloomIntensity;

layout(location = 0) out vec4 FragColor;

void main()
{
    vec3 sceneColor = texture(scene, fs_in.TexCoords).rgb;
    vec3 bloomColor = texture(bloom, fs_in.TexCoords).rgb;
    
    // Add bloom to scene
    vec3 result = sceneColor + bloomColor * bloomIntensity;
    
    // Apply gamma correction
    const float gamma = 2.2;
    result = pow(result, vec3(1.0 / gamma));
    
    FragColor = vec4(result, 1.0);
}
