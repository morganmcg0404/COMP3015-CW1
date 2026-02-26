#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <glm/glm.hpp>
#include <string>

class TextRenderer {
public:
    // Render a text string at the given NDC coordinates
    // x, y are in normalized device coordinates (-1 to 1)
    // scale is the size multiplier
    // color is RGB
    static void renderText(const std::string& text, float x, float y, float scale, glm::vec3 color);
    
    static void initialize();
    
private:
    static void renderCharacter(char c, float& x, float y, float scale, glm::vec3 color);
};

#endif
