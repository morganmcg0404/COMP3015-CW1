#include "arena.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cstdlib>
#include <cmath>
#include <iostream>

Arena::Arena() : meshesCreated(false) {}

Arena::~Arena() {
    cleanupMeshes();
}

void Arena::generate(ArenaSize size, int seed) {
    arenaSize = size;
    setSizeParameters();
    
    srand(seed);
    platforms.clear();
    pillars.clear();
    lights.clear();
    walls.clear();
    
    // Generate one half
    generateHalf();
    
    // Mirror to create the other half
    mirrorHalf();
    
    // Create floor and walls
    createFloorAndWalls();
    
    // Create meshes for rendering
    createMeshes();
}

void Arena::generateHalf() {
    // Generate pillars
    for (int x = 0; x < gridWidth; x++) {
        for (int z = 0; z < gridDepth / 2; z++) {
            if ((rand() % 100) / 100.0f < pillarDensity) {
                float posX = (x - gridWidth / 2.0f) * gridCellSize + gridCellSize / 2.0f;
                float posZ = z * gridCellSize + gridCellSize / 2.0f;
                float height = 3.0f + (rand() % 30) / 10.0f; // 3-6 units
                float radius = 0.3f + (rand() % 20) / 100.0f;
                
                glm::vec3 color(
                    0.4f + (rand() % 60) / 100.0f,
                    0.4f + (rand() % 60) / 100.0f,
                    0.4f + (rand() % 60) / 100.0f
                );
                
                pillars.push_back({
                    glm::vec3(posX, height / 2.0f, posZ),
                    height,
                    radius,
                    color
                });
            }
        }
    }
    
    // Generate lights on top of pillars - neon style
    for (const auto& pillar : pillars) {
        // Place light on top of each pillar
        float posY = pillar.position.y + pillar.height / 2.0f + 0.5f; // Above pillar top
        
        // Neon colors
        int colorType = rand() % 5;
        glm::vec3 color;
        switch (colorType) {
            case 0: color = glm::vec3(0.0f, 1.0f, 1.0f); break; // Cyan
            case 1: color = glm::vec3(1.0f, 0.0f, 1.0f); break; // Magenta
            case 2: color = glm::vec3(0.0f, 1.0f, 0.0f); break; // Green
            case 3: color = glm::vec3(1.0f, 0.5f, 0.0f); break; // Orange
            case 4: color = glm::vec3(1.0f, 0.0f, 0.0f); break; // Red
            default: color = glm::vec3(1.0f, 1.0f, 0.0f); break; // Yellow
        }
        
        lights.push_back({
            glm::vec3(pillar.position.x, posY, pillar.position.z),
            color,
            0.6f  // Reduced intensity
        });
    }
}

void Arena::mirrorHalf() {
    float arenaDepth = gridDepth * gridCellSize;
    
    // Mirror platforms around the center
    int platformCount = (int)platforms.size();
    for (int i = 0; i < platformCount; i++) {
        Platform p = platforms[i];
        p.position.z = arenaDepth - p.position.z;
        platforms.push_back(p);
    }
    
    // Mirror pillars
    int pillarCount = (int)pillars.size();
    for (int i = 0; i < pillarCount; i++) {
        Pillar p = pillars[i];
        p.position.z = arenaDepth - p.position.z;
        pillars.push_back(p);
    }
    
    // Generate lights for mirrored pillars
    int originalPillarCount = pillarCount;
    for (int i = originalPillarCount; i < (int)pillars.size(); i++) {
        const auto& pillar = pillars[i];
        float posY = pillar.position.y + pillar.height / 2.0f + 0.5f;
        
        int colorType = rand() % 5;
        glm::vec3 color;
        switch (colorType) {
            case 0: color = glm::vec3(0.0f, 1.0f, 1.0f); break; // Cyan
            case 1: color = glm::vec3(1.0f, 0.0f, 1.0f); break; // Magenta
            case 2: color = glm::vec3(0.0f, 1.0f, 0.0f); break; // Green
            case 3: color = glm::vec3(1.0f, 0.5f, 0.0f); break; // Orange
            case 4: color = glm::vec3(1.0f, 0.0f, 0.0f); break; // Red
            default: color = glm::vec3(1.0f, 1.0f, 0.0f); break; // Yellow
        }
        
        lights.push_back({
            glm::vec3(pillar.position.x, posY, pillar.position.z),
            color,
            0.6f
        });
    }
    
    std::cout << "Arena (" << (arenaSize == ArenaSize::SMALL ? "SMALL" : arenaSize == ArenaSize::MEDIUM ? "MEDIUM" : "LARGE")
              << ") generated: " << platforms.size() << " platforms, "
              << pillars.size() << " pillars, " << lights.size() << " lights" << std::endl;
}

