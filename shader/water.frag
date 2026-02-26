#version 460

in VS_OUT {
    vec3 worldPos;
    vec3 normal;
    float depth;
} fs_in;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 cameraPos;
uniform float time;

layout(location = 0) out vec4 FragColor;

void main()
{
    // Water base color - semi-transparent blue
    vec3 waterColor = vec3(0.1, 0.4, 0.7);
    
    // Calculate light direction and reflection
    vec3 lightDir = normalize(lightPos - fs_in.worldPos);
    vec3 viewDir = normalize(cameraPos - fs_in.worldPos);
    vec3 normal = normalize(fs_in.normal);
    
    // Calculate reflection (Fresnel effect - more reflective at grazing angles)
    float fresnel = pow(1.0 - max(0.0, dot(viewDir, normal)), 3.0);
    fresnel = mix(0.1, 0.8, fresnel);  // Reflectivity between 10% and 80%
    
    // Light reflection on water surface
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(0.0, dot(viewDir, reflectDir)), 16.0);
    vec3 specular = lightColor * spec * 1.2;  // Increased for more shine
    
    // Basic diffuse lighting
    float diff = max(0.0, dot(normal, lightDir));
    vec3 diffuse = waterColor * lightColor * diff * 0.3;
    
    // Combine colors
    vec3 finalColor = waterColor * 0.5 + diffuse + specular;
    
    // Apply fresnel to control transparency
    float alpha = 0.5 + fresnel * 0.3;  // 50-80% opaque
    
    FragColor = vec4(finalColor, alpha);
}
