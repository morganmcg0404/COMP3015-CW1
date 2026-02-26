#include "coin.h"
#include <glm/gtc/constants.hpp>
#include <cmath>

Coin::Coin(float radius, float thickness, int segments) {
    generateCoin(radius, thickness, segments);
}

void Coin::generateCoin(float radius, float thickness, int segments) {
    std::vector<GLfloat> points;
    std::vector<GLfloat> normals;
    std::vector<GLuint> indices;

    float halfThickness = thickness / 2.0f;
    GLuint vertexCount = 0;

    // Generate vertices for top face
    int topStartIdx = vertexCount;
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * glm::pi<float>() * i / segments;
        float x = radius * std::cos(angle);
        float y = radius * std::sin(angle);

        points.push_back(x);
        points.push_back(y);
        points.push_back(halfThickness);

        normals.push_back(0.0f);
        normals.push_back(0.0f);
        normals.push_back(1.0f);
        
        vertexCount++;
    }

    // Top face center
    int topCenterIdx = vertexCount;
    points.push_back(0.0f);
    points.push_back(0.0f);
    points.push_back(halfThickness);
    normals.push_back(0.0f);
    normals.push_back(0.0f);
    normals.push_back(1.0f);
    vertexCount++;

    // Generate indices for top face
    for (int i = 0; i < segments; i++) {
        indices.push_back(topCenterIdx);
        indices.push_back(topStartIdx + i);
        indices.push_back(topStartIdx + i + 1);
    }

    // Generate vertices for bottom face
    int bottomStartIdx = vertexCount;
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * glm::pi<float>() * i / segments;
        float x = radius * std::cos(angle);
        float y = radius * std::sin(angle);

        points.push_back(x);
        points.push_back(y);
        points.push_back(-halfThickness);

        normals.push_back(0.0f);
        normals.push_back(0.0f);
        normals.push_back(-1.0f);
        
        vertexCount++;
    }

    // Bottom face center
    int bottomCenterIdx = vertexCount;
    points.push_back(0.0f);
    points.push_back(0.0f);
    points.push_back(-halfThickness);
    normals.push_back(0.0f);
    normals.push_back(0.0f);
    normals.push_back(-1.0f);
    vertexCount++;

    // Generate indices for bottom face
    for (int i = 0; i < segments; i++) {
        indices.push_back(bottomCenterIdx);
        indices.push_back(bottomStartIdx + i + 1);
        indices.push_back(bottomStartIdx + i);
    }

    // Generate vertices for side faces with proper normals
    int sideStartIdx = vertexCount;
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * glm::pi<float>() * i / segments;
        float nx = std::cos(angle);
        float ny = std::sin(angle);

        // Top vertex on side
        points.push_back(radius * nx);
        points.push_back(radius * ny);
        points.push_back(halfThickness);
        normals.push_back(nx);
        normals.push_back(ny);
        normals.push_back(0.0f);

        // Bottom vertex on side
        points.push_back(radius * nx);
        points.push_back(radius * ny);
        points.push_back(-halfThickness);
        normals.push_back(nx);
        normals.push_back(ny);
        normals.push_back(0.0f);
        
        vertexCount += 2;
    }

    // Generate indices for side faces
    for (int i = 0; i < segments; i++) {
        int t1 = sideStartIdx + i * 2;
        int t2 = sideStartIdx + (i + 1) * 2;
        int b1 = sideStartIdx + i * 2 + 1;
        int b2 = sideStartIdx + (i + 1) * 2 + 1;

        // First triangle
        indices.push_back(t1);
        indices.push_back(b1);
        indices.push_back(t2);

        // Second triangle
        indices.push_back(t2);
        indices.push_back(b1);
        indices.push_back(b2);
    }

    // Initialize the mesh with triangle list
    initBuffers(&indices, &points, &normals);
}
