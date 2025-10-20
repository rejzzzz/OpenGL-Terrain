#include "skybox/skybox.h" // Include the header you just made

// OpenGL / GLEW
#include <GL/glew.h>
#include <GL/glu.h>

// Include the camera header to get its full definition
#include "camera/Camera.h"

// This tells stb_image to create the function implementations
// This MUST be done in exactly ONE .cpp file in your whole project
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// A static global variable. "static" here means it's only visible
// inside this .cpp file. This holds the OpenGL ID for our texture.
static GLuint skyboxTextureID;

void loadSkybox(const std::vector<std::string>& faces) {
    glGenTextures(1, &skyboxTextureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);

    int width, height, nrChannels;
    
    GLenum targets[] = {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X, // right
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X, // left
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y, // top
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, // bottom
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z, // front
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z  // back
    };

    stbi_set_flip_vertically_on_load(false);

    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            
            // --- THIS IS THE FIX ---
            // Detect the image format (RGB vs RGBA)
            GLenum format;
            if (nrChannels == 3)
                format = GL_RGB;
            else if (nrChannels == 4)
                format = GL_RGBA;
            else {
                printf("Unknown number of channels (%d) in image: %s\n", nrChannels, faces[i].c_str());
                stbi_image_free(data);
                continue; // Skip this texture
            }
            
            // Use the detected 'format' for both the internal format and the source format
            glTexImage2D(targets[i], 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            // --- END OF FIX ---

            stbi_image_free(data);
        } else {
            // This is the error you're looking for in Solution 1
            printf("Skybox texture failed to load: %s\n", faces[i].c_str());
            stbi_image_free(data);
        }
    }

    // These parameters are all correct
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

// --- THIS LINE IS MODIFIED ---
void drawSkybox(const Camera& camera, const glm::vec3& center) {
    // --- State Setup ---
    glDepthMask(GL_FALSE);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST); 
    
    // --- THIS IS THE FIX ---
    // Reset the color to white. This stops the skybox from being
    // tinted by whatever color was used to draw the player.
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    
    glEnable(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    // --- The Camera Trick (Corrected) ---
    glm::vec3 eye = camera.GetPosition();
    glm::vec3 up(0, 1, 0);                
    glm::vec3 lookDirection = center - eye;
    
    gluLookAt(0, 0, 0, 
              lookDirection.x, lookDirection.y, lookDirection.z, 
              up.x, up.y, up.z);
    
    // --- Draw the Cube ---
    float size = 100.0f;

    glBegin(GL_QUADS);
        // Positive X (Right)
        glTexCoord3f( 1.0f, -1.0f, -1.0f); glVertex3f( size, -size, -size);
        glTexCoord3f( 1.0f, -1.0f,  1.0f); glVertex3f( size, -size,  size);
        glTexCoord3f( 1.0f,  1.0f,  1.0f); glVertex3f( size,  size,  size);
        glTexCoord3f( 1.0f,  1.0f, -1.0f); glVertex3f( size,  size, -size);
        // Negative X (Left)
        glTexCoord3f(-1.0f, -1.0f,  1.0f); glVertex3f(-size, -size,  size);
        glTexCoord3f(-1.0f, -1.0f, -1.0f); glVertex3f(-size, -size, -size);
        glTexCoord3f(-1.0f,  1.0f, -1.0f); glVertex3f(-size,  size, -size);
        glTexCoord3f(-1.0f,  1.0f,  1.0f); glVertex3f(-size,  size,  size);
        // Positive Y (Top)
        glTexCoord3f(-1.0f, 1.0f, -1.0f); glVertex3f(-size,  size, -size);
        glTexCoord3f( 1.0f, 1.0f, -1.0f); glVertex3f( size,  size, -size);
        glTexCoord3f( 1.0f, 1.0f,  1.0f); glVertex3f( size,  size,  size);
        glTexCoord3f(-1.0f, 1.0f,  1.0f); glVertex3f(-size,  size,  size);
        // Negative Y (Bottom)
        glTexCoord3f(-1.0f, -1.0f,  1.0f); glVertex3f(-size, -size,  size);
        glTexCoord3f( 1.0f, -1.0f,  1.0f); glVertex3f( size, -size,  size);
        glTexCoord3f( 1.0f, -1.0f, -1.0f); glVertex3f( size, -size, -size);
        glTexCoord3f(-1.0f, -1.0f, -1.0f); glVertex3f(-size, -size, -1.0f);
        // Positive Z (Front)
        glTexCoord3f( 1.0f, -1.0f, 1.0f); glVertex3f( size, -size, size);
        glTexCoord3f(-1.0f, -1.0f, 1.0f); glVertex3f(-size, -size, size);
        glTexCoord3f(-1.0f,  1.0f, 1.0f); glVertex3f(-size,  size, size);
        glTexCoord3f( 1.0f,  1.0f, 1.0f); glVertex3f( size,  size, size);
        // Negative Z (Back)
        glTexCoord3f(-1.0f, -1.0f, -1.0f); glVertex3f(-size, -size, -1.0f);
        glTexCoord3f( 1.0f, -1.0f, -1.0f); glVertex3f( size, -size, -1.0f);
        glTexCoord3f( 1.0f,  1.0f, -1.0f); glVertex3f( size,  size, -1.0f);
        glTexCoord3f(-1.0f,  1.0f, -1.0f); glVertex3f(-size,  size, -1.0f);
    glEnd();

    // --- State Restore ---
    glPopMatrix(); 
    glDisable(GL_TEXTURE_CUBE_MAP);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING); 
    glDepthMask(GL_TRUE); 
}