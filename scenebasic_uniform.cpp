#include "scenebasic_uniform.h"

#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <string>
using std::string;

#include <iostream>
using std::cerr;
using std::endl;

#include "helper/glutils.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "helper/stb/stb_image.h"

using glm::vec3;

// Static member initialization
SceneBasic_Uniform* SceneBasic_Uniform::instancePtr = nullptr;

SceneBasic_Uniform::SceneBasic_Uniform() : angle(0.0f), indexCount(0) {
    cameraPos = vec3(0.0f, 3.0f, 5.0f);
    cameraFront = vec3(0.0f, -0.5f, -1.0f);
    cameraUp = vec3(0.0f, 1.0f, 0.0f);
    cameraSpeed = 5.0f;
    cameraSensitivity = 0.05f;
    smoothCameraHeight = 3.0f;  // Initialize smooth camera height
    
    // Player character
    playerPos = vec3(0.0f, 3.0f, 5.0f);
    playerYaw = -90.0f;
    playerPitch = 0.0f;
    playerSpeed = 5.0f;
    playerHeight = PLAYER_HEIGHT;
    isSprintingEnabled = false;
    
    // Player jump
    playerVerticalVelocity = 0.0f;
    isPlayerInAir = false;
    jumpPower = JUMP_POWER;
    gravity = GRAVITY;
    
    // Coins
    coinsCollected = 0;
    lastCoinCollectionTime = -1.0f;
    coinMesh = nullptr;
    coinTexture = 0;
    gameInitialized = false;
    gameCompleted = false;
    
    // Zoom variables
    isZooming = false;
    zoomLevel = 1.0f;  // Default zoom is 1x (normal, no zoom active)
    baseFOV = 45.0f;
    lastZoomMouseY = 0.0;
    scrollOffset = 0.0;
    rememberedZoomLevel = 2.0f;  // Remember 2x as default starting point
    
    // Vignette effect
    vignetteEnabled = true;  // Enable vignette by default
    vignetteIntensity = 0.4f;  // Slightly less intense
    
    // Mouse variables
    lastMouseX = 400.0; // Half of window width
    lastMouseY = 300.0; // Half of window height
    firstMouse = true;
    yaw = 0.0f;
    pitch = 0.0f;
    
    // Textures
    grassTexture = 0;
    stoneTexture = 0;
    dirtTexture = 0;
    firstMouse = true;
    yaw = -90.0f;
    pitch = -30.0f;
    
    // Day-night cycle starts at noon
    dayNightTime = 0.25f;  // 0.25 = noon (0=midnight, 0.5=noon from opposite side)
    
    // Set this instance as the active one for scroll callback
    instancePtr = this;
}

void SceneBasic_Uniform::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (instancePtr) {
        instancePtr->scrollOffset += yoffset;
    }
}

GLuint SceneBasic_Uniform::loadTexture(const char* path)
{
    // Try multiple possible paths
    const char* possiblePaths[] = {
        path,
        ("../../../" + std::string(path)).c_str(),
        ("../../" + std::string(path)).c_str(),
        ("../" + std::string(path)).c_str(),
    };
    
    int width, height, nrChannels;
    unsigned char* data = nullptr;
    const char* loadedPath = nullptr;
    
    for (const char* tryPath : possiblePaths) {
        data = stbi_load(tryPath, &width, &height, &nrChannels, 0);
        if (data) {
            loadedPath = tryPath;
            break;
        }
    }
    
    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        std::cerr << "  stbi error: " << stbi_failure_reason() << std::endl;
        return 0;
    }
    
    std::cout << "Successfully loaded texture: " << loadedPath << " (" << width << "x" << height << ", channels: " << nrChannels << ")" << std::endl;
    
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return texture;
}

GLuint SceneBasic_Uniform::createDefaultTexture()
{
    // Create a simple white 1x1 texture as default
    GLuint texture;
    unsigned char white[] = { 255, 255, 255 };
    
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, white);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return texture;
}

GLuint SceneBasic_Uniform::createDefaultNormalMap()
{
    // Create a default normal map (neutral blue pointing straight up)
    // Normal (0, 0, 1) is encoded as RGB (128, 128, 255)
    GLuint texture;
    unsigned char normal[] = { 128, 128, 255 };
    
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, normal);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return texture;
}

void SceneBasic_Uniform::initScene()
{
    compile();
    
    // Initialize random number generator with current time
    srand((unsigned int)time(nullptr));
    
    // Generate a random world seed - each load creates a different world
    uint32_t seed1 = (uint32_t)rand();
    uint32_t seed2 = (uint32_t)rand();
    uint32_t seed3 = (uint32_t)rand();
    worldSeed = seed1 ^ (seed2 << 10) ^ (seed3 << 20);
    
    // Load textures - use defaults if loading fails
    GLuint defaultTex = createDefaultTexture();
    std::cout << "Loading textures..." << std::endl;
    
    grassTexture = loadTexture("textures/grass.jpg");
    if (grassTexture == 0) {
        std::cout << "Grass texture not found, using default white texture" << std::endl;
        grassTexture = defaultTex;
    }
    
    stoneTexture = loadTexture("textures/stone.jpg");
    if (stoneTexture == 0) {
        std::cout << "Stone texture not found, using default white texture" << std::endl;
        stoneTexture = defaultTex;
    }
    
    dirtTexture = loadTexture("textures/dirt.jpg");
    if (dirtTexture == 0) {
        std::cout << "Dirt texture not found, using default white texture" << std::endl;
        dirtTexture = defaultTex;
    }
    
    // Load normal maps for terrain textures
    std::cout << "Loading normal maps..." << std::endl;
    grassNormalMap = loadTexture("textures/grass_normal.png");
    if (grassNormalMap == 0) {
        std::cout << "Grass normal map not found, using default" << std::endl;
        grassNormalMap = createDefaultNormalMap();
    }
    
    stoneNormalMap = loadTexture("textures/stone_normal.png");
    if (stoneNormalMap == 0) {
        std::cout << "Stone normal map not found, using default" << std::endl;
        stoneNormalMap = createDefaultNormalMap();
    }
    
    dirtNormalMap = loadTexture("textures/dirt_normal.png");
    if (dirtNormalMap == 0) {
        std::cout << "Dirt normal map not found, using default" << std::endl;
        dirtNormalMap = createDefaultNormalMap();
    }
    
    glClearColor(0.2f, 0.5f, 0.8f, 1.0f); // Sky blue
    glEnable(GL_DEPTH_TEST);

    createPlane();
    createSkybox();
    createSun();
    createWater();
    
    // Create procedurally generated coin
    coinMesh = new Coin(0.5f, 0.1f, 32);
    coinTexture = loadTexture("textures/gltf_embedded_0.png");
    if (coinTexture == 0) {
        std::cout << "Coin texture not found, using default gold color" << std::endl;
        coinTexture = 0;  // Will render with material color instead
    }
    spawnCoins();
    
    // Set player position above terrain at starting location
    float startTerrainHeight = getTerrainHeightAt(0.0f, 0.0f);
    playerPos = glm::vec3(0.0f, startTerrainHeight + playerHeight, 0.0f);
    cameraPos = playerPos;
    
    // Initialize matrices
    model = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 1000.0f);
    
    updateCamera();
    
    // Initialize text renderer
    textRenderer = new TextRenderer();
    // Try to load a system font - Windows fonts are typically in C:\Windows\Fonts
    if (!textRenderer->LoadFont("C:\\Windows\\Fonts\\arial.ttf", 48)) {
        std::cerr << "Failed to load font for text rendering" << std::endl;
    }
    
    gameInitialized = true;  // Game fully loaded, enable coin collection
}

void SceneBasic_Uniform::createPlane()
{
    // Initialize with center chunks
    lastChunkCoord = glm::ivec2(-999, -999);
    updateTerrainChunks();
    
    // Create UI quad for coin counter display
    createUIQuad();
}