void Arena::createMeshes() {
    createPlatformMesh();
    createPillarMesh();
    meshesCreated = true;
}

void Arena::setSizeParameters() {
    switch (arenaSize) {
        case ArenaSize::SMALL:
            gridWidth = 12;
            gridDepth = 24;
            gridCellSize = 0.8f;
            platformDensity = 0.12f;
            pillarDensity = 0.08f;
            lightDensity = 0.06f;
            break;
        case ArenaSize::MEDIUM:
            gridWidth = 20;
            gridDepth = 40;
            gridCellSize = 1.0f;
            platformDensity = 0.15f;
            pillarDensity = 0.1f;
            lightDensity = 0.08f;
            break;
        case ArenaSize::LARGE:
            gridWidth = 28;
            gridDepth = 56;
            gridCellSize = 1.2f;
            platformDensity = 0.18f;
            pillarDensity = 0.12f;
            lightDensity = 0.1f;
            break;
    }
}

void Arena::createFloorAndWalls() {
    float arenaWidth = gridWidth * gridCellSize;
    float arenaDepth = gridDepth * gridCellSize;
    float wallThickness = 0.5f;
    float wallHeight = 10.0f;
    
    // Create floor plane - matches the arena grid dimensions
    floorPlane = {
        glm::vec3(0.0f, -0.1f, arenaDepth / 2.0f),
        glm::vec3(arenaWidth, 0.1f, arenaDepth),
        glm::vec3(0.3f, 0.3f, 0.35f) // Dark gray-blue for floor
    };
    platforms.push_back(floorPlane);
    
    // Front wall (at z = -wallThickness/2)
    walls.push_back({
        glm::vec3(0.0f, wallHeight / 2.0f, -wallThickness / 2.0f),
        glm::vec3(arenaWidth + wallThickness * 2, wallHeight, wallThickness),
        glm::vec3(0.2f, 0.2f, 0.25f) // Darker for walls
    });
    
    // Back wall (at z = arenaDepth - wallThickness/2)
    walls.push_back({
        glm::vec3(0.0f, wallHeight / 2.0f, arenaDepth - wallThickness / 2.0f),
        glm::vec3(arenaWidth + wallThickness * 2, wallHeight, wallThickness),
        glm::vec3(0.2f, 0.2f, 0.25f)
    });
    
    // Left wall (at x = -arenaWidth/2 - wallThickness/2)
    walls.push_back({
        glm::vec3(-arenaWidth / 2.0f - wallThickness / 2.0f, wallHeight / 2.0f, arenaDepth / 2.0f),
        glm::vec3(wallThickness, wallHeight, arenaDepth),
        glm::vec3(0.2f, 0.2f, 0.25f)
    });
    
    // Right wall (at x = arenaWidth/2 + wallThickness/2)
    walls.push_back({
        glm::vec3(arenaWidth / 2.0f + wallThickness / 2.0f, wallHeight / 2.0f, arenaDepth / 2.0f),
        glm::vec3(wallThickness, wallHeight, arenaDepth),
        glm::vec3(0.2f, 0.2f, 0.25f)
    });
    
    // Add walls to platforms list for rendering
    for (const auto& wall : walls) {
        platforms.push_back(wall);
    }
}

