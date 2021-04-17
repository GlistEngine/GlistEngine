/*
 * gFont.cpp
 *
 *  Created on: May 17, 2020
 *      Author: noyan
 */

#include "gFont.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


gFont::gFont() {
	vao = 0;
	vbo = 0;
	fontsize = 12;
	face = nullptr;
	ft = nullptr;
}

gFont::~gFont() {
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void gFont::loadFont(std::string fontPath, int size) {
	load(gGetFontsDir() + fontPath, size);
}


void gFont::load(std::string fullPath, int size) {
	fontsize = size;
    // FreeType
    // --------
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

    // load font as face
    if (FT_New_Face(ft, fullPath.c_str(), 0, &face))
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

    // set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, fontsize);

    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // load first 128 characters of ASCII set
    for (ci = 0; ci < 128; ci++) {
        // Load character glyph
        if (FT_Load_Char(face, ci, FT_LOAD_RENDER)) {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        Character character;
        character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
        };
        Characters.insert(std::pair<char, Character>(ci, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    // destroy FreeType once we're finished
//    FT_Done_Face(face);
//    FT_Done_FreeType(ft);

    // configure VAO/VBO for texture quads
    // -----------------------------------
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


void gFont::drawText(std::string text, float x, float y) {
    // OpenGL state
    // ------------
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // activate corresponding render state
	int vertexColorLocation = glGetUniformLocation(renderer->getFontShader()->id, "textColor");
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(renderer->getWidth()), 0.0f, static_cast<float>(renderer->getHeight()));
    renderer->getFontShader()->use();
    glUniform3f(vertexColorLocation, renderer->getColor()->r, renderer->getColor()->g, renderer->getColor()->b);
    glUniformMatrix4fv(glGetUniformLocation(renderer->getFontShader()->id, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(vao);

    // iterate through all characters
    for (c = text.begin(); c != text.end(); c++) {
        ch = Characters[*c];

        xpos = x + ch.Bearing.x;
        ypos = renderer->getHeight() - y - (ch.Size.y - ch.Bearing.y);

        csx = ch.Size.x;
        csy = ch.Size.y;
//        logi("gFont", "ch:" + str(ch.Advance) + ", c:" + str(*c) + ", w:" + str(w) + ", h:" + str(h) + ", xpos:" + str(xpos) + ", ypos:" + str(ypos) + ", texid:" + str(ch.TextureID));
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + csy,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + csx, ypos,       1.0f, 1.0f },

            { xpos,     ypos + csy,   0.0f, 0.0f },
            { xpos + csx, ypos,       1.0f, 1.0f },
            { xpos + csx, ypos + csy,   1.0f, 0.0f }
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6); // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);

}


float gFont::getStringWidth(std::string text) {
    ssw = 0.0f;
    for (cs1 = text.begin(); cs1 != text.end(); cs1++) {
    	ssw += (Characters[*cs1].Advance >> 6);
    }
	return ssw;
}

float gFont::getStringHeight(std::string text) {
	ssh = 0.0f;
    for (cs2 = text.begin(); cs2 != text.end(); cs2++) {
    	cthy = Characters[*cs2].Size.y;
    	if (cthy > ssh) ssh = cthy;
    }
	return ssh;
}


