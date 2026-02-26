#ifndef ARENA_H
#define ARENA_H

#include <glm/glm.hpp>
#include <vector>
#include <glad/glad.h>

enum class ArenaSize {
    SMALL,
    MEDIUM,
    LARGE
};

struct Platform {
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 color;
};

struct Pillar {
    glm::vec3 position;
    float height;
    float radius;
    glm::vec3 color;
};

struct Light {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
};

struct Mesh {
    GLuint vao;
    GLuint indexCount;
    GLuint vbo;
    GLuint ebo;
};

class Arena {
private:
    std::vector<Platform> platforms;
    std::vector<Pillar> pillars;
    std::vector<Light> lights;
    
    Platform floorPlane;
    std::vector<Platform> walls;
    
    Mesh platformMesh;
    Mesh pillarMesh;
    
    bool meshesCreated;
    ArenaSize arenaSize;
    
    // Arena dimensions based on size
    int gridWidth;
    int gridDepth;
    float gridCellSize;
    
    float platformDensity;
    float pillarDensity;
    float lightDensity;
    
public:
    Arena();
    ~Arena();
    
    void generate(ArenaSize size, int seed);
    void createMeshes();
    void renderPlatforms();
    void renderPillars();
    
    const std::vector<Light>& getLights() const { return lights; }
    int getLightCount() const { return (int)lights.size(); }
    
private:
    void setSizeParameters();
    void createFloorAndWalls();
    void generateHalf();
    void mirrorHalf();
    void createPlatformMesh();
    void createPillarMesh();
    void cleanupMeshes();
};

#endif // ARENA_H
