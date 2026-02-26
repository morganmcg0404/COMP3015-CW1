#include <iostream>
#include <vector>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

// Simple normal map generator - creates a basic normal map texture
// A neutral normal map points straight up (0.5, 0.5, 1.0) in RGB
void generateNormalMap(int width, int height, const char* filename) {
    std::vector<unsigned char> data(width * height * 3);
    
    // Create a basic normal map - most pixels point up (neutral)
    // with some slight variation for detail
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = (y * width + x) * 3;
            
            // Create subtle variation using Perlin-like noise (simple sine waves)
            float nx = 0.5f + 0.1f * sin(x * 0.05f) * cos(y * 0.05f);
            float ny = 0.5f + 0.1f * cos(x * 0.05f) * sin(y * 0.05f);
            float nz = 1.0f;  // Z always pointing up for subtle effect
            
            // Normalize the vector
            float len = sqrt(nx * nx + ny * ny + nz * nz);
            nx /= len;
            ny /= len;
            nz /= len;
            
            // Convert from [-1, 1] to [0, 255]
            data[idx] = (unsigned char)((nx + 1.0f) * 127.5f);
            data[idx + 1] = (unsigned char)((ny + 1.0f) * 127.5f);
            data[idx + 2] = (unsigned char)((nz + 1.0f) * 127.5f);
        }
    }
    
    // Write PNG
    if (stbi_write_png(filename, width, height, 3, data.data(), width * 3)) {
        std::cout << "Generated normal map: " << filename << std::endl;
    } else {
        std::cerr << "Failed to generate normal map: " << filename << std::endl;
    }
}

int main() {
    // Generate normal maps for terrain textures
    // 256x256 is a good size for terrain
    generateNormalMap(256, 256, "textures/grass_normal.png");
    generateNormalMap(256, 256, "textures/stone_normal.png");
    generateNormalMap(256, 256, "textures/dirt_normal.png");
    
    std::cout << "Normal maps generated successfully!" << std::endl;
    return 0;
}
