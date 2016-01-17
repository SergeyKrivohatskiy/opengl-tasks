#include "utils.h"
#include <glm/glm.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>


using namespace glm;

GLFWwindow* init()
{
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return nullptr;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	GLFWwindow* window = glfwCreateWindow(1300, 800, "Fractal", NULL, NULL);

	if (window == NULL)
	{
		fprintf(stderr, "Failed to open GLFW window\n");
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr, "Failed to initialize GLEW\n");
		return nullptr;
	}
	return window;
}