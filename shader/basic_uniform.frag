#version 460

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec3 Color;
    vec4 FragPosLightSpace;
    vec2 UV;
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
uniform vec3 lightColor;  // Color of the light source (day/night blended)
uniform vec3 fogColor;    // Fog color (matches sky)
uniform float lightIntensity;  // 0.0 = night, 1.0 = full daylight
uniform int applyLighting;
uniform bool renderingCoin;  // Flag to indicate coin rendering
uniform vec3 coinColor;  // Color for coin rendering

void main() {
    vec3 color = fs_in.Color;
    
    // If applyLighting is 0, just use light color (for sun/moon/skybox)
    if (applyLighting == 0) {
        FragColor = vec4(lightColor, 1.0);
        return;
    }
    
    // Sample and blend textures
    vec3 texColor = color;
    vec3 normal = normalize(fs_in.Normal);  // Default to vertex normal
    
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
            vec3 stoneNorm = texture(stoneNormal, fs_in.UV).rgb;
            texColor = mix(color, stoneTex, 0.85);
            // Convert normal from [0, 1] to [-1, 1] and perturb the vertex normal slightly
            normal = normalize(normal + (stoneNorm * 2.0 - 1.0) * 0.3);
        }
        // Green dominant (plains/grass) - green significantly higher than other channels
        else if (color.g > color.r + 0.1 && color.g > color.b + 0.1) {
            vec3 grassTex = texture(grassTexture, fs_in.UV).rgb;
            vec3 grassNorm = texture(grassNormal, fs_in.UV).rgb;
            texColor = mix(color, grassTex, 0.85);
            // Convert normal from [0, 1] to [-1, 1] and perturb the vertex normal slightly
            normal = normalize(normal + (grassNorm * 2.0 - 1.0) * 0.3);
        }
        // Low areas (browns/dark) - use dirt texture
        else {
            vec3 dirtTex = texture(dirtTexture, fs_in.UV).rgb;
            vec3 dirtNorm = texture(dirtNormal, fs_in.UV).rgb;
            texColor = mix(color, dirtTex, 0.85);
            // Convert normal from [0, 1] to [-1, 1] and perturb the vertex normal slightly
            normal = normalize(normal + (dirtNorm * 2.0 - 1.0) * 0.3);
        }
    }
    
    // Diffuse lighting
    vec3 norm = normalize(normal);
    vec3 sunDir = normalize(sunDirection);
    float diffuse = max(dot(norm, sunDir), 0.0);
    float minAmbient = 0.1 * lightIntensity;  // Ambient light varies with time of day
    diffuse = max(diffuse, minAmbient);
    
    // Blinn-Phong specular lighting
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 halfVec = normalize(sunDir + viewDir);
    float spec = pow(max(dot(norm, halfVec), 0.0), 32.0);
    float specular = renderingCoin ? spec * 0.8 : spec * 0.1;  // Higher specularity for shiny coins
    
    // Apply diffuse and specular lighting with light color
    vec3 diffuseLighting = texColor * diffuse * lightColor;
    vec3 specularLighting = vec3(1.0) * specular * lightColor;
    vec3 lighting = diffuseLighting + specularLighting;
    
    // Fog effect - more aggressive to be visible
    float fogDistance = length(viewPos - fs_in.FragPos);
    float fogStart = 50.0;
    float fogEnd = 250.0;
    float fogFactor = clamp((fogDistance - fogStart) / (fogEnd - fogStart), 0.0, 1.0);
    
    // Blend with fog
    lighting = mix(lighting, fogColor, fogFactor);
    
    FragColor = vec4(lighting, 1.0);
}
