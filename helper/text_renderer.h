#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <map>
#include <ft2build.h>
#include FT_FREETYPE_H

class TextRenderer
{
public:
    struct Character {
        GLuint TextureID;  // ID handle of the glyph texture
        glm::ivec2 Size;   // Size of glyph
        glm::ivec2 Bearing;// Offset from baseline to left/top of glyph
        GLuint Advance;    // Horizontal offset to advance to next glyph
    };

    TextRenderer();
    ~TextRenderer();

    // Load a TrueType font
    bool LoadFont(const std::string& font_path, unsigned int font_size);

    // Render text using orthographic projection
    void RenderText(const std::string& text, GLfloat x, GLfloat y, GLfloat scale, const glm::vec3& color,
                    GLuint shaderProgram, glm::mat4 projection);

    // Calculate the width of text when rendered at given scale
    GLfloat GetTextWidth(const std::string& text, GLfloat scale);

    // Set texture unit for character rendering
    void SetTextureUnit(GLint unit) { textureUnit = unit; }

private:
    FT_Library ft_library;
    FT_Face ft_face;
    std::map<char, Character> characters;
    GLuint VAO, VBO;
    GLint textureUnit;

    void SetupRenderData();
};
