//
//  main.cpp
//  ModelViewer
//
//  Created by Mattias Bergström on 2014-01-21.
//  Copyright (c) 2014 Mattias Bergström. All rights reserved.
//

#include <iostream>
#include <fstream>
#include "glm/glm.hpp"
#include <cmath>
#include "Mesh.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <OpenGL/glu.h>
#include "OFFReader.h"
#include "ShaderLoader.h"
#include "SimpleModel.h"
#include "SceneRenderer.h"
#include "Constants.h"
#include "GeometryShader.h"

SceneRenderer renderer;
glm::vec3 move;
float rotationY;
float rotationYaw;

static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    renderer.updateResolution(width, height);
}

void updateInput(GLFWwindow* window) {
    glm::vec3 velocity;
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        velocity.z += cosf(rotationY * M_PI / 180);
        velocity.x -= sinf(rotationY * M_PI / 180);
        
    } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        velocity.z -= cosf(rotationY * M_PI / 180);
        velocity.x += sinf(rotationY * M_PI / 180);
    }
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        velocity.z += sinf(rotationY * M_PI / 180);
        velocity.x += cosf(rotationY * M_PI / 180);
    } else if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        velocity.z -= sinf(rotationY * M_PI / 180);
        velocity.x -= cosf(rotationY * M_PI / 180);
    }
    
    if(glm::length(velocity) > 0) {
        velocity = glm::normalize(velocity)*0.1f;
        
        move += velocity;
    }
    
    if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        rotationY -= 3.0f;
    } else if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        rotationY += 3.0f;
    }
    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        rotationYaw -= 3.0f;
    } else if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        rotationYaw += 3.0;
    }
}

void initGL() {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
}

GLFWwindow* initGLWindow() {
    GLFWwindow* window;
    
    
    glfwSetErrorCallback(error_callback);
    
    if (!glfwInit())
        exit(EXIT_FAILURE);
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    window = glfwCreateWindow(854, 480, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    // Initialize GLEW
    glewExperimental=true; // Needed in core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        exit(EXIT_FAILURE);
    }
    
    return window;
}

int main(void)
{
    GLFWwindow* window = initGLWindow();
    
    initGL();
    
    std::ifstream file1;
    file1.open("/Users/mattiasbergstrom/Desktop/cooldragon.off");
    
    if(!file1.is_open()) {
        std::cerr << "Could not open file" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    std::ifstream file2;
    file2.open("/Users/mattiasbergstrom/Desktop/cooldragon.off");
    
    if(!file2.is_open()) {
        std::cerr << "Could not open file" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    std::shared_ptr<GeometryShader> simpleShader(new GeometryShader);
    std::shared_ptr<GeometryShader> newShader(new GeometryShader);

    simpleShader->init();
    newShader->init();
    
    std::shared_ptr<Mesh> mesh1 = std::make_shared<Mesh>(OFFReader::read(file1));
    std::shared_ptr<Mesh> mesh2 = std::make_shared<Mesh>(OFFReader::read(file2));
    std::shared_ptr<SimpleModel> test = std::shared_ptr<SimpleModel>(new SimpleModel());
    std::shared_ptr<SimpleModel> dragon = std::shared_ptr<SimpleModel>(new SimpleModel());
    
    mesh1->material.diffuse = glm::vec4(1.0f, 0.6f, 5.0f, 1.0f);
    mesh2->material.diffuse = glm::vec4(0.7f, 0.5f, 0.8f, 1.0f);
    mesh1->material.ambient = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
    mesh2->material.ambient = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
    mesh1->material.specular = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
    mesh2->material.specular = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
    mesh1->material.shininess = 0.8f;
    mesh2->material.shininess = 0.2f;
    
    dragon->init(mesh1, simpleShader);
    dragon->position = glm::vec3(0.0f, -0.2f, -2.0f);
    
    
    test->init(mesh2, newShader);
    test->position = glm::vec3(1.0f, 0.0f, -4.0f);
    
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    
    // Init renderer and add nodes
    
    renderer.init(width, height);
    renderer.nodes.push_back(dragon);
    renderer.nodes.push_back(test);
    
    while (!glfwWindowShouldClose(window))
    {
        updateInput(window);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glfwGetFramebufferSize(window, &width, &height);
        float ratio = width / (float)height;
        
        renderer.proj = glm::perspective(75.0f, ratio, 1.0f, 1000.0f);
        
        // Rotation Up/Down
        renderer.view = glm::rotate(glm::mat4(1.0f), rotationYaw, glm::vec3(1.0f, 0.0f, 0.0f));
        
        // Rotation Y
        renderer.view = glm::rotate(renderer.view, rotationY, glm::vec3(0.0f, 1.0f, 0.0f));
        renderer.view = glm::translate(renderer.view, move);
        renderer.renderScene();
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwDestroyWindow(window);
    
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
