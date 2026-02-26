#include "text_renderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <iostream>

TextRenderer::TextRenderer() : ft_library(nullptr), ft_face(nullptr), textureUnit(0)
{
    // Initialize FreeType
    if (FT_Init_FreeType(&ft_library))
    {
        std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    }
}

TextRenderer::~TextRenderer()
{
    if (ft_face)
        FT_Done_Face(ft_face);
    if (ft_library)
        FT_Done_FreeType(ft_library);
    
    if (VAO != 0)
        glDeleteVertexArrays(1, &VAO);
    if (VBO != 0)
        glDeleteBuffers(1, &VBO);
}

bool TextRenderer::LoadFont(const std::string& font_path, unsigned int font_size)
{
    // Load font
    if (FT_New_Face(ft_library, font_path.c_str(), 0, &ft_face))
    {
        std::cerr << "ERROR::FREETYPE: Failed to load font: " << font_path << std::endl;
        return false;
    }

    // Set font size
    FT_Set_Pixel_Sizes(ft_face, 0, font_size);

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Load first 128 characters of ASCII set
    for (unsigned char c = 0; c < 128; c++)
    {
        // Load character glyph
        if (FT_Load_Char(ft_face, c, FT_LOAD_RENDER))
        {
            std::cerr << "ERROR::FRETYPE: Failed to load Glyph" << std::endl;
            continue;
        }

        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            ft_face->glyph->bitmap.width,
            ft_face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            ft_face->glyph->bitmap.buffer
        );

        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(ft_face->glyph->bitmap.width, ft_face->glyph->bitmap.rows),
            glm::ivec2(ft_face->glyph->bitmap_left, ft_face->glyph->bitmap_top),
            (GLuint)ft_face->glyph->advance.x
        };
        characters.insert(std::pair<char, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    // Setup render data
    SetupRenderData();

    return true;
}

void TextRenderer::SetupRenderData()
{
    // Vertex data for a unit quad
    GLfloat vertices[6][4] = {
        { 0.0f, 1.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 1.0f, 0.0f },

        { 0.0f, 1.0f, 0.0f, 1.0f },
        { 1.0f, 0.0f, 1.0f, 0.0f },
        { 1.0f, 1.0f, 1.0f, 1.0f }
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void TextRenderer::RenderText(const std::string& text, GLfloat x, GLfloat y, GLfloat scale, 
                              const glm::vec3& color, GLuint shaderProgram, glm::mat4 projection)
{
    glUseProgram(shaderProgram);
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    GLint textColorLoc = glGetUniformLocation(shaderProgram, "textColor");
    glUniform3f(textColorLoc, color.x, color.y, color.z);

    glActiveTexture(GL_TEXTURE0 + textureUnit);
    GLint textLoc = glGetUniformLocation(shaderProgram, "text");
    glUniform1i(textLoc, textureUnit);

    glBindVertexArray(VAO);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;

        // Update VBO for each character
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 1.0f },
            { xpos,     ypos,       0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 0.0f },

            { xpos,     ypos + h,   0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 0.0f },
            { xpos + w, ypos + h,   1.0f, 1.0f }
        };

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_COPY_WRITE_BUFFER, VBO);
        glBufferSubData(GL_COPY_WRITE_BUFFER, 0, sizeof(vertices), vertices);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLfloat TextRenderer::GetTextWidth(const std::string& text, GLfloat scale)
{
    GLfloat width = 0.0f;
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = characters[*c];
        width += (ch.Advance >> 6) * scale;
    }
    return width;
}
