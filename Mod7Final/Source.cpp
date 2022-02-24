#include <iostream>         // Allow for input/output
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#include <string>

// GLM Libraries
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// STB Library to load an image
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Coordinates.h"  // Class to hold/retrieve object coordinates
#include "camera.h" // Camera class file originated from website LearnOpenGL.com

/*

This program uses the following keys to navigate the camera in a 3D scene:

    W : Move forward        Q : Move down
    S : Move back           E : Move up
    A : Move left           P : Change between Perspective/Orthographic view
    D : Move right

            **Scrolling the mouse change camera speed**

*/

using namespace std;

// Shader program Macro
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

namespace
{
    // Set window title
    const char* const WINDOW_TITLE = "Final 3D Scene By Paul K.";

    // window variables
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // Store mesh data
    struct GLMesh
    {
        GLuint vao[11];         // Handle for vertex array object
        GLuint vbo[11];         // Handle for  vertex buffer object
        GLuint nVertices[11];   // Number of indices of the mesh
    };

    // Store light data
    class GLLight
    {
    public:
        GLuint shaderProgram;     // Handle for shader program
        glm::vec3 lightPosition;  // Position of light in 3Dscene
        glm::vec3 lightScale;     // Scale of light 
        glm::vec3 lightColor;     // Color of light
        float lightIntensity;     //  Light intensity
        float highlightSize;
    };

    GLFWwindow* gWindow = nullptr;  // Declare new window object
    GLMesh gMesh;   // Triangle mesh data

    // Texture and scale
    GLuint texture1, texture2, texture3, texture4, texture5, texture6, texture7, texture8, texture9, texture10;
    glm::vec2 gUVScale(1.0f, 1.0f);

    // Vector to hold light data that is passed to CalcPointLight
    vector<GLLight> gSceneLights{
        { 0, glm::vec3(16.0f, 20.0f, -5.0f), glm::vec3(0.1f), glm::vec3(0.33f, 0.24f, 0.3f), 0.3f, 256.0f},
        { 0, glm::vec3(8.0f, 20.0f, 5.0f), glm::vec3(0.1f), glm::vec3(0.33f, 0.24f, 0.3f), 0.1f, 256.0f},
        { 0, glm::vec3(-8.0f, 20.0f, 5.0f), glm::vec3(0.1f), glm::vec3(0.33f, 0.24f, 0.3f), 0.1f, 256.0f},
        { 0, glm::vec3(-16.0f, 20.0f, -5.0f), glm::vec3(0.1f), glm::vec3(0.33f, 0.24f, 0.03f), 0.3f, 256.0f},
        { 0, glm::vec3(1.0f, 5.0f, 25.0f), glm::vec3(0.3f), glm::vec3(0.82f, 0.79f, 0.74f), 0.2f, 2.0f},
    };

    // Decalre Shader program object
    GLuint shaderProgramId;

    // Define camera position
    Camera gCamera(glm::vec3(0.0f, 30.0f, 40.0f));

    // Variables to ensure program runs the same on all hardware
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    float gDeltaTime = 0.0f;
    float gLastFrame = 0.0f;

    GLfloat scroll = 10.0f;     // Camera speed
    bool gFirstMouse = true;    // Detect initial mouse movement    
    bool perspective = true;    // boolean to change between perspective and orthographic
}

// Input fucntions 
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// Functions to create, compile, destroy the shader program, create and render primitives
void UCreateMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);

// Vertex Shader Source Code
const GLchar* vertexShaderSource = GLSL(440,
    // Declare attribute locations
    layout(location = 0) in vec3 position;          // Vertex position 
layout(location = 1) in vec3 normal;            // Normals
layout(location = 2) in vec2 textureCoordinate; // Textures

out vec3 vertexNormal;              // Outgoing normals to fragment shader
out vec3 vertexFragmentPos;         // Outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;   // Outgoing texture to fragment shader

// Uniform/Global variables for transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transform vertices to clip coordinates

    vertexFragmentPos = vec3(model * vec4(position, 1.0f));         // Get fragment / pixel position into world space only

    // Get normals in world space only (exclude normal translation properties)
    vertexNormal = mat3(transpose(inverse(model))) * normal;
    vertexTextureCoordinate = textureCoordinate;
}
);

