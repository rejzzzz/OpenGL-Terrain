#include "skybox/skybox.h"
#include <GL/glew.h>
#include <GL/glu.h>
#include "camera/Camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <vector>
#include <string>
#include <cstdio>
#include <iostream>

// Desired skybox face size (width and height)
static const int SKYBOX_FACE_SIZE = 512;

// Simple nearest-neighbour resize from (sw x sh) to (tw x th)
static std::vector<unsigned char> resizeNearest(const unsigned char* src, int sw, int sh, int channels, int tw, int th) {
    std::vector<unsigned char> dst((size_t)tw * th * channels);
    dst.assign((size_t)tw * th * channels, 0);
    for (int y = 0; y < th; ++y) {
        int sy = (y * sh) / th;
        if (sy >= sh) sy = sh - 1;
        for (int x = 0; x < tw; ++x) {
            int sx = (x * sw) / tw;
            if (sx >= sw) sx = sw - 1;
            size_t sidx = ((size_t)sy * sw + sx) * channels;
            size_t didx = ((size_t)y * tw + x) * channels;
            for (int c = 0; c < channels; ++c) dst[didx + c] = src[sidx + c];
        }
    }
    return dst;
}

// Static texture ID for the skybox cube map
static GLuint skyboxTextureID;

void loadSkybox(const std::vector<std::string>& faces) {
    glGenTextures(1, &skyboxTextureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);

    int width, height, nrChannels;
    GLenum targets[] = {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };

    stbi_set_flip_vertically_on_load(false);

    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            printf("Loaded %s (%dx%d, %d channels)\n", faces[i].c_str(), width, height, nrChannels);

            GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

            // If the image isn't the desired SKYBOX_FACE_SIZE, resample it.
            if (width != SKYBOX_FACE_SIZE || height != SKYBOX_FACE_SIZE) {
                auto resized = resizeNearest(data, width, height, nrChannels, SKYBOX_FACE_SIZE, SKYBOX_FACE_SIZE);
                printf("Resampled %s -> %dx%d\n", faces[i].c_str(), SKYBOX_FACE_SIZE, SKYBOX_FACE_SIZE);
                glTexImage2D(targets[i], 0, format, SKYBOX_FACE_SIZE, SKYBOX_FACE_SIZE, 0, format, GL_UNSIGNED_BYTE, resized.data());
            } else {
                glTexImage2D(targets[i], 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            }

            stbi_image_free(data);
        } else {
            printf("❌ Failed to load skybox texture: %s\n", faces[i].c_str());
            stbi_image_free(data);
        }
    }

    // --- High-quality texture filtering and wrapping ---
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Generate mipmaps for smooth transitions
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // Enable anisotropic filtering (if supported)
    GLfloat maxAniso = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
    if (maxAniso > 0.0f) {
        glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
        printf("Anisotropic filtering enabled (%.1fx)\n", maxAniso);
    } else {
        printf("Anisotropic filtering not supported.\n");
    }
}

void drawSkybox(const Camera& camera, const glm::vec3& center) {
    // Save relevant OpenGL states
    glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // --- State setup ---
    glDepthMask(GL_FALSE);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);
    glColor3f(1.0f, 1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    // Camera trick: only rotate skybox with camera, not translate
    glm::vec3 eye = camera.GetPosition();
    glm::vec3 up(0, 1, 0);
    glm::vec3 lookDirection = center - eye;

    gluLookAt(0, 0, 0,
              lookDirection.x, lookDirection.y, lookDirection.z,
              up.x, up.y, up.z);

    // --- Draw cube ---
    float size = 50.0f;

    glBegin(GL_QUADS);
        // +X (right)
        glTexCoord3f( 1, -1, -1); glVertex3f( size, -size, -size);
        glTexCoord3f( 1, -1,  1); glVertex3f( size, -size,  size);
        glTexCoord3f( 1,  1,  1); glVertex3f( size,  size,  size);
        glTexCoord3f( 1,  1, -1); glVertex3f( size,  size, -size);

        // -X (left)
        glTexCoord3f(-1, -1,  1); glVertex3f(-size, -size,  size);
        glTexCoord3f(-1, -1, -1); glVertex3f(-size, -size, -size);
        glTexCoord3f(-1,  1, -1); glVertex3f(-size,  size, -size);
        glTexCoord3f(-1,  1,  1); glVertex3f(-size,  size,  size);

        // +Y (top)
        glTexCoord3f(-1, 1, -1); glVertex3f(-size,  size, -size);
        glTexCoord3f( 1, 1, -1); glVertex3f( size,  size, -size);
        glTexCoord3f( 1, 1,  1); glVertex3f( size,  size,  size);
        glTexCoord3f(-1, 1,  1); glVertex3f(-size,  size,  size);

        // -Y (bottom)
        glTexCoord3f(-1, -1,  1); glVertex3f(-size, -size,  size);
        glTexCoord3f( 1, -1,  1); glVertex3f( size, -size,  size);
        glTexCoord3f( 1, -1, -1); glVertex3f( size, -size, -size);
        glTexCoord3f(-1, -1, -1); glVertex3f(-size, -size, -size);

        // +Z (front)
        glTexCoord3f( 1, -1, 1); glVertex3f( size, -size, size);
        glTexCoord3f(-1, -1, 1); glVertex3f(-size, -size, size);
        glTexCoord3f(-1,  1, 1); glVertex3f(-size,  size, size);
        glTexCoord3f( 1,  1, 1); glVertex3f( size,  size, size);

        // -Z (back)
        glTexCoord3f(-1, -1, -1); glVertex3f(-size, -size, -size);
        glTexCoord3f( 1, -1, -1); glVertex3f( size, -size, -size);
        glTexCoord3f( 1,  1, -1); glVertex3f( size,  size, -size);
        glTexCoord3f(-1,  1, -1); glVertex3f(-size,  size, -size);
    glEnd();

    glPopMatrix();

    // Restore previous OpenGL states
    glPopAttrib();
}