void SceneBasic_Uniform::createUIQuad()
{
    // Create a proper square quad for UI (from -0.5 to 0.5 in both X and Y)
    float quadVertices[] = {
        // First triangle
        -0.5f,  0.5f, 0.0f,  // top-left
        -0.5f, -0.5f, 0.0f,  // bottom-left
         0.5f, -0.5f, 0.0f,  // bottom-right
        
        // Second triangle
        -0.5f,  0.5f, 0.0f,  // top-left
         0.5f, -0.5f, 0.0f,  // bottom-right
         0.5f,  0.5f, 0.0f   // top-right
    };
    
    GLuint VBO;
    glGenVertexArrays(1, &uiQuadVAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(uiQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void SceneBasic_Uniform::createTerrainChunk(int chunkX, int chunkZ)
{
    float chunkSize = 20.0f;
    float cellSize = 0.4f;
    int gridWidth = (int)(chunkSize / cellSize);
    int gridDepth = (int)(chunkSize / cellSize);
    
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> colors;
    std::vector<GLuint> indices;
    
    // First pass: generate height map
    std::vector<std::vector<float>> heightMap(gridWidth + 1, std::vector<float>(gridDepth + 1));
    
    for (int x = 0; x <= gridWidth; x++) {
        for (int z = 0; z <= gridDepth; z++) {
            float worldX = chunkX * chunkSize + x * cellSize;
            float worldZ = chunkZ * chunkSize + z * cellSize;
            
            // Smooth biome blending
            float biomeX = worldX / 100.0f;
            float biomeZ = worldZ / 100.0f;
            
            // Create a blend factor based on position within the biome region
            float blendX = fmod(biomeX, 1.0f);
            float blendZ = fmod(biomeZ, 1.0f);
            
            // Use smoothstep for smooth transitions (transition zone is 20% on each side)
            float transitionWidth = 0.2f;
            float blendFactorX = glm::smoothstep(0.0f, transitionWidth, blendX) * (1.0f - glm::smoothstep(1.0f - transitionWidth, 1.0f, blendX));
            float blendFactorZ = glm::smoothstep(0.0f, transitionWidth, blendZ) * (1.0f - glm::smoothstep(1.0f - transitionWidth, 1.0f, blendZ));
            
            // Combine blend factors - favor plains in transition zones
            float biomeBlend = glm::clamp(blendFactorX * blendFactorZ, 0.0f, 1.0f);
            
            // Use seed to create phase shifts for different terrain each load
            float seedPhase1 = (worldSeed % 1000) * 0.001f;
            float seedPhase2 = ((worldSeed >> 10) % 1000) * 0.001f;
            float seedScale = 0.5f + (worldSeed % 100) * 0.005f; // Scale between 0.5 and 1.0
            
            // Generate mountain terrain with seed-based variation
            float mountainHeight = 0.0f;
            mountainHeight += sin(worldX * 0.12f + seedPhase1) * 7.0f * seedScale + cos(worldZ * 0.14f + seedPhase2) * 7.0f * seedScale;
            mountainHeight += sin(worldX * 0.27f + seedPhase1 * 2.0f) * 3.5f * seedScale + cos(worldZ * 0.31f + seedPhase2 * 2.0f) * 3.5f * seedScale;
            mountainHeight += sin(worldX * 0.06f + seedPhase1 * 0.5f) * 4.5f * seedScale + cos(worldZ * 0.08f + seedPhase2 * 0.5f) * 4.5f * seedScale;
            mountainHeight += sin(worldX * 0.44f + seedPhase1 * 3.0f) * 2.0f * seedScale + cos(worldZ * 0.41f + seedPhase2 * 3.0f) * 2.0f * seedScale;
            mountainHeight = (mountainHeight + 26.0f) * 0.4f;
            
            // Generate plains terrain with seed-based variation
            float plainsHeight = 0.0f;
            plainsHeight += sin(worldX * 0.22f + seedPhase1) * 1.5f * seedScale + cos(worldZ * 0.25f + seedPhase2) * 1.5f * seedScale;
            plainsHeight += sin(worldX * 0.45f + seedPhase1 * 2.0f) * 0.8f * seedScale + cos(worldZ * 0.48f + seedPhase2 * 2.0f) * 0.8f * seedScale;
            plainsHeight += sin(worldX * 0.11f + seedPhase1 * 0.5f) * 1.2f * seedScale + cos(worldZ * 0.13f + seedPhase2 * 0.5f) * 1.2f * seedScale;
            plainsHeight += sin(worldX * 0.67f + seedPhase1 * 3.0f) * 0.5f * seedScale + cos(worldZ * 0.71f + seedPhase2 * 3.0f) * 0.5f * seedScale;
            plainsHeight = (plainsHeight + 8.0f) * 0.6f;
            
            // Blend heights based on position in biome region
            float height = glm::mix(plainsHeight, mountainHeight, biomeBlend);
            
            // Lake effect: create a depression offset into the plains biome
            float lakeCenter_X = -35.0f;  // Offset lake center into plains biome
            float lakeCenter_Z = -35.0f;
            float distanceFromLake = glm::length(glm::vec2(worldX - lakeCenter_X, worldZ - lakeCenter_Z));
            float lakeRadius = 20.0f;  // Radius of the lake
            float lakeEdgeFalloff = 12.0f;  // Distance for smooth edges (increased for gentler slope)
            float lakeFloor = 0.0f;  // Keep lake floor flat
            
            // Add subtle noise to lake radius for slightly irregular shape
            float angle = atan2(worldZ - lakeCenter_Z, worldX - lakeCenter_X);
            float noiseVal = sin(angle * 6.0f) * 0.08f + sin(angle * 2.5f) * 0.06f;
            float randomRadius = lakeRadius * (1.0f + noiseVal);
            
            if (distanceFromLake < randomRadius + lakeEdgeFalloff) {
                // Calculate lake depression
                float lakeDepth = 5.0f;  // How deep the lake is
                
                if (distanceFromLake < randomRadius) {
                    // Deep part of lake - flat floor with no islands
                    height = lakeFloor - lakeDepth;
                } else {
                    // Smooth edge falloff with gentler slope
                    float edgeDistance = distanceFromLake - randomRadius;
                    float edgeFactor = glm::clamp(edgeDistance / lakeEdgeFalloff, 0.0f, 1.0f);
                    // Use a smoother curve for more natural slope (cubic instead of quadratic)
                    float smoothFactor = edgeFactor * edgeFactor * (3.0f - 2.0f * edgeFactor);
                    // Smooth transition from lake floor to surrounding terrain
                    height = glm::mix(lakeFloor - lakeDepth, height, smoothFactor);
                }
            }
            
            // Island effect: apply falloff from center
            float distanceFromCenter = glm::length(glm::vec2(worldX, worldZ));
            float islandRadius = 150.0f;  // Radius of the island
            float falloffStart = 100.0f;  // Where falloff begins
            
            if (distanceFromCenter > falloffStart) {
                // Smooth falloff to make edges gradually sink into water
                float falloffDistance = distanceFromCenter - falloffStart;
                float falloffMax = islandRadius - falloffStart;
                float falloffFactor = glm::clamp(falloffDistance / falloffMax, 0.0f, 1.0f);
                
                // Apply smooth falloff (smoothstep for smooth edges)
                falloffFactor = falloffFactor * falloffFactor;  // Make it steeper
                
                // Lower the terrain height at edges
                height -= falloffFactor * 20.0f;
            }
            
            heightMap[x][z] = height;
        }
    }
    
    // Second pass: generate vertices and colors based on height and slope
    for (int x = 0; x <= gridWidth; x++) {
        for (int z = 0; z <= gridDepth; z++) {
            float worldX = chunkX * chunkSize + x * cellSize;
            float worldZ = chunkZ * chunkSize + z * cellSize;
            float height = heightMap[x][z];
            
            vertices.push_back(glm::vec3(worldX, height, worldZ));
            
            // Calculate slope by looking at neighboring heights
            float slope = 0.0f;
            int sampleCount = 0;
            
            for (int dx = -1; dx <= 1; dx++) {
                for (int dz = -1; dz <= 1; dz++) {
                    if (dx == 0 && dz == 0) continue;
                    int nx = x + dx;
                    int nz = z + dz;
                    if (nx >= 0 && nx <= gridWidth && nz >= 0 && nz <= gridDepth) {
                        float heightDiff = abs(heightMap[nx][nz] - height);
                        slope += heightDiff;
                        sampleCount++;
                    }
                }
            }
            slope /= sampleCount;
            
            // Determine biome for coloring with smooth blending
            float biomeX = worldX / 100.0f;
            float biomeZ = worldZ / 100.0f;
            float blendX = fmod(biomeX, 1.0f);
            float blendZ = fmod(biomeZ, 1.0f);
            
            float transitionWidth = 0.2f;
            float blendFactorX = glm::smoothstep(0.0f, transitionWidth, blendX) * (1.0f - glm::smoothstep(1.0f - transitionWidth, 1.0f, blendX));
            float blendFactorZ = glm::smoothstep(0.0f, transitionWidth, blendZ) * (1.0f - glm::smoothstep(1.0f - transitionWidth, 1.0f, blendZ));
            float biomeBlend = glm::clamp(blendFactorX * blendFactorZ, 0.0f, 1.0f);
            
            // Mountain biome colors
            glm::vec3 mountainColor;
            if (height < 5.0f) {
                mountainColor = glm::vec3(0.3f, 0.25f, 0.2f); // Dark brown
            } else if (height < 10.0f) {
                mountainColor = glm::vec3(0.4f, 0.35f, 0.25f); // Mid brown
            } else if (height < 15.0f) {
                mountainColor = glm::vec3(0.5f, 0.5f, 0.5f); // Gray
            } else {
                mountainColor = glm::vec3(0.8f, 0.8f, 0.8f); // Light gray/snow
            }
            
            // Plains biome colors
            glm::vec3 plainsColor;
            if (height < 2.0f) {
                plainsColor = glm::vec3(0.2f, 0.35f, 0.1f); // Dark green
            } else if (height < 4.0f) {
                plainsColor = glm::vec3(0.15f, 0.45f, 0.1f); // Medium green
            } else {
                plainsColor = glm::vec3(0.25f, 0.5f, 0.15f); // Light green
            }
            
            // Blend colors between biomes
            glm::vec3 color = glm::mix(plainsColor, mountainColor, biomeBlend);
            
            // Darken steep slopes
            float steepness = glm::clamp(slope * 2.0f, 0.0f, 1.0f);
            color *= (1.0f - steepness * 0.5f); // Darken by up to 50% for very steep slopes
            
            colors.push_back(color);
        }
    }
    
    // Third pass: calculate normals and tangents from height map
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> tangents;
    for (int x = 0; x <= gridWidth; x++) {
        for (int z = 0; z <= gridDepth; z++) {
            // Calculate normal using cross product of tangent vectors
            glm::vec3 tangentX(cellSize, 0.0f, 0.0f);
            glm::vec3 tangentZ(0.0f, 0.0f, cellSize);
            
            // Get height differences for slopes
            float hLeft = (x > 0) ? heightMap[x-1][z] : heightMap[x][z];
            float hRight = (x < gridWidth) ? heightMap[x+1][z] : heightMap[x][z];
            float hBack = (z > 0) ? heightMap[x][z-1] : heightMap[x][z];
            float hFront = (z < gridDepth) ? heightMap[x][z+1] : heightMap[x][z];
            
            tangentX.y = hRight - hLeft;
            tangentZ.y = hFront - hBack;
            
            glm::vec3 normal = glm::normalize(glm::cross(tangentZ, tangentX));
            normals.push_back(normal);
            
            // Tangent for terrain: mostly aligned with world X axis but adjusted for slope
            glm::vec3 tangent = glm::normalize(tangentX);
            tangents.push_back(tangent);
        }
    }
    
    // Fourth pass: generate UV coordinates based on height and biome
    std::vector<glm::vec2> uvs;
    for (int x = 0; x <= gridWidth; x++) {
        for (int z = 0; z <= gridDepth; z++) {
            float worldX = chunkX * chunkSize + x * cellSize;
            float worldZ = chunkZ * chunkSize + z * cellSize;
            float height = heightMap[x][z];
            
            // Create repeating UV coordinates (scale them so textures repeat nicely)
            float u = fmod(worldX * 0.25f, 1.0f);
            float v = fmod(worldZ * 0.25f, 1.0f);
            
            uvs.push_back(glm::vec2(u, v));
        }
    }
    
    // Generate indices
    for (int x = 0; x < gridWidth; x++) {
        for (int z = 0; z < gridDepth; z++) {
            int a = z * (gridWidth + 1) + x;
            int b = a + 1;
            int c = a + (gridWidth + 1);
            int d = c + 1;
            
            indices.push_back(a);
            indices.push_back(c);
            indices.push_back(b);
            
            indices.push_back(b);
            indices.push_back(c);
            indices.push_back(d);
        }
    }
    
    TerrainChunk chunk;
    chunk.position = glm::vec3(chunkX * chunkSize, 0.0f, chunkZ * chunkSize);
    chunk.indexCount = (GLuint)indices.size();
    
    GLuint positionVBO;
    glGenBuffers(1, &positionVBO);
    glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    
    GLuint colorVBO;
    glGenBuffers(1, &colorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_STATIC_DRAW);
    
    GLuint normalVBO;
    glGenBuffers(1, &normalVBO);
    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
    
    GLuint uvVBO;
    glGenBuffers(1, &uvVBO);
    glBindBuffer(GL_ARRAY_BUFFER, uvVBO);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_STATIC_DRAW);
    
    GLuint tangentVBO;
    glGenBuffers(1, &tangentVBO);
    glBindBuffer(GL_ARRAY_BUFFER, tangentVBO);
    glBufferData(GL_ARRAY_BUFFER, tangents.size() * sizeof(glm::vec3), tangents.data(), GL_STATIC_DRAW);
    
    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
    
    glGenVertexArrays(1, &chunk.vao);
    glBindVertexArray(chunk.vao);
    
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, uvVBO);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    
    glEnableVertexAttribArray(4);
    glBindBuffer(GL_ARRAY_BUFFER, tangentVBO);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    
    glBindVertexArray(0);
    
    terrainChunks[{chunkX, chunkZ}] = chunk;
}

void SceneBasic_Uniform::updateTerrainChunks()
{
    float chunkSize = 20.0f;
    float islandRadius = 150.0f;  // Island radius
    float islandFalloff = 100.0f;  // Falloff distance
    float maxLoadDistance = islandRadius + islandFalloff + 20.0f;  // Only load chunks near island
    
    // Get current chunk coordinates based on player position
    glm::ivec2 currentChunk = glm::ivec2(
        (int)floor(playerPos.x / chunkSize),
        (int)floor(playerPos.z / chunkSize)
    );
    
    // If camera moved to a different chunk
    if (currentChunk != lastChunkCoord) {
        lastChunkCoord = currentChunk;
        
        // Only generate chunks within the island area
        for (int x = currentChunk.x - 10; x <= currentChunk.x + 10; x++) {
            for (int z = currentChunk.y - 10; z <= currentChunk.y + 10; z++) {
                // Check if chunk center is within island loading distance
                float chunkCenterX = (x + 0.5f) * chunkSize;
                float chunkCenterZ = (z + 0.5f) * chunkSize;
                float distToIsland = sqrt(chunkCenterX * chunkCenterX + chunkCenterZ * chunkCenterZ);
                
                if (distToIsland <= maxLoadDistance) {
                    if (terrainChunks.find({x, z}) == terrainChunks.end()) {
                        createTerrainChunk(x, z);
                    }
                }
            }
        }
        
        // Remove far chunks and cleanup their GPU resources
        std::vector<std::pair<int, int>> toRemove;
        for (auto& chunk : terrainChunks) {
            float chunkCenterX = (chunk.first.first + 0.5f) * chunkSize;
            float chunkCenterZ = (chunk.first.second + 0.5f) * chunkSize;
            float distToIsland = sqrt(chunkCenterX * chunkCenterX + chunkCenterZ * chunkCenterZ);
            
            // Keep chunks within island loading distance
            if (distToIsland > maxLoadDistance + 10.0f) {  // 10 unit safety margin
                toRemove.push_back(chunk.first);
            }
        }
        
        for (auto& key : toRemove) {
            // Delete GPU resources before removing from map
            auto& chunk = terrainChunks[key];
            glDeleteVertexArrays(1, &chunk.vao);  // Deletes VAO and its associated VBOs
            terrainChunks.erase(key);
        }
    }
}

void SceneBasic_Uniform::createSkybox()
{
    // Create a large cube for the skybox - make it bigger to avoid seeing seams
    float size = 500.0f; // Increased to prevent seams
    
    std::vector<glm::vec3> vertices = {
        // Front face
        glm::vec3(-size, -size, size), glm::vec3(size, -size, size), glm::vec3(size, size, size), glm::vec3(-size, size, size),
        // Back face
        glm::vec3(-size, -size, -size), glm::vec3(-size, size, -size), glm::vec3(size, size, -size), glm::vec3(size, -size, -size),
        // Top face
        glm::vec3(-size, size, -size), glm::vec3(-size, size, size), glm::vec3(size, size, size), glm::vec3(size, size, -size),
        // Bottom face
        glm::vec3(-size, -size, -size), glm::vec3(size, -size, -size), glm::vec3(size, -size, size), glm::vec3(-size, -size, size),
        // Right face
        glm::vec3(size, -size, -size), glm::vec3(size, size, -size), glm::vec3(size, size, size), glm::vec3(size, -size, size),
        // Left face
        glm::vec3(-size, -size, -size), glm::vec3(-size, -size, size), glm::vec3(-size, size, size), glm::vec3(-size, size, -size)
    };
    
    std::vector<glm::vec3> colors(vertices.size(), glm::vec3(0.5f, 0.7f, 0.9f)); // Lighter sky blue
    
    std::vector<GLuint> indices = {
        // Front
        0, 1, 2, 0, 2, 3,
        // Back
        4, 6, 5, 4, 7, 6,
        // Top
        8, 9, 10, 8, 10, 11,
        // Bottom
        12, 13, 14, 12, 14, 15,
        // Right
        16, 17, 18, 16, 18, 19,
        // Left
        20, 21, 22, 20, 22, 23
    };
    
    skyboxIndexCount = (GLuint)indices.size();
    
    GLuint positionVBO;
    glGenBuffers(1, &positionVBO);
    glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    
    GLuint colorVBO;
    glGenBuffers(1, &colorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_STATIC_DRAW);
    
    GLuint skyboxEBO;
    glGenBuffers(1, &skyboxEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
    
    glGenVertexArrays(1, &skyboxVAO);
    glBindVertexArray(skyboxVAO);
    
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
    
    glBindVertexArray(0);
}

void SceneBasic_Uniform::createSun()
{
    // Create a sphere for the sun
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> colors;
    std::vector<GLuint> indices;
    
    const int stacks = 20;
    const int slices = 20;
    float radius = 5.0f;
    
    for (int i = 0; i <= stacks; ++i) {
        float phi = glm::pi<float>() * i / stacks;
        for (int j = 0; j <= slices; ++j) {
            float theta = 2.0f * glm::pi<float>() * j / slices;
            
            float x = radius * sin(phi) * cos(theta);
            float y = radius * cos(phi);
            float z = radius * sin(phi) * sin(theta);
            
            vertices.push_back(glm::vec3(x, y, z));
            colors.push_back(glm::vec3(1.0f, 1.0f, 0.0f)); // Bright yellow
        }
    }
    
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int a = i * (slices + 1) + j;
            int b = a + slices + 1;
            
            indices.push_back(a);
            indices.push_back(b);
            indices.push_back(a + 1);
            
            indices.push_back(b);
            indices.push_back(b + 1);
            indices.push_back(a + 1);
        }
    }
    
    sunIndexCount = (GLuint)indices.size();
    
    GLuint positionVBO;
    glGenBuffers(1, &positionVBO);
    glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    
    GLuint colorVBO;
    glGenBuffers(1, &colorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_STATIC_DRAW);
    
    GLuint sunEBO;
    glGenBuffers(1, &sunEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sunEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
    
    glGenVertexArrays(1, &sunVAO);
    glBindVertexArray(sunVAO);
    
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sunEBO);
    
    glBindVertexArray(0);
}