// Fragment Shader Source Code
const GLchar* fragmentShaderSource = GLSL(440,

    in vec3 vertexNormal;              // Incoming normals
in vec3 vertexFragmentPos;         // Incoming fragment position
in vec2 vertexTextureCoordinate;   // Incoming texture coordinates

out vec4 fragmentColor;             // Outgoing color to GPU

// Uniform/Global variables for scene lights, view (camera) position, texture, and scale 
uniform vec3 lightColor1;
uniform vec3 lightPos1;             // Light 1
uniform float lightIntensity1;
uniform float highlightSize1;
uniform vec3 lightColor2;
uniform vec3 lightPos2;             // Light 2
uniform float lightIntensity2;
uniform float highlightSize2;
uniform vec3 lightColor3;
uniform vec3 lightPos3;             // Light 3
uniform float lightIntensity3;
uniform float highlightSize3;
uniform vec3 lightColor4;
uniform vec3 lightPos4;             // Light 4
uniform float lightIntensity4;
uniform float highlightSize4;
uniform vec3 lightColor5;
uniform vec3 lightPos5;             // Light 5
uniform float lightIntensity5;
uniform float highlightSize5;

uniform vec3 viewPosition;
uniform sampler2D uTexture;
uniform vec2 uvScale;

/*Phong lighting model calculations to generate ambient, diffuse, and specular components*/
vec3 CalcPointLight(vec3 lightPos, vec3 lightColor, float lightIntensity, vec3 vertexFragmentPos, vec3 viewPosition, float highlightSize)
{
    // Calculate Ambient lighting
    vec3 ambient = lightIntensity * lightColor;

    // Calculate Diffuse lighting
    vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
    vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance between light source and fragments/pixels
    float impact = max(dot(norm, lightDirection), 0.2);// Calculate diffuse impact
    vec3 diffuse = impact * lightColor;

    // Calculate Specular lighting
    float specularIntensity = 0.2f; // Set specular light strength
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
    vec3 specular = specularIntensity * specularComponent * lightColor;

    // Calculate phong result
    vec3 phong = (ambient + diffuse + specular);

    return phong;
}

void main()
{
    vec3 result = vec3(0.0);
    vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

    // Calculate lights
    result += CalcPointLight(lightPos1, lightColor1, lightIntensity1, vertexFragmentPos, viewPosition, highlightSize1) * textureColor.xyz;
    result += CalcPointLight(lightPos2, lightColor2, lightIntensity2, vertexFragmentPos, viewPosition, highlightSize2) * textureColor.xyz;
    result += CalcPointLight(lightPos3, lightColor3, lightIntensity3, vertexFragmentPos, viewPosition, highlightSize3) * textureColor.xyz;
    result += CalcPointLight(lightPos4, lightColor4, lightIntensity4, vertexFragmentPos, viewPosition, highlightSize4) * textureColor.xyz;
    result += CalcPointLight(lightPos5, lightColor5, lightIntensity5, vertexFragmentPos, viewPosition, highlightSize5) * textureColor.xyz;

    fragmentColor = vec4(result, 1.0); // Send results to GPU
}
);

// Lamp vertex Shader Source Code
const GLchar* lampVertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position;  // Declare attribute locations

    // Uniform/Global variables for transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices to clip coordinates
}
);

// Lamp fragment Shader Source Code
const GLchar* lampFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor;

void main()
{
    fragmentColor = vec4(1.0f); // Set color to white w/ alpha 1
}
);

// MAIN FUNCTION
int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow)) // Call function to initialize GLFW, GLEW, and create a window
        return EXIT_FAILURE;

    UCreateMesh(gMesh); // Call function to create VBO/VAOs

    // Create fucntion to create shader programs
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, shaderProgramId))
        return EXIT_FAILURE;
    for (int i = 0; i < gSceneLights.size(); i++)
    {
        if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gSceneLights[i].shaderProgram))
            return EXIT_FAILURE;  // Loop through vector to release shader program for lights
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);   // Set background color to black

    // Render loop (infinite loop until user closes window)
    while (!glfwWindowShouldClose(gWindow))
    {
        // Set delta time and ensure we are transforming at consistent rate
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // Activate/Bind texture unitS
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, texture2);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, texture3);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, texture4);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, texture5);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, texture6);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, texture7);
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, texture8);
        glActiveTexture(GL_TEXTURE9);
        glBindTexture(GL_TEXTURE_2D, texture9);
        glActiveTexture(GL_TEXTURE10);
        glBindTexture(GL_TEXTURE_2D, texture10);

        UProcessInput(gWindow); // Call fucntion to get input from user

        URender();              // Call function to render frame

        glfwPollEvents();       // Process events
    }

    UDestroyMesh(gMesh);          // Release mesh data 
    UDestroyTexture(texture1);    // Release texture data
    UDestroyTexture(texture2);
    UDestroyTexture(texture3);
    UDestroyTexture(texture4);
    UDestroyTexture(texture5);
    UDestroyTexture(texture6);
    UDestroyTexture(texture7);
    UDestroyTexture(texture8);
    UDestroyTexture(texture9);
    UDestroyTexture(texture10);
    UDestroyShaderProgram(shaderProgramId); // Release shader program 
    for (const GLLight light : gSceneLights)
    {
        UDestroyShaderProgram(light.shaderProgram);  // Loop through vector to release shader program for lights
    }

    exit(EXIT_SUCCESS); // Terminate the program successfully
}

// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // Initialize glfw library 
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create window
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(*window);  // Make context current for calling thread
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
   glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // Capture mouse - Normal cursor enabled (testing)
    // glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Disable cursor (testing)

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }
    return true;
}

// Function to process user keyboard input
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)  // Exit application if escape key pressed
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)       // If 'W' pressed, move camera forward (toward object)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)       // If 'S' pressed, move camera backward (away from object)	
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)       // If 'A' pressed, move camera left
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)       // If 'D' pressed, move camera right
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)       // If 'E' pressed, move camera up
        gCamera.ProcessKeyboard(UP, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)       // If 'Q' pressed, move camera down
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)       // If 'P' pressed, change projection matrix between perspective/ortho
        perspective = !perspective;
}

// Resize window and graphics simultaneously
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Function to capture mouse movement
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}

// Function to process mouse scroll (currently zooms)
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}

// Functioned called to render a frame
void URender()
{
    glEnable(GL_DEPTH_TEST);    // Allows for depth comparisons and to update the depth buffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);   // Clear the frame and z buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(gMesh.vao[0]);   // Activate the lights VAO  
    glUseProgram(shaderProgramId);  // Set the shader to be used

    // Initialize transformations
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 translation = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    glm::mat4 scale = glm::scale(glm::vec3(1.0f));
    glm::mat4 rotation = glm::rotate(0.0f, glm::vec3(0.0, 0.0f, 0.0f));

    // Create view matrix that transforms all world coordinates to view space
    glm::mat4 view = gCamera.GetViewMatrix();
    glm::mat4 projection = glm::mat4(1.0f);

    // Conditional loop allows user to change view of scene between orthographic (2D) and perspective (3D) views
    if (perspective)
    {
        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
    }
    else {
        float scale = 50;
        projection = glm::ortho(-((float)WINDOW_WIDTH / scale), (float)WINDOW_WIDTH / scale, -(float)WINDOW_HEIGHT / scale, ((float)WINDOW_HEIGHT / scale), 0.1f, 100.0f);
    }

    // Retrieve and pass transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(shaderProgramId, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgramId, "view");
    GLint projLoc = glGetUniformLocation(shaderProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //Draw lights
    for (int i = 0; i < gSceneLights.size(); i++)
    {   // Retrieve and pass color position, and intensity data to the Shader program
        GLint lightColorLoc = glGetUniformLocation(shaderProgramId, ("lightColor" + std::to_string(i + 1)).c_str());
        GLint lightPositionLoc = glGetUniformLocation(shaderProgramId, ("lightPos" + std::to_string(i + 1)).c_str());
        GLint lightIntensityLoc = glGetUniformLocation(shaderProgramId, ("lightIntensity" + std::to_string(i + 1)).c_str());
        GLint highlightSizeLoc = glGetUniformLocation(shaderProgramId, ("highlightSize" + std::to_string(i + 1)).c_str());

        glUniform3f(lightColorLoc, gSceneLights[i].lightColor[0], gSceneLights[i].lightColor[1], gSceneLights[i].lightColor[2]);
        glUniform3f(lightPositionLoc, gSceneLights[i].lightPosition[0], gSceneLights[i].lightPosition[1], gSceneLights[i].lightPosition[2]);
        glUniform1f(lightIntensityLoc, (gSceneLights[i].lightIntensity));
        glUniform1f(highlightSizeLoc, gSceneLights[i].highlightSize);
    }

    // Pass camera and scale data to the Shader program
    GLint viewPositionLoc = glGetUniformLocation(shaderProgramId, "viewPosition");
    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    GLint UVScaleLoc = glGetUniformLocation(shaderProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // Draw Milk Bottom
    model = glm::mat4(1.0f);
    scale = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));         // Scale 
    model = scale;
    modelLoc = glGetUniformLocation(shaderProgramId, "model");
    viewLoc = glGetUniformLocation(shaderProgramId, "view");
    projLoc = glGetUniformLocation(shaderProgramId, "projection");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform1i(glGetUniformLocation(shaderProgramId, "uTexture"), 2);  // Set texture as texture unit
    glBindVertexArray(gMesh.vao[2]);
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices[2]);

    // Draw Milk Top
    model = glm::mat4(1.0f);
    scale = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));         // Scale  
    model = scale;
    modelLoc = glGetUniformLocation(shaderProgramId, "model");
    viewLoc = glGetUniformLocation(shaderProgramId, "view");
    projLoc = glGetUniformLocation(shaderProgramId, "projection");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform1i(glGetUniformLocation(shaderProgramId, "uTexture"), 3);
    glBindVertexArray(gMesh.vao[3]);
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices[3]);

    // Draw cap top
    model = glm::mat4(1.0f);
    translation = glm::translate(model, glm::vec3(-3.35f, 11.0f, -2.8f)); // Translate, rotate, and scale cap
    rotation = glm::rotate(model, glm::degrees(6.1f), glm::vec3(1.0f, 0.0f, 0.0f));
    scale = glm::scale(model, glm::vec3(0.85f, 1.0f, 0.85f));
    model = translation * rotation * scale;
    modelLoc = glGetUniformLocation(shaderProgramId, "model");
    viewLoc = glGetUniformLocation(shaderProgramId, "view");
    projLoc = glGetUniformLocation(shaderProgramId, "projection");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform1i(glGetUniformLocation(shaderProgramId, "uTexture"), 7);
    glBindVertexArray(gMesh.vao[7]);
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices[7]);

    // Draw cap sides
    model = glm::mat4(1.0f);
    translation = glm::translate(model, glm::vec3(-3.35f, 11.0f, -2.8f));  // Translate, rotate, and scale
    rotation = glm::rotate(model, glm::degrees(6.1f), glm::vec3(1.0f, 0.0f, 0.0f));
    scale = glm::scale(model, glm::vec3(0.85f, 1.0f, 0.85f));
    model = translation * rotation * scale;
    modelLoc = glGetUniformLocation(shaderProgramId, "model");
    viewLoc = glGetUniformLocation(shaderProgramId, "view");
    projLoc = glGetUniformLocation(shaderProgramId, "projection");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform1i(glGetUniformLocation(shaderProgramId, "uTexture"), 8);
    glBindVertexArray(gMesh.vao[8]);
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices[8]);

    // Draw donut box
    model = glm::mat4(1.0f);
    translation = glm::translate(model, glm::vec3(5.0f, 0.0f, 2.0f));  // Translate and scale 
    scale = glm::scale(model, glm::vec3(0.7f, 0.6f, 0.7f));
    model = translation * scale;
    modelLoc = glGetUniformLocation(shaderProgramId, "model");
    viewLoc = glGetUniformLocation(shaderProgramId, "view");
    projLoc = glGetUniformLocation(shaderProgramId, "projection");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform1i(glGetUniformLocation(shaderProgramId, "uTexture"), 4);
    glBindVertexArray(gMesh.vao[4]);
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices[4]);

    // Draw donut 
    model = glm::mat4(1.0f);
    translation = glm::translate(model, glm::vec3(0.0f, 0.0f, 6.0f)); // Translate and scale 
    scale = glm::scale(model, glm::vec3(0.6f, 0.7f, 0.6f));
    model = translation * scale;
    modelLoc = glGetUniformLocation(shaderProgramId, "model");
    viewLoc = glGetUniformLocation(shaderProgramId, "view");
    projLoc = glGetUniformLocation(shaderProgramId, "projection");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform1i(glGetUniformLocation(shaderProgramId, "uTexture"), 9);
    glBindVertexArray(gMesh.vao[9]);
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices[9]);

    // Draw glass top
    model = glm::mat4(1.0f);
    translation = glm::translate(model, glm::vec3(-5.0f, 0.0f, 4.0f)); // Translate and scale 
    scale = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
    model = translation * scale;
    modelLoc = glGetUniformLocation(shaderProgramId, "model");
    viewLoc = glGetUniformLocation(shaderProgramId, "view");
    projLoc = glGetUniformLocation(shaderProgramId, "projection");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform1i(glGetUniformLocation(shaderProgramId, "uTexture"), 5);
    glBindVertexArray(gMesh.vao[5]);
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices[5]);

    // Draw glass sides
    model = glm::mat4(1.0f);
    translation = glm::translate(model, glm::vec3(-5.0f, 0.0f, 4.0f)); // Translate and scale 
    scale = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
    model = translation * scale;
    modelLoc = glGetUniformLocation(shaderProgramId, "model");
    viewLoc = glGetUniformLocation(shaderProgramId, "view");
    projLoc = glGetUniformLocation(shaderProgramId, "projection");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform1i(glGetUniformLocation(shaderProgramId, "uTexture"), 6);
    glBindVertexArray(gMesh.vao[6]);
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices[6]);

    // Draw Milk Plane
    model = glm::mat4(1.0f);
    scale = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));         // Scale 
    model = scale;
    modelLoc = glGetUniformLocation(shaderProgramId, "model");
    viewLoc = glGetUniformLocation(shaderProgramId, "view");
    projLoc = glGetUniformLocation(shaderProgramId, "projection");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform1i(glGetUniformLocation(shaderProgramId, "uTexture"), 10);
    glBindVertexArray(gMesh.vao[10]);
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices[10]);

    // Draw Plane
    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(shaderProgramId, "model");
    viewLoc = glGetUniformLocation(shaderProgramId, "view");
    projLoc = glGetUniformLocation(shaderProgramId, "projection");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform1i(glGetUniformLocation(shaderProgramId, "uTexture"), 1);
    glBindVertexArray(gMesh.vao[1]);
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices[1]);

    // Draw Lamps
    for (int i = 0; i < gSceneLights.size(); i++)
    {
        glUseProgram(gSceneLights[i].shaderProgram); // Activate shader program
        glBindVertexArray(gMesh.vao[0]);

        // Transform lights
        model = glm::translate(gSceneLights[i].lightPosition) * glm::scale(gSceneLights[i].lightScale);

        // Select uniform shader and variable
        modelLoc = glGetUniformLocation(gSceneLights[i].shaderProgram, "model");
        viewLoc = glGetUniformLocation(gSceneLights[i].shaderProgram, "view");
        projLoc = glGetUniformLocation(gSceneLights[i].shaderProgram, "projection");

        // Pass matrix data to Lamp Shader program
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        //glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices[0]); // Draws lamps (deactivated)
    }

    // Deactivate VAO and shader program
    glBindVertexArray(0);
    glUseProgram(0);
    glfwSwapBuffers(gWindow);    // Swap front and back buffers of window
}

