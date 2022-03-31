#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

const char vertex_shader[] = "#version 330 core\nlayout (location = 0) in vec3 position;layout (location = 1) in vec3 rgb;uniform mat4 transform;uniform mat4 projection;uniform mat4 view;out vec3 RGB;void main(){gl_Position = projection * view * transform * vec4(position.xzy, 1.0f);RGB = rgb;}";
const char fragment_shader[] = "#version 330 core\nout vec4 color;in vec3 RGB;void main(){color = vec4(RGB.xyz, 1.0f);}";

const float obj_space_x_min = -10.0f, obj_space_x_max = 10.0f;
const float obj_space_y_min = -10.0f, obj_space_y_max = 10.0f;

const float moveSpeed = 3.0f;

float lastX, lastY;

vec3 cameraFrnt = {0.0f, 0.0f, -1.0f};
float yaw = M_PI / 2.0f;
float pitch = 0;

typedef struct Vertex { float x, y, z, r, g, b; } Vertex;

float Paraboloid(float x, float y);

float Wingle(float x, float y);

float Tube(float x, float y);

Vertex* genVertex(float (*f)(float,float), const float Δx, const float Δy, size_t* size, const float x_min, const float x_max, const float y_min, const float y_max);

unsigned int genVBO(Vertex* polygons, size_t size);

unsigned int genShader(const char* vs, const char* fs);

void walk(GLFWwindow* window, float* cameraPos, float* cameraFront, float* cameraUp, float Δt);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);

int main(int argc, char** argv)
{
    const int width = 640, height = 480;

    lastX = width / 2;
    lastY = height/ 2;

    GLFWwindow* window;

    if (!glfwInit()) { return -1; }

    window = glfwCreateWindow(width, height, "Graph3D", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);

    if(glewInit() != GLEW_OK) { return -1; }

    size_t bytes;
    Vertex* polygons;

    if(argc == 2)
    {
        if(!strcmp("Wigle", argv[1])) { polygons = genVertex(Wingle, 0.1f, 0.1f, &bytes, obj_space_x_min, obj_space_x_max, obj_space_y_min, obj_space_y_max); }
        else if(!strcmp("Paraboloid", argv[1])) { polygons = genVertex(Paraboloid, 0.1f, 0.1f, &bytes, obj_space_x_min, obj_space_x_max, obj_space_y_min, obj_space_y_max); }
        else if(!strcmp("Tube", argv[1])) { polygons = genVertex(Tube, 0.1f, 0.1f, &bytes, obj_space_x_min, obj_space_x_max, obj_space_y_min, obj_space_y_max); }
        else { polygons = genVertex(Wingle, 0.1f, 0.1f, &bytes, obj_space_x_min, obj_space_x_max, obj_space_y_min, obj_space_y_max); }
    }
    else { polygons = genVertex(Wingle, 0.1f, 0.1f, &bytes, obj_space_x_min, obj_space_x_max, obj_space_y_min, obj_space_y_max); }

    unsigned int VBO = genVBO(polygons, bytes);

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3*sizeof(float)));
    glBindVertexArray(0);

    unsigned int shader = genShader(vertex_shader, fragment_shader);

    glEnable(GL_DEPTH_TEST);

    glUseProgram(shader);
    glBindVertexArray(VAO);

    unsigned int transform_uniform_location = glGetUniformLocation(shader, "transform");
    unsigned int projection_uniform_location = glGetUniformLocation(shader, "projection");
    unsigned int view_uniform_location = glGetUniformLocation(shader, "view");

    vec3 cameraPos =  {0.0f, 0.0f,  0.0f};
    vec3 cameraUp =   {0.0f, 1.0f,  0.0f};

    float lastFrame = 0.0f;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
    glfwSetCursorPosCallback(window, mouse_callback); 

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mat4 transform = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
        
        mat4 projection;
        glm_perspective(M_PI/2.0f, width/(float)height, 0.1f, 100.f, projection);

        mat4 view;
        vec3 tmp;
        glm_vec3_add(cameraPos, cameraFrnt, tmp);
        glm_lookat(cameraPos, tmp, cameraUp, view);

        glUniformMatrix4fv(transform_uniform_location, 1, GL_FALSE, (float*)transform);
        glUniformMatrix4fv(projection_uniform_location, 1, GL_FALSE, (float*)projection);
        glUniformMatrix4fv(view_uniform_location, 1, GL_FALSE, (float*)view);

        glDrawArrays(GL_TRIANGLES, 0, bytes/sizeof(Vertex));

        glfwSwapBuffers(window);

        glfwPollEvents();

        float now = glfwGetTime();
        walk(window, cameraPos, cameraFrnt, cameraUp, now - lastFrame);
        lastFrame = now;
    }

    free(polygons);

    glfwTerminate();

    return 0;
}