void SceneBasic_Uniform::createWater()
{
    // Create a large flat grid for water surface with many vertices for wave animation
    waterLevel = WATER_LEVEL;
    
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<GLuint> indices;
    
    const int gridSize = 100;  // 100x100 grid for smooth waves
    const float scale = 10.0f;  // Spacing between vertices
    const float totalSize = gridSize * scale;
    const float startX = -totalSize / 2.0f;
    const float startZ = -totalSize / 2.0f;
    
    // Generate grid vertices
    for (int z = 0; z <= gridSize; ++z) {
        for (int x = 0; x <= gridSize; ++x) {
            float posX = startX + x * scale;
            float posZ = startZ + z * scale;
            vertices.push_back(glm::vec3(posX, waterLevel, posZ));
            normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));  // Flat normal initially
        }
    }
    
    // Generate indices for triangle mesh
    for (int z = 0; z < gridSize; ++z) {
        for (int x = 0; x < gridSize; ++x) {
            int a = z * (gridSize + 1) + x;
            int b = a + 1;
            int c = a + (gridSize + 1);
            int d = c + 1;
            
            // First triangle
            indices.push_back(a);
            indices.push_back(c);
            indices.push_back(b);
            
            // Second triangle
            indices.push_back(b);
            indices.push_back(c);
            indices.push_back(d);
        }
    }
    
    waterIndexCount = (GLuint)indices.size();
    
    // Create VBOs
    GLuint positionVBO;
    glGenBuffers(1, &positionVBO);
    glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    
    GLuint normalVBO;
    glGenBuffers(1, &normalVBO);
    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
    
    GLuint waterEBO;
    glGenBuffers(1, &waterEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, waterEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
    
    // Create VAO
    glGenVertexArrays(1, &waterVAO);
    glBindVertexArray(waterVAO);
    
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, waterEBO);
    
    glBindVertexArray(0);
}

