// Include standard headers
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

using namespace glm;

float scale = 2.2f;
float offset_x = -1.6f;
float offset_y = -1.1f;
int max_iterations = 20;

void mouse_callback(GLFWwindow *window, double, double y_offset)
{
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	int xsize, ysize;
	glfwGetWindowSize(window, &xsize, &ysize);
	xpos = xpos / xsize;
	ypos = ypos / ysize;
	double scale_mult = pow(1.1, -y_offset);
	offset_x = static_cast<float>(offset_x - xpos * scale * (scale_mult - 1.0));
	offset_y = static_cast<float>(offset_y - ypos * scale * (scale_mult - 1.0));
	scale = static_cast<float>(scale * scale_mult);

}

int main(void)
{
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
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
		return -1;
	}
	glfwMakeContextCurrent(window);

	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	GLuint programID = LoadShaders("task1.vertexshader", "task1.fragmentshader");
	//GLuint programID = LoadShaders("task1Serpinski.vertexshader", "task1Serpinski.fragmentshader");
	static const GLfloat g_vertex_buffer_data[] =
	{
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f
	};
	static const GLfloat g_uv_buffer_data[] =
	{
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f
	};

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	// 2nd attribute buffer : uv's
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glfwSetScrollCallback(window, mouse_callback);

	static const size_t TEXTURE_WIDTH = 128;
	GLuint texture_id = 0;
	glBindTexture(GL_TEXTURE_1D, texture_id);
	{
		std::vector<vec3> texture;
		texture.reserve(TEXTURE_WIDTH);
		for (size_t idx = 0; idx < TEXTURE_WIDTH; ++idx)
		{
			float r = idx / static_cast<float>(TEXTURE_WIDTH);
			texture.push_back(vec3(r, 1.0 - r, r * r));
		}
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, TEXTURE_WIDTH, 0, GL_RGB, GL_FLOAT, &texture.front());
	}
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	do
	{
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(programID);
		GLint loc;
		loc = glGetUniformLocation(programID, "max_iterations");
		assert(loc != -1);
		glUniform1i(loc, max_iterations);

		loc = glGetUniformLocation(programID, "texture_sampler");
		glUniform1i(loc, texture_id);
		loc = glGetUniformLocation(programID, "scale");
		assert(loc != -1);
		glUniform1f(loc, scale);
		loc = glGetUniformLocation(programID, "offset");
		assert(loc != -1);
		glUniform2f(loc, offset_x, offset_y);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glfwSwapBuffers(window);
		glfwPollEvents();
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		{
			max_iterations += 1;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		{
			max_iterations = std::max(1, max_iterations - 1);
		}
	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glDeleteBuffers(1, &vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteProgram(programID);

	glfwTerminate();

	return 0;
}