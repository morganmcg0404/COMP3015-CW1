#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"
#include "helper/text_renderer.h"
#include "helper/coin.h"
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <map>
#include "arena.h"

class SceneBasic_Uniform : public Scene
{
private:
    GLuint vaoHandle;
    GLuint indexCount;
    GLSLProgram prog;
    GLSLProgram progNormalMap;  // Normal mapping shader program
    GLSLProgram shadowProg;
    GLSLProgram bloomThresholdProg;
    GLSLProgram blurProg;
    GLSLProgram bloomBlendProg;
    GLSLProgram toneMappingProg;
    GLSLProgram vignetteProg;
    GLSLProgram waterProg;
    GLSLProgram textProg;
    
    // Text rendering
    TextRenderer* textRenderer;
    
    // Shadow mapping
    GLuint shadowFBO;
    GLuint shadowDepthMap;
    const GLuint SHADOW_WIDTH = 2048;
    const GLuint SHADOW_HEIGHT = 2048;
    
    // HDR framebuffer for tone mapping
    GLuint hdrFBO;
    GLuint hdrColorTexture;
    GLuint hdrDepthRBO;
    
    // Bloom effect
    GLuint sceneFBO;
    GLuint sceneTexture;
    GLuint sceneDepthRBO;
    GLuint bloomFBO;
    GLuint bloomTexture;
    GLuint blurFBO;
    GLuint blurTexture;
    GLuint bloomQuadVAO;
    
    // Final framebuffer for vignette post-processing
    GLuint finalFBO;
    GLuint finalTexture;
    
    // HDR control
    bool hdrEnabled;
    
    float angle;
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    
    // Camera
    glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    float cameraSpeed;
    float cameraSensitivity;
    float smoothCameraHeight;  // For smooth camera transitions on slopes
    const float CAMERA_SMOOTH_SPEED = 8.0f;  // How quickly camera height smooths
    
    // Player character
    glm::vec3 playerPos;
    float playerYaw;
    float playerPitch;
    float playerSpeed;
    float playerHeight;
    const float PLAYER_HEIGHT = 1.7f;
    bool isSprintingEnabled;
    const float SPRINT_MULTIPLIER = 1.5f;
    
    // Player jump
    float playerVerticalVelocity;
    bool isPlayerInAir;
    float jumpPower;
    float gravity;
    const float JUMP_POWER = 8.0f;
    const float GRAVITY = 9.81f;
    
    // Get terrain height at a given world position
    float getTerrainHeightAt(float x, float z);
    
    // Zoom controls
    bool isZooming;
    float zoomLevel;
    float baseFOV;
    double lastZoomMouseY;
    double scrollOffset;  // For scroll wheel zoom
    float rememberedZoomLevel;  // Remember zoom level when Z is released
    static SceneBasic_Uniform* instancePtr;  // For scroll callback
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    
    // Vignette effect
    bool vignetteEnabled;
    float vignetteIntensity;
    
    // Mouse
    double lastMouseX;
    double lastMouseY;
    bool firstMouse;
    float yaw;
    float pitch;
    
    // Arena
    Arena arena;
    
    // Sun
    GLuint sunVAO;
    GLuint sunIndexCount;
    
    // Water
    GLuint waterVAO;
    GLuint waterIndexCount;
    float waterLevel;  // Fixed water height
    const float WATER_LEVEL = 0.5f;
    
    // Coin model
    struct CoinInstance {
        glm::vec3 position;
        bool collected;
    };
    Coin* coinMesh;  // Procedurally generated coin
    GLuint coinTexture;
    std::vector<CoinInstance> coins;
    int coinsCollected;
    float lastCoinCollectionTime;
    bool gameInitialized;
    bool gameCompleted;
    const float COIN_PICKUP_RADIUS = 3.0f;
    const float COIN_COLLECTION_COOLDOWN = 0.5f;  // 0.5 second cooldown between collections
    void spawnCoins();
    void spawnSingleCoin();
    
    // UI elements
    GLuint uiQuadVAO;
    void createUIQuad();
    
    // Skybox
    GLuint skyboxVAO;
    GLuint skyboxIndexCount;
    std::vector<glm::vec3> skyboxColors;
    
    // Terrain chunks
    struct TerrainChunk {
        GLuint vao;
        GLuint indexCount;
        glm::vec3 position;
    };
    std::map<std::pair<int, int>, TerrainChunk> terrainChunks;
    glm::ivec2 lastChunkCoord;
    
    // Seed for terrain generation
    uint32_t worldSeed;
    
    // Day-night cycle
    float dayNightTime;  // 0 to 1 represents a full day cycle
    
    // Textures
    GLuint grassTexture;
    GLuint stoneTexture;
    GLuint dirtTexture;
    GLuint grassNormalMap;
    GLuint stoneNormalMap;
    GLuint dirtNormalMap;
    GLuint loadTexture(const char* path);
    GLuint createDefaultTexture();
    GLuint createDefaultNormalMap();

    void compile();
    void createPlane();
    void createTerrainChunk(int chunkX, int chunkZ);
    void updateTerrainChunks();
    void createSun();
    void createSkybox();
    void createWater();
    void handleInput();
    void updateCamera();
    
    // Hash function for seeded randomness
    float seededNoise(float x, float y, uint32_t seed);

public:
    SceneBasic_Uniform();

    void initScene();
    void update( float t );
    void render();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H
