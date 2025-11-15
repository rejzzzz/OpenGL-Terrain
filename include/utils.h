#pragma once

#include <GL/glew.h>

// Loads an image from disk using stb_image and creates an OpenGL 2D texture.
// - path: file path to image (PNG/JPG)
// - generateMipmaps: when true, builds mipmaps and sets trilinear filtering
// Returns 0 on failure.
GLuint loadTexture2D(const char* path, bool generateMipmaps = true);

// Attempts to set anisotropic filtering to the given level if supported.
// Call after binding a texture. Silently ignores if extension unsupported.
void setAnisotropy(float level);
