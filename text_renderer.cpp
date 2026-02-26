#include "text_renderer.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

// Simple bitmap character representation (5x7 grid for each character)
// Each character is represented as a 7-byte array where each byte is a row of pixels

struct BitmapChar {
    unsigned char pixels[7];
};

// Simple 5x7 bitmap fonts for common characters
static BitmapChar bitmapFont[256];

void initializeBitmapFont() {
    // Initialize with simple dot patterns for letters (simplified for brevity)
    // In a real implementation, this would contain actual bitmap data for each character
    
    // A
    bitmapFont['A'].pixels[0] = 0b01110;
    bitmapFont['A'].pixels[1] = 0b10001;
    bitmapFont['A'].pixels[2] = 0b10001;
    bitmapFont['A'].pixels[3] = 0b11111;
    bitmapFont['A'].pixels[4] = 0b10001;
    bitmapFont['A'].pixels[5] = 0b10001;
    bitmapFont['A'].pixels[6] = 0b00000;
    
    // G (for "GAME COMPLETE")
    bitmapFont['G'].pixels[0] = 0b01110;
    bitmapFont['G'].pixels[1] = 0b10001;
    bitmapFont['G'].pixels[2] = 0b10000;
    bitmapFont['G'].pixels[3] = 0b10111;
    bitmapFont['G'].pixels[4] = 0b10001;
    bitmapFont['G'].pixels[5] = 0b01110;
    bitmapFont['G'].pixels[6] = 0b00000;
    
    // M
    bitmapFont['M'].pixels[0] = 0b10001;
    bitmapFont['M'].pixels[1] = 0b11011;
    bitmapFont['M'].pixels[2] = 0b10101;
    bitmapFont['M'].pixels[3] = 0b10001;
    bitmapFont['M'].pixels[4] = 0b10001;
    bitmapFont['M'].pixels[5] = 0b10001;
    bitmapFont['M'].pixels[6] = 0b00000;
    
    // E
    bitmapFont['E'].pixels[0] = 0b11111;
    bitmapFont['E'].pixels[1] = 0b10000;
    bitmapFont['E'].pixels[2] = 0b11110;
    bitmapFont['E'].pixels[3] = 0b10000;
    bitmapFont['E'].pixels[4] = 0b10000;
    bitmapFont['E'].pixels[5] = 0b11111;
    bitmapFont['E'].pixels[6] = 0b00000;
    
    // C
    bitmapFont['C'].pixels[0] = 0b01110;
    bitmapFont['C'].pixels[1] = 0b10001;
    bitmapFont['C'].pixels[2] = 0b10000;
    bitmapFont['C'].pixels[3] = 0b10000;
    bitmapFont['C'].pixels[4] = 0b10001;
    bitmapFont['C'].pixels[5] = 0b01110;
    bitmapFont['C'].pixels[6] = 0b00000;
    
    // Space
    for (int i = 0; i < 7; i++) {
        bitmapFont[' '].pixels[i] = 0b00000;
    }
}

void TextRenderer::initialize() {
    initializeBitmapFont();
}

void TextRenderer::renderText(const std::string& text, float x, float y, float scale, glm::vec3 color) {
    float currentX = x;
    float charWidth = 0.05f * scale;
    float charHeight = 0.08f * scale;
    
    for (char c : text) {
        if (c == ' ') {
            currentX += charWidth * 0.8f;
            continue;
        }
        
        renderCharacter(c, currentX, y, scale, color);
        currentX += charWidth;
    }
}

void TextRenderer::renderCharacter(char c, float& x, float y, float scale, glm::vec3 color) {
    // For now, just render a simple placeholder rectangle for each character
    // In a full implementation, this would render the bitmap to a texture and display it
    // This is a simplified version that just shows colored blocks
    
    float charWidth = 0.04f * scale;
    float charHeight = 0.06f * scale;
    
    // Render as a simple rectangle (placeholder)
    // This will be drawn using the existing UI quad system
}
