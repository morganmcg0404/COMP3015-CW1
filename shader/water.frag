#version 460

in VS_OUT {
    vec3 worldPos;
    vec3 normal;
    float depth;
} fs_in;

uniform vec3 sunDirection;  // Normalized sun direction
uniform vec3 lightColor;
uniform vec3 cameraPos;
uniform float time;

layout(location = 0) out vec4 FragColor;

void main()
{
    // Water base color - semi-transparent blue
    vec3 waterColor = vec3(0.1, 0.4, 0.7);
    
    // Use the pre-normalized sun direction for consistent lighting across the plane
    vec3 lightDir = sunDirection;
    vec3 viewDir = normalize(cameraPos - fs_in.worldPos);
    
    // Use flat normal pointing straight up
    vec3 normal = vec3(0.0, 1.0, 0.0);
    
    // Calculate Fresnel effect
    float fresnel = pow(1.0 - max(0.0, dot(viewDir, normal)), 3.0);
    fresnel = mix(0.1, 0.8, fresnel);  // Reflectivity between 10% and 80%
    
    // Blinn-Phong specular lighting for water
    vec3 halfVec = normalize(lightDir + viewDir);
    float sunAboveHorizon = step(0.0, lightDir.y);  // 1.0 if above, 0.0 if below
    float spec = pow(max(0.0, dot(normal, halfVec)), 24.0);  // Broad highlight
    vec3 specular = lightColor * spec * 15.0 * sunAboveHorizon;
    
    // Basic diffuse lighting  
    float diff = max(0.0, dot(normal, lightDir)) * sunAboveHorizon;
    vec3 diffuse = waterColor * lightColor * diff * 0.4;
    
    // Combine colors
    vec3 finalColor = waterColor * 0.4 + diffuse + specular;
    
    // Alpha blending with fresnel
    float alpha = 0.6 + fresnel * 0.2;
    
    FragColor = vec4(finalColor, alpha);
}
