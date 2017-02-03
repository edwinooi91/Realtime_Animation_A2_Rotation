#include <iostream>
#include <cmath>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// assimp
#include <assimp/cimport.h>

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

// Other includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

using namespace std;

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void do_movement();
glm::mat4 toEuler(GLfloat yaw, GLfloat pitch, GLfloat roll);
glm::mat4 toQuaternion(GLfloat x, GLfloat y, GLfloat z);

// Camera
Camera  camera(glm::vec3(0.0f, 0.0f, 3.0f));
GLfloat lastX  =  400;
GLfloat lastY  =  300;
bool firstMouse = true;
bool keys[1024];

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

//Rotation angle
bool euler = true;
GLfloat Yaw, Pitch, Roll = 0.0f;
GLfloat deltaRot = glm::radians(0.5);

glm::quat quaternion;
GLfloat x, y, z;
GLfloat detlaAngle = glm::radians(20.0f);
glm::mat4 modelMatrix;

bool firstPerson = true;

// The MAIN function, from here we start the application and run the game loop
int main()
{
    // Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint( GLFW_RESIZABLE, GL_FALSE );
    
    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Realtime Hatching", nullptr, nullptr);
    if(window == nullptr){
        cout << "Failed to open GLFW window." << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    
    // GLFW Options
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL Function pointers
    if(glewInit() != GLEW_OK){
        cout << "Failed to initialize GLEW" << endl;
        return -1;
    }
    
    // OpenGL options
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_DEPTH_TEST);
    
    Shader shader("diffuse.vs", "diffuse.frag");
    Model plane("Heli/heli.obj");
    
    // Game loop
    while(!glfwWindowShouldClose(window)) {
        // Calculate deltatime of current frame
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        glfwPollEvents();
        do_movement();
        
        glClearColor(0.45f, 0.78f, 0.9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        
        shader.Use();
        
        glm::mat4 projection = glm::perspective( camera.GetZoom( ), ( float )SCREEN_WIDTH/( float )SCREEN_HEIGHT, 0.1f, 100.0f );

        glm::mat4 view;
        if(firstPerson)
            view = camera.GetViewMatrix();
        
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        
        if(euler)
            modelMatrix = toEuler(Yaw, Pitch, Roll);
        else{
            glm::mat4 model;
            model = toQuaternion(x, y, z);
            x = y = z = 0.0f;
            modelMatrix *= model;
        }
      
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
        plane.Draw(shader);
        
        glfwSwapBuffers(window);
    }
    glfwTerminate();
    return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if ( GLFW_KEY_ESCAPE == key && GLFW_PRESS == action )
        glfwSetWindowShouldClose(window, GL_TRUE);
    
    if ( key >= 0 && key < 1024 )
        if ( action == GLFW_PRESS )
            keys[key] = true;
        else if ( action == GLFW_RELEASE )
            keys[key] = false;
}

void do_movement()
{
    // Camera controls
    if (keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (keys[GLFW_KEY_U]){
        if(euler)
            Yaw += deltaRot;
        else
            x += detlaAngle;
    }
    if (keys[GLFW_KEY_J]){
        if(euler)
            Yaw -= deltaRot;
        else
            x -= detlaAngle;

    }
    if (keys[GLFW_KEY_I]){
        if(euler)
            Pitch += deltaRot;
        else
            y += detlaAngle;
    }
    if (keys[GLFW_KEY_K]){
        if(euler)
            Pitch -= deltaRot;
        else
            y -= detlaAngle;
    }
    if (keys[GLFW_KEY_O]){
        if(euler)
            Roll += deltaRot;
        else
            z += detlaAngle;
    }
    if (keys[GLFW_KEY_L]){
        if(euler)
            Roll -= deltaRot;
        else
            z -= detlaAngle;
    }
    if(keys[GLFW_KEY_1]){
        Yaw = Pitch = Roll = 0.0f;
        euler = true;
    }
    if(keys[GLFW_KEY_2]){
        x = 0; y = 0; z = 0;
        euler = false;
    }
    if(keys[GLFW_KEY_3]){
        camera.changeView();
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if(firstPerson){
        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }
        
        GLfloat xoffset = xpos - lastX;
        GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left
        
        lastX = xpos;
        lastY = ypos;
        
        camera.ProcessMouseMovement(xoffset, yoffset);
    }
}

glm::mat4 toEuler(GLfloat yaw, GLfloat pitch, GLfloat roll){
    glm::mat4 m;
    m[0][0] = glm::cos(yaw)*glm::cos(pitch);
    m[0][1] = glm::cos(pitch)*glm::sin(yaw);
    m[0][2] = -1*glm::sin(pitch);
    m[1][0] = glm::cos(yaw)*glm::sin(roll)*glm::sin(pitch) - glm::cos(roll)*glm::sin(yaw);
    m[1][1] = glm::cos(roll)*glm::cos(yaw) + glm::sin(roll)*glm::sin(yaw)*glm::sin(pitch);
    m[1][2] = glm::cos(pitch)*glm::sin(roll);
    m[2][0] = glm::sin(roll)*glm::sin(yaw) + glm::cos(roll)*glm::cos(yaw)*glm::sin(pitch);
    m[2][1] = glm::cos(roll)*glm::sin(yaw)*glm::sin(pitch) - glm::cos(yaw)*glm::sin(roll);
    m[2][2] = glm::cos(roll)*glm::cos(pitch);
    
    return m;
}

glm::mat4 toQuaternion(GLfloat x, GLfloat y, GLfloat z){
    glm::quat p, q, r, s;
    p.x = 1 * glm::sin(glm::radians(x/2));
    p.y = 0 * glm::sin(glm::radians(x/2));
    p.z = 0 * glm::sin(glm::radians(x/2));
    p.w = glm::cos(glm::radians(x/2));
    
    q.x = 0 * glm::sin(glm::radians(y/2));
    q.y = 1 * glm::sin(glm::radians(y/2));
    q.z = 0 * glm::sin(glm::radians(y/2));
    q.w = glm::cos(glm::radians(y/2));
    
    s = glm::cross(p, q);
    
    r.x = 0 * glm::sin(glm::radians(z/2));
    r.y = 0 * glm::sin(glm::radians(z/2));
    r.z = 1 * glm::sin(glm::radians(z/2));
    r.w = glm::cos(glm::radians(z/2));

    s = glm::cross(s, r);
    
    GLfloat norm = sqrt(s.x*s.x + s.y*s.y + s.z*s.z + s.w*s.w);
    s.x =  s.x / norm;
    s.y =  s.y / norm;
    s.z =  s.z / norm;
    s.w =  s.w / norm;
    
    glm::mat4 m = glm::mat4_cast(s);
    return m;
}