/*Function holds object coordinates, generates/activates VAO/VBO,
create/enable Vertex Attribute Pointers, and loads texture to texture variable*/
void UCreateMesh(GLMesh& mesh)
{
    // Identify how many floats for Position, Normal, and Texture coordinates
    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    // lights
    std::vector<GLfloat>* lightCoords = Coordinates::getLightCoords();
    GLfloat lightVertices[108]{};
    for (int i = 0; i < lightCoords->size(); i++) {
        lightVertices[i] = lightCoords->at(i);
    }
    mesh.nVertices[0] = sizeof(lightVertices) / (sizeof(lightVertices[0]) * (floatsPerVertex));
    glGenVertexArrays(1, &mesh.vao[0]); // Create and bind Vertex Array Object (repeated for each obj)
    glBindVertexArray(mesh.vao[0]);
    glGenBuffers(1, &mesh.vbo[0]); // Create and activate Vertex Buffer Object (repeated for each obj)
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lightVertices), lightVertices, GL_STATIC_DRAW); // Send vertex data to the GPU (repeated for each obj)
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, sizeof(float) * floatsPerVertex, 0);
    glEnableVertexAttribArray(0);

    //Plane
    std::vector<GLfloat>* planeCoords = Coordinates::getPlaneCoords();
    GLfloat planeVertices[48]{};
    for (int i = 0; i < planeCoords->size(); i++) {
        planeVertices[i] = planeCoords->at(i);
    }
    mesh.nVertices[1] = sizeof(planeVertices) / (sizeof(planeVertices[1]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
    glGenVertexArrays(1, &mesh.vao[1]);
    glBindVertexArray(mesh.vao[1]);
    glGenBuffers(1, &mesh.vbo[1]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    // Create Vertex Attribute Pointers - position, normal, texture (repeated for each obj except plane)
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    //milk Bottom
    std::vector<GLfloat>* milkBottomCoords = Coordinates::getMilkBotCoords();
    GLfloat milkBottomVertices[192]{};
    for (int i = 0; i < milkBottomCoords->size(); i++) {
        milkBottomVertices[i] = milkBottomCoords->at(i);
    }
    mesh.nVertices[2] = sizeof(milkBottomVertices) / (sizeof(milkBottomVertices[1]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
    glGenVertexArrays(1, &mesh.vao[2]);
    glBindVertexArray(mesh.vao[2]);
    glGenBuffers(1, &mesh.vbo[2]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(milkBottomVertices), milkBottomVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    // milk top
    std::vector<GLfloat>* milkTopCoords = Coordinates::getMilkTopCoords();
    GLfloat milkTopVertices[144]{};
    for (int i = 0; i < milkTopCoords->size(); i++) {
        milkTopVertices[i] = milkTopCoords->at(i);
    }
    mesh.nVertices[3] = sizeof(milkTopVertices) / (sizeof(milkTopVertices[1]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
    glGenVertexArrays(1, &mesh.vao[3]);
    glBindVertexArray(mesh.vao[3]);
    glGenBuffers(1, &mesh.vbo[3]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(milkTopVertices), milkTopVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    // dontut box
    std::vector<GLfloat>* donutBoxCoords = Coordinates::getBoxCoords();
    GLfloat donutBoxVertices[240]{};
    for (int i = 0; i < donutBoxCoords->size(); i++) {
        donutBoxVertices[i] = donutBoxCoords->at(i);
    }
    mesh.nVertices[4] = sizeof(donutBoxVertices) / (sizeof(donutBoxVertices[1]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
    glGenVertexArrays(1, &mesh.vao[4]);
    glBindVertexArray(mesh.vao[4]);
    glGenBuffers(1, &mesh.vbo[4]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[4]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(donutBoxVertices), donutBoxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    // Glass top
    std::vector<GLfloat>* glassTopCoords = Coordinates::getGlassTopcoords();
    GLfloat glassTopVertices[192]{};
    for (int i = 0; i < glassTopCoords->size(); i++) {
        glassTopVertices[i] = glassTopCoords->at(i);
    }
    mesh.nVertices[5] = sizeof(glassTopVertices) / (sizeof(glassTopVertices[1]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
    glGenVertexArrays(1, &mesh.vao[5]);
    glBindVertexArray(mesh.vao[5]);
    glGenBuffers(1, &mesh.vbo[5]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[5]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glassTopVertices), glassTopVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    // glass side getGlassSideCoords
    std::vector<GLfloat>* glassSideCoords = Coordinates::getGlassSideCoords();
    GLfloat glassSideVertices[384]{};
    for (int i = 0; i < glassSideCoords->size(); i++) {
        glassSideVertices[i] = glassSideCoords->at(i);
    }
    mesh.nVertices[6] = sizeof(glassSideVertices) / (sizeof(glassSideVertices[1]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
    glGenVertexArrays(1, &mesh.vao[6]);
    glBindVertexArray(mesh.vao[6]);
    glGenBuffers(1, &mesh.vbo[6]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[6]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glassSideVertices), glassSideVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    // cap Top
    std::vector<GLfloat>* capTopCoords = Coordinates::getCapTopCoords();
    GLfloat capTopVertices[192]{};
    for (int i = 0; i < capTopCoords->size(); i++) {
        capTopVertices[i] = capTopCoords->at(i);
    }
    mesh.nVertices[7] = sizeof(capTopVertices) / (sizeof(capTopVertices[1]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
    glGenVertexArrays(1, &mesh.vao[7]);
    glBindVertexArray(mesh.vao[7]);
    glGenBuffers(1, &mesh.vbo[7]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[7]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(capTopVertices), capTopVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    // cap side  
    std::vector<GLfloat>* capSideCoords = Coordinates::getCapSideCoords();
    GLfloat capSideVertices[384]{};
    for (int i = 0; i < capSideCoords->size(); i++) {
        capSideVertices[i] = capSideCoords->at(i);
    }
    mesh.nVertices[8] = sizeof(capSideVertices) / (sizeof(capSideVertices[1]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
    glGenVertexArrays(1, &mesh.vao[8]);
    glBindVertexArray(mesh.vao[8]);
    glGenBuffers(1, &mesh.vbo[8]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[8]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(capSideVertices), capSideVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    // donut  
    std::vector<GLfloat>* donutCoords = Coordinates::getDonutCoords();
    GLfloat donutVertices[3072]{};
    for (int i = 0; i < donutCoords->size(); i++) {
        donutVertices[i] = donutCoords->at(i);
    }
    mesh.nVertices[9] = sizeof(donutVertices) / (sizeof(donutVertices[1]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
    glGenVertexArrays(1, &mesh.vao[9]);
    glBindVertexArray(mesh.vao[9]);
    glGenBuffers(1, &mesh.vbo[9]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[9]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(donutVertices), donutVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    // milk plane  
    std::vector<GLfloat>* milkPlaneCoords = Coordinates::getMilkPlaneCoords();
    GLfloat milkPlaneVertices[48]{};
    for (int i = 0; i < milkPlaneCoords->size(); i++) {
        milkPlaneVertices[i] = milkPlaneCoords->at(i);
    }
    mesh.nVertices[10] = sizeof(milkPlaneVertices) / (sizeof(milkPlaneVertices[1]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
    glGenVertexArrays(1, &mesh.vao[10]);
    glBindVertexArray(mesh.vao[10]);
    glGenBuffers(1, &mesh.vbo[10]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[10]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(milkPlaneVertices), milkPlaneVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    // Call function to generate textures passing texture image files
    const char* texFilename = "plane1.jpg";
    if (!UCreateTexture(texFilename, texture1))
    {
        cout << "Failed to load texture " << texFilename << endl;
    }
    texFilename = "milkCarton.jpg";
    if (!UCreateTexture(texFilename, texture2))
    {
        cout << "Failed to load texture " << texFilename << endl;
    }
    texFilename = "milkTop.jpg";
    if (!UCreateTexture(texFilename, texture3))
    {
        cout << "Failed to load texture " << texFilename << endl;
    }
    texFilename = "DonutBox1.jpg";
    if (!UCreateTexture(texFilename, texture4))
    {
        cout << "Failed to load texture " << texFilename << endl;
    }
    texFilename = "glassTop8.jpg";
    if (!UCreateTexture(texFilename, texture5))
    {
        cout << "Failed to load texture " << texFilename << endl;
    }
    texFilename = "milkSide.jpg";
    if (!UCreateTexture(texFilename, texture6))
    {
        cout << "Failed to load texture " << texFilename << endl;
    }
    texFilename = "capTop.jpg";
    if (!UCreateTexture(texFilename, texture7))
    {
        cout << "Failed to load texture " << texFilename << endl;
    }
    texFilename = "capSide.jpg";
    if (!UCreateTexture(texFilename, texture8))
    {
        cout << "Failed to load texture " << texFilename << endl;
    }
    texFilename = "test5.jpg";
    if (!UCreateTexture(texFilename, texture10))
    {
        cout << "Failed to load texture " << texFilename << endl;
    }
    texFilename = "donut1.png";
    if (!UCreateTexture(texFilename, texture9))
    {
        cout << "Failed to load texture " << texFilename << endl;
    }
}

// Function to destroy VAO and VBO
void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, mesh.vao);
    glDeleteBuffers(1, mesh.vbo);
}

// Function to load and bind texture
bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        stbi_set_flip_vertically_on_load(true); // Flip y-axis during image loading so that image is not upside down

        glGenTextures(1, &textureId);               // Create texture ID
        glBindTexture(GL_TEXTURE_2D, textureId);    // Bind texure 

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);       // Specify how to wrap texture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   // Specify how to filter texture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);    // Generate all  required mipmaps for currently bound texture

        stbi_image_free(image);             // Free image memory
        glBindTexture(GL_TEXTURE_2D, 0);    // Unbind the texture

        return true;                        // Image loaded and texture bound successfully
    }

    return false;   // Error loading the image
}

// Function to destroy texture
void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}

// Function to create shader program
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create shader program object
    programId = glCreateProgram();

    // Create  vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile vertex shader and print compilation errors
    glCompileShader(vertexShaderId);
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        return false;
    }
    // Compile fragment shader and print compilation errors
    glCompileShader(fragmentShaderId);
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    // Link shader program and print linking errors
    glLinkProgram(programId);
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        return false;
    }

    glUseProgram(programId);    // Use shader program
    return true;
}

// Function to destroy shader program
void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}