float SceneBasic_Uniform::seededNoise(float x, float y, uint32_t seed)
{
    // Simple hash-based noise function using seed
    uint32_t n = seed;
    n = (n << 13) ^ n;
    n = (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff;
    
    uint32_t hx = (uint32_t)(x * 73856093) ^ (uint32_t)(y * 19349663) ^ seed;
    hx = (hx << 13) ^ hx;
    hx = (hx * (hx * hx * 15731 + 789221) + 1376312589) & 0x7fffffff;
    
    return 2.0f * (hx / 2147483647.0f) - 1.0f; // Return value in [-1, 1]
}

void SceneBasic_Uniform::spawnCoins()
{
	coins.clear();
	coinsCollected = 0;
	spawnSingleCoin();
}

void SceneBasic_Uniform::spawnSingleCoin()
{
	float x, z;
	
	// First coin spawns in front of the player
	if (coinsCollected == 0) {
		// Place coin 15 units in front of player in the direction they're looking
		glm::vec3 frontDirection = glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
		glm::vec3 coinSpawnPos = playerPos + frontDirection * 15.0f;
		x = coinSpawnPos.x;
		z = coinSpawnPos.z;
	} else {
		// Spawn at random location on the island
		float angle = (rand() / (float)RAND_MAX) * glm::two_pi<float>();
		float radius = 50.0f + (rand() % 40);  // Radius between 50-90 units
		
		x = cos(angle) * radius;
		z = sin(angle) * radius;
	}
	
	float terrainHeight = getTerrainHeightAt(x, z);
	
	// Keep trying until we find a spot on solid ground (not water)
	int attempts = 0;
	while (terrainHeight < WATER_LEVEL && attempts < 10) {
		float angle = (rand() / (float)RAND_MAX) * glm::two_pi<float>();
		float radius = 50.0f + (rand() % 40);
		x = cos(angle) * radius;
		z = sin(angle) * radius;
		terrainHeight = getTerrainHeightAt(x, z);
		attempts++;
	}
	
	CoinInstance coin;
	coin.position = glm::vec3(x, terrainHeight + 2.0f, z);  // 2 units above terrain
	coin.collected = false;
	coins.push_back(coin);
}

void SceneBasic_Uniform::compile()
{
	try {
		// Register scroll callback
		GLFWwindow* window = glfwGetCurrentContext();
		if (window) {
			glfwSetScrollCallback(window, scrollCallback);
		}
		
		prog.compileShader("../../shader/basic_uniform.vert");
		prog.compileShader("../../shader/basic_uniform.frag");
		prog.link();
		prog.use();
		
		// Compile normal mapping shaders
		progNormalMap.compileShader("../../shader/basic_uniform_normalmap.vert");
		progNormalMap.compileShader("../../shader/basic_uniform_normalmap.frag");
		progNormalMap.link();
		
		// Compile shadow shaders
		shadowProg.compileShader("../../shader/shadow.vert");
		shadowProg.compileShader("../../shader/shadow.frag");
		shadowProg.link();
		
		// Compile bloom shaders
		bloomThresholdProg.compileShader("../../shader/bloom_pass.vert");
		bloomThresholdProg.compileShader("../../shader/bloom_threshold.frag");
		bloomThresholdProg.link();
		
		blurProg.compileShader("../../shader/bloom_pass.vert");
		blurProg.compileShader("../../shader/bloom_blur.frag");
		blurProg.link();
		
		bloomBlendProg.compileShader("../../shader/bloom_pass.vert");
		bloomBlendProg.compileShader("../../shader/bloom_blend.frag");
		bloomBlendProg.link();
		
		// Compile tone mapping shader for HDR
		toneMappingProg.compileShader("../../shader/tonemap.vert");
		toneMappingProg.compileShader("../../shader/tonemap.frag");
		toneMappingProg.link();
		
		// Compile vignette shader for framebuffer effect
		vignetteProg.compileShader("../../shader/vignette.vert");
		vignetteProg.compileShader("../../shader/vignette.frag");
		vignetteProg.link();
		
		// Compile water shader
		waterProg.compileShader("../../shader/water.vert");
		waterProg.compileShader("../../shader/water.frag");
		waterProg.link();
		
		// Compile text shader
		textProg.compileShader("../../shader/text.vert");
		textProg.compileShader("../../shader/text.frag");
		textProg.link();
		
		// Setup scene framebuffer (render scene to texture)
		glGenFramebuffers(1, &sceneFBO);
		glGenTextures(1, &sceneTexture);
		glBindTexture(GL_TEXTURE_2D, sceneTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneTexture, 0);
		
		// Add depth renderbuffer
		glGenRenderbuffers(1, &sceneDepthRBO);
		glBindRenderbuffer(GL_RENDERBUFFER, sceneDepthRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, sceneDepthRBO);
		
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cerr << "Scene framebuffer is not complete!" << std::endl;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		// Setup bloom texture
		glGenFramebuffers(1, &bloomFBO);
		glGenTextures(1, &bloomTexture);
		glBindTexture(GL_TEXTURE_2D, bloomTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloomTexture, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		// Setup blur texture
		glGenFramebuffers(1, &blurFBO);
		glGenTextures(1, &blurTexture);
		glBindTexture(GL_TEXTURE_2D, blurTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindFramebuffer(GL_FRAMEBUFFER, blurFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurTexture, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		// Setup HDR framebuffer for tone mapping
		glGenFramebuffers(1, &hdrFBO);
		glGenTextures(1, &hdrColorTexture);
		glBindTexture(GL_TEXTURE_2D, hdrColorTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrColorTexture, 0);
		
		// Add depth renderbuffer to HDR framebuffer
		glGenRenderbuffers(1, &hdrDepthRBO);
		glBindRenderbuffer(GL_RENDERBUFFER, hdrDepthRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, hdrDepthRBO);
		
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cerr << "HDR framebuffer is not complete!" << std::endl;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		// Setup final framebuffer for vignette post-processing
		glGenFramebuffers(1, &finalFBO);
		glGenTextures(1, &finalTexture);
		glBindTexture(GL_TEXTURE_2D, finalTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindFramebuffer(GL_FRAMEBUFFER, finalFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, finalTexture, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		// Initialize HDR state
		hdrEnabled = true;
		
		// Create screen quad for bloom post-processing
		float quadVertices[] = {
			-1.0f,  1.0f,  0.0f, 1.0f,
			-1.0f, -1.0f,  0.0f, 0.0f,
			 1.0f,  1.0f,  1.0f, 1.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,
		};
		
		GLuint quadVBO;
		glGenVertexArrays(1, &bloomQuadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(bloomQuadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		
	} catch (GLSLProgramException &e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
}

float SceneBasic_Uniform::getTerrainHeightAt(float x, float z)
{
	// Sample terrain height at world position (x, z)
	// This uses simplified procedural generation to match createTerrainChunk
	
	// Get biome blend
	float biomeX = x / 100.0f;
	float biomeZ = z / 100.0f;
	float blendX = fmod(biomeX, 1.0f);
	float blendZ = fmod(biomeZ, 1.0f);
	float transitionWidth = 0.2f;
	float blendFactorX = glm::smoothstep(0.0f, transitionWidth, blendX) * (1.0f - glm::smoothstep(1.0f - transitionWidth, 1.0f, blendX));
	float blendFactorZ = glm::smoothstep(0.0f, transitionWidth, blendZ) * (1.0f - glm::smoothstep(1.0f - transitionWidth, 1.0f, blendZ));
	float biomeBlend = glm::clamp(blendFactorX * blendFactorZ, 0.0f, 1.0f);
	
	// Use seed for noise generation
	float seedPhase1 = (worldSeed % 1000) * 0.001f;
	float seedPhase2 = ((worldSeed >> 10) % 1000) * 0.001f;
	float seedScale = 0.5f + (worldSeed % 100) * 0.005f;
	
	// Mountain terrain
	float mountainHeight = 0.0f;
	mountainHeight += sin(x * 0.12f + seedPhase1) * 7.0f * seedScale + cos(z * 0.14f + seedPhase2) * 7.0f * seedScale;
	mountainHeight += sin(x * 0.27f + seedPhase1 * 2.0f) * 3.5f * seedScale + cos(z * 0.31f + seedPhase2 * 2.0f) * 3.5f * seedScale;
	mountainHeight += sin(x * 0.06f + seedPhase1 * 0.5f) * 4.5f * seedScale + cos(z * 0.08f + seedPhase2 * 0.5f) * 4.5f * seedScale;
	mountainHeight += sin(x * 0.44f + seedPhase1 * 3.0f) * 2.0f * seedScale + cos(z * 0.41f + seedPhase2 * 3.0f) * 2.0f * seedScale;
	mountainHeight = (mountainHeight + 26.0f) * 0.4f;
	
	// Plains terrain
	float plainsHeight = 0.0f;
	plainsHeight += sin(x * 0.22f + seedPhase1) * 1.5f * seedScale + cos(z * 0.25f + seedPhase2) * 1.5f * seedScale;
	plainsHeight += sin(x * 0.45f + seedPhase1 * 2.0f) * 0.8f * seedScale + cos(z * 0.48f + seedPhase2 * 2.0f) * 0.8f * seedScale;
	plainsHeight += sin(x * 0.11f + seedPhase1 * 0.5f) * 1.2f * seedScale + cos(z * 0.13f + seedPhase2 * 0.5f) * 1.2f * seedScale;
	plainsHeight += sin(x * 0.67f + seedPhase1 * 3.0f) * 0.5f * seedScale + cos(z * 0.71f + seedPhase2 * 3.0f) * 0.5f * seedScale;
	plainsHeight = (plainsHeight + 8.0f) * 0.6f;
	
	// Blend heights
	float height = glm::mix(plainsHeight, mountainHeight, biomeBlend);
	
	// Lake effect
	float lakeCenter_X = -35.0f;
	float lakeCenter_Z = -35.0f;
	float distanceFromLake = glm::length(glm::vec2(x - lakeCenter_X, z - lakeCenter_Z));
	float lakeRadius = 20.0f;
	float lakeEdgeFalloff = 8.0f;
	float lakeFloor = 0.0f;
	
	float angle = atan2(z - lakeCenter_Z, x - lakeCenter_X);
	float noiseVal = sin(angle * 6.0f) * 0.08f + sin(angle * 2.5f) * 0.06f;
	float randomRadius = lakeRadius * (1.0f + noiseVal);
	
	if (distanceFromLake < randomRadius + lakeEdgeFalloff) {
		float lakeDepth = 5.0f;
		if (distanceFromLake < randomRadius) {
			height = lakeFloor - lakeDepth;
		} else {
			float edgeDistance = distanceFromLake - randomRadius;
			float edgeFactor = glm::clamp(edgeDistance / lakeEdgeFalloff, 0.0f, 1.0f);
			height = glm::mix(lakeFloor - lakeDepth, height, edgeFactor * edgeFactor);
		}
	}
	
	// Island effect
	float distanceFromCenter = glm::length(glm::vec2(x, z));
	float islandRadius = 150.0f;
	float falloffStart = 100.0f;
	
	if (distanceFromCenter > falloffStart) {
		float falloffDistance = distanceFromCenter - falloffStart;
		float falloffMax = islandRadius - falloffStart;
		float falloffFactor = glm::clamp(falloffDistance / falloffMax, 0.0f, 1.0f);
		falloffFactor = falloffFactor * falloffFactor;
		height -= falloffFactor * 20.0f;
	}
	
	// Clamp terrain to water level outside lake
	const float WATER_LEVEL_MIN = 0.5f;
	if (distanceFromLake >= randomRadius + lakeEdgeFalloff) {
		height = glm::max(height, WATER_LEVEL_MIN);
	} else if (distanceFromLake >= randomRadius) {
		height = glm::min(height, WATER_LEVEL_MIN - 0.1f);
	}
	
	return height;
}

void SceneBasic_Uniform::update( float t )
{
	angle = t;
	model = glm::mat4(1.0f); // Static plane, no rotation
	
	// Update day-night cycle (1 full cycle every 30 seconds for faster testing)
	dayNightTime = fmod(t / 30.0f, 1.0f);
	
	// Apply jump physics
	float deltaTime = 0.016f; // Approximate 60 FPS
	float waterCollisionHeight = WATER_LEVEL + 0.5f;  // Player top touches water at this height
	
	if (isPlayerInAir) {
		playerVerticalVelocity -= gravity * deltaTime;
		playerPos.y += playerVerticalVelocity * deltaTime;
		
		// Check collision with water - bounce effect
		if (playerPos.y <= waterCollisionHeight && playerVerticalVelocity < 0.0f) {
			// Bounce off water with reduced velocity
			playerVerticalVelocity = -playerVerticalVelocity * 0.6f;  // 60% bounce damping
			playerPos.y = waterCollisionHeight;  // Keep player at water surface
		}
		
		// Check collision with ground
		float terrainHeight = getTerrainHeightAt(playerPos.x, playerPos.z);
		float groundLevel = terrainHeight + playerHeight;
		
		if (playerPos.y <= groundLevel) {
			playerPos.y = groundLevel;
			playerVerticalVelocity = 0.0f;
			isPlayerInAir = false;
		}
	} else {
		// When walking normally, check if we've walked into water
		if (playerPos.y <= waterCollisionHeight) {
			// Player is in water, float on surface
			playerPos.y = waterCollisionHeight;
			playerVerticalVelocity = 0.0f;
			isPlayerInAir = true;  // Enable air state to allow bouncing/jumping out
		}
	}
	
	// Check for coin collection
	if (gameInitialized && !coins.empty()) {
		CoinInstance& coin = coins[0];
		float distanceToCoin = glm::distance(playerPos, coin.position);
		
		if (distanceToCoin < COIN_PICKUP_RADIUS) {
			// Check cooldown to prevent multiple collections per frame
			if (glfwGetTime() - lastCoinCollectionTime >= COIN_COLLECTION_COOLDOWN) {
				coinsCollected++;
				lastCoinCollectionTime = glfwGetTime();
				std::cout << "Coin collected! Total coins: " << coinsCollected << std::endl;
				
				// Check if game is complete
				if (coinsCollected >= 10) {
					gameCompleted = true;
					coins.clear();  // Delete the last coin
					std::cout << "=== GAME COMPLETE! ===" << std::endl;
					std::cout << "You collected all 10 coins!" << std::endl;
				} else {
					coins.clear();  // Remove the collected coin
					spawnSingleCoin();  // Spawn new coin at random location
				}
			}
		}
	}
	
	handleInput();
	updateCamera();
	updateTerrainChunks(); // Update loaded chunks based on camera position
}

void SceneBasic_Uniform::handleInput()
{
    // Disable input if game is completed
    if (gameCompleted) {
        return;
    }
    // Disable input if game is completed
    if (gameCompleted) {
        return;
    }
	// Get the GLFW window
	GLFWwindow* window = glfwGetCurrentContext();
	if (!window) return;

	float moveDistance = playerSpeed * 0.016f; // Approximate 60 FPS delta time

	// H key to toggle HDR
	static bool hKeyPressed = false;
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
		if (!hKeyPressed) {
			hdrEnabled = !hdrEnabled;
			std::cout << "HDR " << (hdrEnabled ? "ENABLED" : "DISABLED") << std::endl;
			hKeyPressed = true;
		}
	} else {
		hKeyPressed = false;
	}
	
	// V key to toggle vignette effect
	static bool vKeyPressed = false;
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
		if (!vKeyPressed) {
			vignetteEnabled = !vignetteEnabled;
			std::cout << "Vignette " << (vignetteEnabled ? "ENABLED" : "DISABLED") << std::endl;
			vKeyPressed = true;
		}
	} else {
		vKeyPressed = false;
	}

	// Z key to toggle zoom mode with scroll wheel
	static bool prevZKeyPressed = false;
	bool zKeyPressed = (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS);
	
	if (zKeyPressed) {
		// When Z is first pressed, restore the remembered zoom level
		if (!prevZKeyPressed) {
			zoomLevel = rememberedZoomLevel;  // Use remembered zoom or 2x as default
			scrollOffset = 0.0;  // Reset scroll offset
		}
		
		isZooming = true;
		
		// Update zoom based on scroll wheel input
		if (scrollOffset != 0.0) {
			zoomLevel += scrollOffset * 0.5f;
			zoomLevel = glm::clamp(zoomLevel, 1.0f, 5.0f);  // Min 1x, max 5x
			std::cout << "Zoom level: " << zoomLevel << "x" << std::endl;
			scrollOffset = 0.0;  // Reset scroll offset after processing
		}
	} else {
		// Z key released - save current zoom and reset to 1x
		if (prevZKeyPressed) {
			rememberedZoomLevel = zoomLevel;  // Remember current zoom level
		}
		isZooming = false;
		zoomLevel = 1.0f;  // Reset zoom to 1x when Z is released
		scrollOffset = 0.0;  // Reset scroll when not zooming
	}
	
	prevZKeyPressed = zKeyPressed;

	// Shift key to sprint
	isSprintingEnabled = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || 
	                       glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

	// WASD movement - move player character
	glm::vec3 moveDir(0.0f);
	
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		moveDir += glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		moveDir -= glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		glm::vec3 right = glm::normalize(glm::cross(glm::vec3(cameraFront.x, 0.0f, cameraFront.z), cameraUp));
		moveDir -= right;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		glm::vec3 right = glm::normalize(glm::cross(glm::vec3(cameraFront.x, 0.0f, cameraFront.z), cameraUp));
		moveDir += right;
	}

	// Apply sprint multiplier if enabled
	float currentMoveDistance = moveDistance;
	if (isSprintingEnabled && glm::length(moveDir) > 0.0f) {
		currentMoveDistance *= SPRINT_MULTIPLIER;
	}

	// Normalize movement direction to prevent faster diagonal movement
	if (glm::length(moveDir) > 0.0f) {
		moveDir = glm::normalize(moveDir);
		glm::vec3 newPos = playerPos + moveDir * currentMoveDistance;
		
		// Only apply terrain height collision when on ground
		if (!isPlayerInAir) {
			// Get terrain height at new position and place player on ground
			float terrainHeight = getTerrainHeightAt(newPos.x, newPos.z);
			newPos.y = terrainHeight + playerHeight;
		} else {
			// In air - keep the horizontal movement but preserve vertical position
			newPos.y = playerPos.y;
		}
		
		playerPos = newPos;
	}

	// Space key to jump
	static bool prevSpacePressed = false;
	bool spacePressed = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
	
	if (spacePressed && !prevSpacePressed && !isPlayerInAir) {
		playerVerticalVelocity = jumpPower;
		isPlayerInAir = true;
	}
	
	prevSpacePressed = spacePressed;

	// Mouse look
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	if (firstMouse) {
		lastMouseX = mouseX;
		lastMouseY = mouseY;
		firstMouse = false;
	}

	double deltaX = mouseX - lastMouseX;
	double deltaY = lastMouseY - mouseY;

	lastMouseX = mouseX;
	lastMouseY = mouseY;

	playerYaw += (float)deltaX * cameraSensitivity;
	playerPitch += (float)deltaY * cameraSensitivity;

	// Clamp pitch to prevent camera flip
	if (playerPitch > 89.0f) playerPitch = 89.0f;
	if (playerPitch < -89.0f) playerPitch = -89.0f;

	// Update front vector based on yaw and pitch
	glm::vec3 direction;
	direction.x = cos(glm::radians(playerYaw)) * cos(glm::radians(playerPitch));
	direction.y = sin(glm::radians(playerPitch));
	direction.z = sin(glm::radians(playerYaw)) * cos(glm::radians(playerPitch));
	cameraFront = glm::normalize(direction);
}

void SceneBasic_Uniform::updateCamera()
{
	// Smooth camera height transitions for natural movement on slopes
	float deltaTime = 0.016f;  // Approximate 60 FPS
	float targetCameraHeight = playerPos.y;
	
	// Interpolate camera height smoothly towards target
	smoothCameraHeight = glm::mix(smoothCameraHeight, targetCameraHeight, CAMERA_SMOOTH_SPEED * deltaTime);
	
	// Position camera at player position (first-person view) with smoothed Y
	cameraPos = glm::vec3(playerPos.x, smoothCameraHeight, playerPos.z);
	
	// Camera looks in the direction player is facing
	glm::vec3 center = cameraPos + cameraFront;
	view = glm::lookAt(cameraPos, center, cameraUp);
}

void SceneBasic_Uniform::render()
{
    // Calculate time-based values for day-night cycle
    // Use full circle (0-2π) for complete sun rotation: 0=midnight, π/2=sunrise, π=noon, 3π/2=sunset, 2π=midnight
    float sunAngle = dayNightTime * glm::two_pi<float>();
    float sunHeight = sin(sunAngle);
    
    // Position sun in circular arc (east to west, up and down)
    float sunDistance = 500.0f;
    glm::vec3 lightPos = glm::vec3(
        cameraPos.x + cos(sunAngle) * sunDistance,
        cameraPos.y + sunHeight * sunDistance,
        cameraPos.z + sin(sunAngle) * sunDistance * 0.5f
    );
    
    // Calculate lighting and sky color based on sun height
    glm::vec3 lightColor;
    glm::vec3 skyColor;
    
    if (sunHeight > -0.1f) {
        // Day time - bright
        if (sunHeight > 0.2f) {
            lightColor = glm::vec3(1.0f, 0.95f, 0.8f) * 1.0f;  // Full brightness warm light
        } else {
            // Transition near horizon
            float transitionProgress = (sunHeight + 0.1f) / 0.3f;
            float intensity = 0.3f + transitionProgress * 0.7f;
            lightColor = glm::vec3(1.0f, 0.95f, 0.8f) * intensity;
        }
        
        // Day sky - bright blue
        float dayProgress = std::max(0.0f, (sunHeight + 0.1f) / 0.3f);
        dayProgress = std::min(1.0f, dayProgress);
        skyColor = glm::mix(
            glm::vec3(0.4f, 0.45f, 0.6f),   // Twilight
            glm::vec3(0.68f, 0.85f, 0.9f),   // Bright blue
            dayProgress
        );
    } else {
        // Night time - dim
        float nightProgress = std::min(1.0f, (fabs(sunHeight) - 0.1f) / 0.15f);
        float lightIntensity = 0.15f + (1.0f - nightProgress) * 0.15f;
        lightColor = glm::vec3(0.6f, 0.7f, 1.0f) * lightIntensity;  // Cool night light
        
        // Night sky - very dark
        skyColor = glm::mix(
            glm::vec3(0.4f, 0.45f, 0.6f),   // Twilight
            glm::vec3(0.02f, 0.02f, 0.08f),  // Almost black
            nightProgress
        );
    }
    
    // Set the clear color to the sky color BEFORE first pass
    glClearColor(skyColor.x, skyColor.y, skyColor.z, 1.0f);
    
    // Update projection matrix with zoom
    float fov = baseFOV / zoomLevel;
    projection = glm::perspective(glm::radians(fov), 4.0f / 3.0f, 0.1f, 1000.0f);
    
    // Render to HDR framebuffer if enabled, otherwise use scene framebuffer
    GLuint targetFBO = hdrEnabled ? hdrFBO : sceneFBO;

    // Render scene
    glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    prog.use();
    // Sun and moon positions based on full circle rotation
    // (sunAngle and sunHeight already calculated above)
    
    // Sun position - always visible, but with alpha blending based on day/night
    float sunX = cos(sunAngle) * 150.0f;
    float sunY = sin(sunAngle) * 100.0f;
    glm::vec3 sunPos = cameraPos + glm::vec3(sunX, sunY, -100.0f);
    
    // Moon position - opposite the sun
    float moonAngle = sunAngle + glm::pi<float>();
    float moonHeight = sin(moonAngle);
    float moonX = cos(moonAngle) * 150.0f;
    float moonY = sin(moonAngle) * 100.0f;
    glm::vec3 moonPos = cameraPos + glm::vec3(moonX, moonY, -100.0f);
    
    // Render sun (visible when above horizon)
    if (sunHeight > -0.1f) {
        glm::mat4 sunModel = glm::translate(glm::mat4(1.0f), sunPos);
        sunModel = glm::scale(sunModel, glm::vec3(0.8f));
        glm::mat4 sunMVP = projection * view * sunModel;
        
        prog.setUniform("MVP", sunMVP);
        prog.setUniform("ModelMatrix", sunModel);
        prog.setUniform("lightColor", glm::vec3(1.0f, 0.95f, 0.1f));
        prog.setUniform("applyLighting", 0);
        
        glBindVertexArray(sunVAO);
        glDrawElements(GL_TRIANGLES, sunIndexCount, GL_UNSIGNED_INT, 0);
    }
    
    // Render moon (visible when below horizon, opposite side)
    if (moonHeight > -0.1f) {
        glm::mat4 moonModel = glm::translate(glm::mat4(1.0f), moonPos);
        moonModel = glm::scale(moonModel, glm::vec3(0.7f));
        glm::mat4 moonMVP = projection * view * moonModel;
        
        prog.setUniform("MVP", moonMVP);
        prog.setUniform("ModelMatrix", moonModel);
        prog.setUniform("lightColor", glm::vec3(0.95f, 0.95f, 0.95f));
        prog.setUniform("applyLighting", 0);
        
        glBindVertexArray(sunVAO);
        glDrawElements(GL_TRIANGLES, sunIndexCount, GL_UNSIGNED_INT, 0);
    }
    
    glEnable(GL_DEPTH_TEST);
    // Render moon (visible when below horizon, opposite side)
    if (moonHeight > -0.1f) {
        glm::mat4 moonModel = glm::translate(glm::mat4(1.0f), moonPos);
        moonModel = glm::scale(moonModel, glm::vec3(0.7f));
        glm::mat4 moonMVP = projection * view * moonModel;
        
        prog.setUniform("MVP", moonMVP);
        prog.setUniform("ModelMatrix", moonModel);
        prog.setUniform("lightColor", glm::vec3(0.95f, 0.95f, 0.95f));
        prog.setUniform("applyLighting", 0);
        
        glBindVertexArray(sunVAO);
        glDrawElements(GL_TRIANGLES, sunIndexCount, GL_UNSIGNED_INT, 0);
    }
    
    glEnable(GL_DEPTH_TEST);
    
    // Render terrain with dynamic lighting based on day-night cycle
    // Light position and color already calculated above based on sun height
    
    // Bind terrain textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, grassTexture);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, stoneTexture);
    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, dirtTexture);
    
    // Bind normal maps for terrain
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, grassNormalMap);
    
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, stoneNormalMap);
    
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, dirtNormalMap);
    
    // Use normal mapping shader for terrain
    progNormalMap.use();
    
    // Set fog color to match the sky color
    progNormalMap.setUniform("fogColor", skyColor);
    progNormalMap.setUniform("grassTexture", 0);
    progNormalMap.setUniform("stoneTexture", 1);
    progNormalMap.setUniform("dirtTexture", 2);
    progNormalMap.setUniform("grassNormal", 3);
    progNormalMap.setUniform("stoneNormal", 4);
    progNormalMap.setUniform("dirtNormal", 5);
    
    for (auto& chunk : terrainChunks) {
        glm::mat4 mvp = projection * view * glm::mat4(1.0f);
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(modelMatrix)));
        
        progNormalMap.setUniform("MVP", mvp);
        progNormalMap.setUniform("ModelMatrix", modelMatrix);
        progNormalMap.setUniform("NormalMatrix", normalMatrix);
        progNormalMap.setUniform("viewPos", cameraPos);
        
        // Use the already-calculated light position and color from day-night cycle above
        glm::vec3 sunDirection = glm::normalize(lightPos - cameraPos);
        
        progNormalMap.setUniform("sunDirection", sunDirection);
        progNormalMap.setUniform("lightIntensity", 1.0f);
        progNormalMap.setUniform("lightColor", lightColor);
        progNormalMap.setUniform("applyLighting", 1); // Apply lighting to terrain
        progNormalMap.setUniform("fogColor", skyColor);
        
        glBindVertexArray(chunk.second.vao);
        glDrawElements(GL_TRIANGLES, chunk.second.indexCount, GL_UNSIGNED_INT, 0);
    }
    
    // Render water with transparency and wave animation
    waterProg.use();
    
    glm::mat4 waterMVP = projection * view * glm::mat4(1.0f);
    waterProg.setUniform("MVP", waterMVP);
    waterProg.setUniform("ModelMatrix", glm::mat4(1.0f));
    waterProg.setUniform("time", float(glfwGetTime()));
    waterProg.setUniform("lightPos", lightPos);
    waterProg.setUniform("lightColor", lightColor);
    waterProg.setUniform("cameraPos", cameraPos);
    
    // Enable blending for water transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glBindVertexArray(waterVAO);
    glDrawElements(GL_TRIANGLES, waterIndexCount, GL_UNSIGNED_INT, 0);
    
    glDisable(GL_BLEND);
    
    // Render coins
    if (!coins.empty() && coinMesh != nullptr) {
        prog.use();  // Use standard shader for coins
        prog.setUniform("lightIntensity", 1.0f);  // Full brightness for coin visibility
        prog.setUniform("fogColor", skyColor);
        
        for (const auto& coin : coins) {
            if (coin.collected) continue;  // Skip collected coins
            
            glm::mat4 coinModel = glm::translate(glm::mat4(1.0f), coin.position);
            coinModel = glm::scale(coinModel, glm::vec3(2.5f));  // Scale coin to 2.5 size
            
            // Spin coin around Y axis first
            float coinTime = float(glfwGetTime());
            coinModel = glm::rotate(coinModel, coinTime * 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            
            // Then calculate direction from coin to camera
            glm::vec3 coinToCamera = glm::normalize(cameraPos - coin.position);
            
            // Calculate rotation to face camera (only around Y axis)
            float angle = atan2(coinToCamera.x, coinToCamera.z);
            coinModel = glm::rotate(coinModel, angle, glm::vec3(0.0f, 1.0f, 0.0f));
            
            glm::mat4 coinMVP = projection * view * coinModel;
            prog.setUniform("MVP", coinMVP);
            prog.setUniform("ModelMatrix", coinModel);
            prog.setUniform("viewPos", cameraPos);
            prog.setUniform("numLights", 0);
            prog.setUniform("applyLighting", 1);  // Apply lighting normally
            prog.setUniform("sunDirection", glm::normalize(lightPos - coin.position));
            prog.setUniform("lightColor", lightColor);
            prog.setUniform("renderingCoin", true);
            prog.setUniform("coinColor", glm::vec3(4.0f, 3.2f, 1.0f));  // Very bright HDR gold for bloom
            
            coinMesh->render();
            
            prog.setUniform("renderingCoin", false);
        }
    }
    
    // Pass 1: Scene already rendered to HDR/scene texture above
    
    glDisable(GL_DEPTH_TEST);
    
    // Determine which texture contains the rendered scene
    GLuint sourceTexture = hdrEnabled ? hdrColorTexture : sceneTexture;
    
    // Pass 2: Bright-pass filter (extract bright pixels above threshold)
    bloomThresholdProg.use();
    glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sourceTexture);
    bloomThresholdProg.setUniform("screenTexture", 0);
    bloomThresholdProg.setUniform("threshold", 0.75f);
    
    glBindVertexArray(bloomQuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    // Pass 3: Horizontal Gaussian blur
    blurProg.use();
    glBindFramebuffer(GL_FRAMEBUFFER, blurFBO);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bloomTexture);
    blurProg.setUniform("image", 0);
    blurProg.setUniform("horizontal", true);
    
    glBindVertexArray(bloomQuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    // Pass 4: Vertical Gaussian blur
    glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, blurTexture);
    blurProg.setUniform("image", 0);
    blurProg.setUniform("horizontal", false);
    
    glBindVertexArray(bloomQuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    // Pass 5: Tone mapping (if HDR enabled) + blend bloom result
    // Pass 5: Blend bloom with scene (no complex tone mapping)
    if (hdrEnabled) {
        // Simple bloom blending without separate tone mapping
        bloomBlendProg.use();
        glBindFramebuffer(GL_FRAMEBUFFER, finalFBO);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Blend scene with bloom
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, hdrColorTexture);
        bloomBlendProg.setUniform("scene", 0);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, bloomTexture);
        bloomBlendProg.setUniform("bloom", 1);
        bloomBlendProg.setUniform("bloomIntensity", 0.5f);
        
        glBindVertexArray(bloomQuadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    } else {
        // No HDR - simple copy without bloom
        glBindFramebuffer(GL_FRAMEBUFFER, finalFBO);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Use bloom shader but with bloom disabled
        bloomBlendProg.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sceneTexture);
        bloomBlendProg.setUniform("scene", 0);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, bloomTexture);  // Bind bloom texture (even if not used)
        bloomBlendProg.setUniform("bloom", 1);
        bloomBlendProg.setUniform("bloomIntensity", 0.0f);  // Don't blend it
        
        glBindVertexArray(bloomQuadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    
    // Apply vignette effect as final post-processing if enabled
    if (vignetteEnabled) {
        vignetteProg.use();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);  // Render to screen
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, finalTexture);  // Read from the composited result
        vignetteProg.setUniform("srcTexture", 0);
        vignetteProg.setUniform("vignetteIntensity", vignetteIntensity);
        
        glBindVertexArray(bloomQuadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    } else {
        // No vignette: copy final result directly to screen
        bloomBlendProg.use();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, finalTexture);
        bloomBlendProg.setUniform("scene", 0);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, finalTexture);  // Use final texture for bloom too (will be 0)
        bloomBlendProg.setUniform("bloom", 1);
        bloomBlendProg.setUniform("bloomIntensity", 0.0f);
        
        glBindVertexArray(bloomQuadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    
    glEnable(GL_DEPTH_TEST);
    
    // Render coin counter overlay at the top-left
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    prog.use();
    prog.setUniform("applyLighting", 0);
    prog.setUniform("numLights", 0);
    
    // Create UI projection matrix once
    glm::mat4 uiProj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    
    // Adjust for aspect ratio to keep UI elements square
    float aspectRatio = (float)width / (float)height;
    uiProj = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f);
    glm::mat4 uiView = glm::mat4(1.0f);
    
    // Draw coin counter indicators (small gold squares)
    // Center horizontally with padding from top
    float totalWidth = 10 * 0.15f;  // 10 coins with increased spacing
    float startX = -(totalWidth / 2.0f);
    float startY = 0.90f;  // Reduced padding from top
    float squareSize = 0.08f;  // Increased from 0.04f
    
    glBindVertexArray(uiQuadVAO);
    
    for (int i = 0; i < 10; i++) {
        glm::mat4 coinModel = glm::translate(glm::mat4(1.0f), glm::vec3(startX + i * 0.15f, startY, 0.0f));
        coinModel = glm::scale(coinModel, glm::vec3(squareSize, squareSize, 1.0f));
        
        glm::mat4 coinMVP = uiProj * uiView * coinModel;
        prog.setUniform("MVP", coinMVP);
        
        // Gold for collected coins, dark gray for uncollected
        if (i < coinsCollected) {
            prog.setUniform("lightColor", glm::vec3(1.0f, 0.84f, 0.0f));  // Gold
        } else {
            prog.setUniform("lightColor", glm::vec3(0.3f, 0.3f, 0.3f));  // Dark gray
        }
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    
    glEnable(GL_DEPTH_TEST);
    
    // Render completion overlay if game is complete
    if (gameCompleted) {
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Draw text using FreeType renderer
        if (textRenderer) {
            textProg.use();
            
            // Create orthographic projection for text (screen space coordinates in pixels)
            glm::mat4 textProj = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
            
            // Center calculations
            float centerX = (float)width / 2.0f;
            float centerY = (float)height / 2.0f;
            
            // Title text: "GAME COMPLETE!"
            glm::vec3 titleColor(1.0f, 1.0f, 1.0f);  // White
            std::string titleText = "GAME COMPLETE!";
            float titleScale = 2.0f;
            float titleWidth = textRenderer->GetTextWidth(titleText, titleScale);
            textRenderer->RenderText(titleText, centerX - titleWidth / 2.0f, centerY - 50.0f, titleScale, titleColor, textProg.getHandle(), textProj);
            
            // Subtitle text: "YOU COLLECTED ALL 10 COINS!"
            glm::vec3 subtitleColor(1.0f, 1.0f, 1.0f);  // White
            std::string subtitleText = "YOU COLLECTED ALL 10 COINS!";
            float subtitleScale = 1.5f;
            float subtitleWidth = textRenderer->GetTextWidth(subtitleText, subtitleScale);
            textRenderer->RenderText(subtitleText, centerX - subtitleWidth / 2.0f, centerY + 50.0f, subtitleScale, subtitleColor, textProg.getHandle(), textProj);
        }
        
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
    }
}

void SceneBasic_Uniform::resize(int w, int h)
{
    width = w;
    height = h;
    glViewport(0,0,w,h);
}
