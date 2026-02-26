#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 MVP;
uniform mat4 ModelMatrix;
uniform float time;

out VS_OUT {
    vec3 worldPos;
    vec3 normal;
    float depth;
} vs_out;

void main()
{
    // Create wave animation based on world position and time
    vec3 pos = position;
    
    // Wave 1 - larger, slower waves (subtle)
    float wave1 = sin(pos.x * 0.1 + time * 0.5) * 0.05;
    wave1 += sin(pos.z * 0.08 + time * 0.4) * 0.05;
    
    // Wave 2 - smaller, faster waves (subtle)
    float wave2 = sin(pos.x * 0.3 + time * 1.2) * 0.02;
    wave2 += sin(pos.z * 0.25 + time * 1.1) * 0.02;
    
    // Combined wave height
    pos.y += wave1 + wave2;
    
    // Calculate depth (for shading - unused but can be useful)
    vs_out.depth = length(pos - vec3(0.0, 0.0, 0.0));
    
    // Transform position
    gl_Position = MVP * vec4(pos, 1.0);
    
    // Pass world position for lighting
    vs_out.worldPos = (ModelMatrix * vec4(pos, 1.0)).xyz;
    
    // Simple normal calculation from waves
    vec3 tangent = normalize(cross(normal, vec3(0.0, 1.0, 0.0)));
    vec3 bitangent = cross(normal, tangent);
    
    // Perturb normal based on waves (more subtle)
    vec3 perturbedNormal = normal;
    perturbedNormal += tangent * sin(pos.x * 0.1 + time * 0.5) * 0.03;
    perturbedNormal += bitangent * sin(pos.z * 0.1 + time * 0.5) * 0.03;
    
    vs_out.normal = normalize(perturbedNormal);
}
