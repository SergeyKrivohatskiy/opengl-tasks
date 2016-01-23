#pragma once
#include <gl/glew.h>
#include <gl/GL.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>

GLFWwindow* init();

// returns vertex array and normals array
std::pair<std::vector<GLfloat>, std::vector<GLfloat>> load_scene(std::string filename);