void Arena::createPlatformMesh() {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> colors;
    std::vector<GLuint> indices;
    
    // Create geometry for each platform
    for (size_t p = 0; p < platforms.size(); p++) {
        const Platform& platform = platforms[p];
        glm::vec3 half_scale = platform.scale / 2.0f;
        glm::vec3 pos = platform.position;
        
        // 8 vertices for a box
        int baseIndex = (int)vertices.size();
        
        // Bottom face
        vertices.push_back(pos + glm::vec3(-half_scale.x, -half_scale.y, -half_scale.z));
        vertices.push_back(pos + glm::vec3(half_scale.x, -half_scale.y, -half_scale.z));
        vertices.push_back(pos + glm::vec3(half_scale.x, -half_scale.y, half_scale.z));
        vertices.push_back(pos + glm::vec3(-half_scale.x, -half_scale.y, half_scale.z));
        
        // Top face
        vertices.push_back(pos + glm::vec3(-half_scale.x, half_scale.y, -half_scale.z));
        vertices.push_back(pos + glm::vec3(half_scale.x, half_scale.y, -half_scale.z));
        vertices.push_back(pos + glm::vec3(half_scale.x, half_scale.y, half_scale.z));
        vertices.push_back(pos + glm::vec3(-half_scale.x, half_scale.y, half_scale.z));
        
        // Add colors for each vertex
        for (int i = 0; i < 8; i++) {
            colors.push_back(platform.color);
        }
        
        // Add indices (6 faces, 2 triangles each, 3 indices each)
        // Front
        indices.push_back(baseIndex + 0); indices.push_back(baseIndex + 1); indices.push_back(baseIndex + 5);
        indices.push_back(baseIndex + 0); indices.push_back(baseIndex + 5); indices.push_back(baseIndex + 4);
        // Back
        indices.push_back(baseIndex + 2); indices.push_back(baseIndex + 3); indices.push_back(baseIndex + 7);
        indices.push_back(baseIndex + 2); indices.push_back(baseIndex + 7); indices.push_back(baseIndex + 6);
        // Left
        indices.push_back(baseIndex + 3); indices.push_back(baseIndex + 0); indices.push_back(baseIndex + 4);
        indices.push_back(baseIndex + 3); indices.push_back(baseIndex + 4); indices.push_back(baseIndex + 7);
        // Right
        indices.push_back(baseIndex + 1); indices.push_back(baseIndex + 2); indices.push_back(baseIndex + 6);
        indices.push_back(baseIndex + 1); indices.push_back(baseIndex + 6); indices.push_back(baseIndex + 5);
        // Top
        indices.push_back(baseIndex + 4); indices.push_back(baseIndex + 5); indices.push_back(baseIndex + 6);
        indices.push_back(baseIndex + 4); indices.push_back(baseIndex + 6); indices.push_back(baseIndex + 7);
        // Bottom
        indices.push_back(baseIndex + 1); indices.push_back(baseIndex + 0); indices.push_back(baseIndex + 3);
        indices.push_back(baseIndex + 1); indices.push_back(baseIndex + 3); indices.push_back(baseIndex + 2);
    }
    
    platformMesh.indexCount = (GLuint)indices.size();
    
    // Create VAO, VBO, EBO
    glGenVertexArrays(1, &platformMesh.vao);
    glGenBuffers(1, &platformMesh.vbo);
    glGenBuffers(1, &platformMesh.ebo);
    
    glBindVertexArray(platformMesh.vao);
    
    // Position buffer
    glBindBuffer(GL_ARRAY_BUFFER, platformMesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Color buffer (interleaved in same VBO)
    GLuint colorVBO;
    glGenBuffers(1, &colorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(1);
    
    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, platformMesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
    
    glBindVertexArray(0);
}

void Arena::createPillarMesh() {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> colors;
    std::vector<GLuint> indices;
    
    // Create cube for each pillar
    for (size_t p = 0; p < pillars.size(); p++) {
        const Pillar& pillar = pillars[p];
        glm::vec3 pos = pillar.position;
        float h = pillar.height;
        float w = pillar.radius * 2.0f; // width (convert radius to width)
        
        int baseIndex = (int)vertices.size();
        
        // 8 vertices for a cube
        // Bottom face
        vertices.push_back(pos + glm::vec3(-w/2, -h/2, -w/2));
        vertices.push_back(pos + glm::vec3(w/2, -h/2, -w/2));
        vertices.push_back(pos + glm::vec3(w/2, -h/2, w/2));
        vertices.push_back(pos + glm::vec3(-w/2, -h/2, w/2));
        
        // Top face
        vertices.push_back(pos + glm::vec3(-w/2, h/2, -w/2));
        vertices.push_back(pos + glm::vec3(w/2, h/2, -w/2));
        vertices.push_back(pos + glm::vec3(w/2, h/2, w/2));
        vertices.push_back(pos + glm::vec3(-w/2, h/2, w/2));
        
        // Add colors for each vertex
        for (int i = 0; i < 8; i++) {
            colors.push_back(pillar.color);
        }
        
        // Add indices (6 faces, 2 triangles each)
        // Front
        indices.push_back(baseIndex + 0); indices.push_back(baseIndex + 1); indices.push_back(baseIndex + 5);
        indices.push_back(baseIndex + 0); indices.push_back(baseIndex + 5); indices.push_back(baseIndex + 4);
        // Back
        indices.push_back(baseIndex + 2); indices.push_back(baseIndex + 3); indices.push_back(baseIndex + 7);
        indices.push_back(baseIndex + 2); indices.push_back(baseIndex + 7); indices.push_back(baseIndex + 6);
        // Left
        indices.push_back(baseIndex + 3); indices.push_back(baseIndex + 0); indices.push_back(baseIndex + 4);
        indices.push_back(baseIndex + 3); indices.push_back(baseIndex + 4); indices.push_back(baseIndex + 7);
        // Right
        indices.push_back(baseIndex + 1); indices.push_back(baseIndex + 2); indices.push_back(baseIndex + 6);
        indices.push_back(baseIndex + 1); indices.push_back(baseIndex + 6); indices.push_back(baseIndex + 5);
        // Top
        indices.push_back(baseIndex + 4); indices.push_back(baseIndex + 5); indices.push_back(baseIndex + 6);
        indices.push_back(baseIndex + 4); indices.push_back(baseIndex + 6); indices.push_back(baseIndex + 7);
        // Bottom
        indices.push_back(baseIndex + 1); indices.push_back(baseIndex + 0); indices.push_back(baseIndex + 3);
        indices.push_back(baseIndex + 1); indices.push_back(baseIndex + 3); indices.push_back(baseIndex + 2);
    }
    
    pillarMesh.indexCount = (GLuint)indices.size();
    
    glGenVertexArrays(1, &pillarMesh.vao);
    glGenBuffers(1, &pillarMesh.vbo);
    glGenBuffers(1, &pillarMesh.ebo);
    
    glBindVertexArray(pillarMesh.vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, pillarMesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);
    
    GLuint colorVBO;
    glGenBuffers(1, &colorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pillarMesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
    
    glBindVertexArray(0);
}

void Arena::renderPlatforms() {
    if (!meshesCreated) return;
    
    glBindVertexArray(platformMesh.vao);
    glDrawElements(GL_TRIANGLES, platformMesh.indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Arena::renderPillars() {
    if (!meshesCreated) return;
    
    glBindVertexArray(pillarMesh.vao);
    glDrawElements(GL_TRIANGLES, pillarMesh.indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Arena::cleanupMeshes() {
    if (meshesCreated) {
        glDeleteBuffers(1, &platformMesh.vbo);
        glDeleteBuffers(1, &platformMesh.ebo);
        glDeleteVertexArrays(1, &platformMesh.vao);
        
        glDeleteBuffers(1, &pillarMesh.vbo);
        glDeleteBuffers(1, &pillarMesh.ebo);
        glDeleteVertexArrays(1, &pillarMesh.vao);
        
        meshesCreated = false;
    }
}