float Paraboloid(float x, float y) { return x*x + y*y - 1.0f; }

float Wingle(float x, float y) { return sinf(x) + sinf(y) - 1.0f; }

float Tube(float x, float y)
{
    if(x == 0 || y == 0)
    {
        return -1000.f;
    }

    return -(5.0f/(x*x+y*y));
}

Vertex* genVertex(float (*f)(float,float), const float Δx, const float Δy, size_t* size, const float x_min, const float x_max, const float y_min, const float y_max)
{
    srand(0x123);

    *size = (size_t)ceilf(fabsf(x_max - x_min) / Δx) * (size_t)ceilf(fabsf(y_max - y_min) / Δy) * 6UL * sizeof(Vertex);

    Vertex* polygons = (Vertex*)malloc(*size);

    unsigned int vertex = 0;

    for(float y = y_min; y < (y_max - Δy); y += Δy)
    {
        for(float x = x_min; x < (x_max - Δx); x += Δx)
        {
            float r = (rand() % 255) / 256.0f;
            float g = (rand() % 255) / 256.0f;
            float b = (rand() % 255) / 256.0f;
            polygons[vertex] = (Vertex){x, y, f(x, y), r, g, b};
            polygons[vertex + 1] = (Vertex){x + Δx, y, f(x + Δx, y), r, g, b};
            polygons[vertex + 2] = (Vertex){x, y + Δy, f(x, y + Δy), r, g, b};

            
            polygons[vertex + 3] = (Vertex){x + Δx, y + Δy, f(x + Δx, y + Δy), r, g, b};
            polygons[vertex + 4] = polygons[vertex + 1];
            polygons[vertex + 5] = polygons[vertex + 2];
            
            vertex += 6;
        }
    }

    return polygons;
}

unsigned int genVBO(Vertex* polygons, size_t size)
{
    unsigned int vbo;
    glGenBuffers(1, &vbo);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, size, polygons, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return vbo;
}

unsigned int genShader(const char* vs, const char* fs)
{
    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vs, NULL);
    glCompileShader(vertex);

    {
        GLint success;
        GLchar infoLog[1024];
        glGetProgramiv(vertex, GL_LINK_STATUS, &success);
        if(!success) { glGetShaderInfoLog(vertex, 1024, NULL, infoLog); printf("%s", infoLog); }
    }

    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fs, NULL);
    glCompileShader(fragment);

    {
        GLint success;
        GLchar infoLog[1024];
        glGetProgramiv(fragment, GL_LINK_STATUS, &success);
        if(!success) { glGetShaderInfoLog(fragment, 1024, NULL, infoLog); printf("%s", infoLog); }
    }

    unsigned int shader = glCreateProgram();
    glAttachShader(shader, vertex);
    glAttachShader(shader, fragment);
    glLinkProgram(shader);

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return shader;
}

void walk(GLFWwindow* window, float* cameraPos, float* cameraFront, float* cameraUp, float Δt)
{

    float walkspeed = moveSpeed * Δt;

    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) { walkspeed *= 2.0f; }

    if(glfwGetKey(window, GLFW_KEY_W))
    {
        vec3 tmp;
        glm_vec3_scale(cameraFront, walkspeed, tmp);
        glm_vec3_add(tmp, cameraPos, cameraPos);
    }
    if(glfwGetKey(window, GLFW_KEY_S))
    {
        vec3 tmp;
        glm_vec3_scale(cameraFront, walkspeed, tmp);
        glm_vec3_sub(cameraPos, tmp, cameraPos);
    }
    if(glfwGetKey(window, GLFW_KEY_A))
    {
        vec3 tmp;
        glm_vec3_cross(cameraFront, cameraUp, tmp);
        glm_vec3_normalize(tmp);
        glm_vec3_scale(tmp, walkspeed, tmp);
        glm_vec3_sub(cameraPos, tmp, cameraPos);
    }
    if(glfwGetKey(window, GLFW_KEY_D))
    {
        vec3 tmp;
        glm_vec3_cross(cameraFront, cameraUp, tmp);
        glm_vec3_normalize(tmp);
        glm_vec3_scale(tmp, walkspeed, tmp);
        glm_vec3_add(cameraPos, tmp, cameraPos);
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f) { pitch = 89.0f; }
    if(pitch < -89.0f) { pitch = -89.0f; }

    vec3 direction;
    
    direction[0] = cosf(glm_rad(yaw)) * cosf(glm_rad(pitch));
    direction[1] = sinf(glm_rad(pitch));
    direction[2] = sinf(glm_rad(yaw)) * cosf(glm_rad(pitch));
    glm_vec3_normalize_to(direction, cameraFrnt);

}  