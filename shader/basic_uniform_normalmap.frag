#version 460

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec3 Color;
    vec4 FragPosLightSpace;
    vec2 UV;
    vec3 LightDirection;
    vec3 ViewDir;
    mat3 TBN;
} fs_in;

layout (location = 0) out vec4 FragColor;

uniform sampler2D grassTexture;
uniform sampler2D stoneTexture;
uniform sampler2D dirtTexture;
uniform sampler2D coinTexture;
uniform sampler2D grassNormal;
uniform sampler2D stoneNormal;
uniform sampler2D dirtNormal;
uniform vec3 sunDirection;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 fogColor;
uniform float lightIntensity;
uniform int applyLighting;
uniform bool renderingCoin;
uniform vec3 coinColor;

void main() {
    vec3 color = fs_in.Color;
    
    // If applyLighting is 0, just use light color (for sun/moon/skybox)
    if (applyLighting == 0) {
        FragColor = vec4(lightColor, 1.0);
        return;
    }
    
    // Sample texture color and normal map
    vec3 texColor = color;
    vec3 normal = vec3(0.5, 0.5, 1.0);  // Default normal pointing up
    
    // If rendering coin, use coin color directly
    if (renderingCoin) {
        texColor = coinColor;
    } else {
        // Use color channels as a proxy for terrain type
        float maxChannel = max(color.r, max(color.g, color.b));
        float minChannel = min(color.r, min(color.g, color.b));
        float channelDiff = maxChannel - minChannel;
        
        // Gray colors (mountains) - all channels similar
        if (channelDiff < 0.15 && maxChannel > 0.3) {
            vec3 stoneTex = texture(stoneTexture, fs_in.UV).rgb;
            normal = texture(stoneNormal, fs_in.UV).rgb;
            texColor = mix(color, stoneTex, 0.85);
        }
        // Green dominant (plains/grass) - green significantly higher than other channels
        else if (color.g > color.r + 0.1 && color.g > color.b + 0.1) {
            vec3 grassTex = texture(grassTexture, fs_in.UV).rgb;
            normal = texture(grassNormal, fs_in.UV).rgb;
            texColor = mix(color, grassTex, 0.85);
        }
        // Low areas (browns/dark) - use dirt texture
        else {
            vec3 dirtTex = texture(dirtTexture, fs_in.UV).rgb;
            normal = texture(dirtNormal, fs_in.UV).rgb;
            texColor = mix(color, dirtTex, 0.85);
        }
    }
    
    // Convert normal map from [0, 1] to [-1, 1]
    normal = normalize(normal * 2.0 - 1.0);
    
    // Blinn-Phong with normal mapping in tangent space
    vec3 lightDir = normalize(fs_in.LightDirection);
    vec3 viewDir = normalize(fs_in.ViewDir);
    vec3 halfVec = normalize(lightDir + viewDir);
    
    // Diffuse lighting
    float diffuse = max(dot(normal, lightDir), 0.0);
    float minAmbient = 0.1 * lightIntensity;
    diffuse = max(diffuse, minAmbient);
    
    // Specular lighting
    float spec = pow(max(dot(normal, halfVec), 0.0), 32.0);
    float specular = renderingCoin ? spec * 0.8 : spec * 0.1;
    
    // Apply diffuse and specular lighting with light color
    vec3 diffuseLighting = texColor * diffuse * lightColor;
    vec3 specularLighting = vec3(1.0) * specular * lightColor;
    vec3 lighting = diffuseLighting + specularLighting;
    
    // Fog effect
    float fogDistance = length(viewPos - fs_in.FragPos);
    float fogStart = 50.0;
    float fogEnd = 250.0;
    float fogFactor = clamp((fogDistance - fogStart) / (fogEnd - fogStart), 0.0, 1.0);
    
    // Blend with fog
    lighting = mix(lighting, fogColor, fogFactor);
    
    FragColor = vec4(lighting, 1.0);
}
