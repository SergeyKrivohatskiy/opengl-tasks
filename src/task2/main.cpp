#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <gl/glew.h>
#include <gl/GL.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <AntTweakBar.h>
#include "../common/shader.hpp"
#include "../common/utils.h"

using namespace glm;

int main() 
{
	GLFWwindow *window = init();
	if (!window)
	{
		return -1;
	}



	glfwTerminate();
	return 0;
}