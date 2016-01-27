#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <gl/glew.h>
#include <gl/GL.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <AntTweakBar.h>
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4996) //4996 for _CRT_SECURE_NO_WARNINGS
#endif
#include <gli/gli.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include "../common/shader.hpp"
#include "../common/utils.h"

static GLfloat const skybox_v[] = {
	-1.0f, 1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, 1.0f, -1.0f,
	-1.0f, 1.0f, -1.0f,

	-1.0f, -1.0f, 1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f, 1.0f, -1.0f,
	-1.0f, 1.0f, -1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f, -1.0f, 1.0f,

	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, -1.0f, 1.0f,
	-1.0f, -1.0f, 1.0f,

	-1.0f, 1.0f, -1.0f,
	1.0f, 1.0f, -1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f, 1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f, 1.0f,
	1.0f, -1.0f, 1.0f
};

using namespace glm;

void load_texture(GLint idx, gli::texture const &texture)
{
	glTexImage2D(idx, 0, GL_RGB,
		texture.dimensions().x,
		texture.dimensions().y,
		0, GL_RGB, GL_UNSIGNED_BYTE, texture.data(0, 0, 0));
}

GLuint load_cubemap(std::string const &name_prefix)
{
	GLuint texture_id;
	glGenTextures(1, &texture_id);
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
	gli::texture texture_p_x = gli::load(name_prefix + "+x.dds");
	assert(!texture_p_x.empty());
	gli::texture texture_p_y = gli::load(name_prefix + "+y.dds");
	assert(!texture_p_y.empty());
	gli::texture texture_p_z = gli::load(name_prefix + "+z.dds");
	assert(!texture_p_z.empty());
	gli::texture texture_m_x = gli::load(name_prefix + "-x.dds");
	assert(!texture_m_x.empty());
	gli::texture texture_m_y = gli::load(name_prefix + "-y.dds");
	assert(!texture_m_y.empty());
	gli::texture texture_m_z = gli::load(name_prefix + "-z.dds");
	assert(!texture_m_z.empty());
	load_texture(GL_TEXTURE_CUBE_MAP_POSITIVE_X, texture_p_x);
	load_texture(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, texture_p_y);
	load_texture(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, texture_p_z);
	load_texture(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, texture_m_x);
	load_texture(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, texture_m_y);
	load_texture(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, texture_m_z);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return texture_id;
}


int main() 
{
	GLFWwindow *window = init();
	if (!window)
	{
		return -1;
	}

	GLuint skybox_shader = LoadShaders("task2_skybox.vertexshader", "task2_skybox.fragmentshader");
	GLuint scene_shader = LoadShaders("task2_scene.vertexshader", "task2_scene.fragmentshader");

	GLuint skybox_vao, skybox_vbo;
	glGenVertexArrays(1, &skybox_vao);
	glGenBuffers(1, &skybox_vbo);
	glBindVertexArray(skybox_vao);
	glBindBuffer(GL_ARRAY_BUFFER, skybox_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_v), &skybox_v, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
	glBindVertexArray(0);

	GLuint skybox_texture = load_cubemap("SantaMariaDeiMiracoli");
	if (!skybox_texture)
	{
		return -5;
	}

	std::pair<std::vector<GLfloat>, std::vector<GLfloat>> vert_and_normals = 
			load_scene("spider.obj");

	std::vector<GLfloat> &vertexes = vert_and_normals.first;
	std::vector<GLfloat> &normals = vert_and_normals.second;
	GLuint vertex_buffer, vertex_array_o;
	glGenVertexArrays(1, &vertex_array_o);
	glGenBuffers(1, &vertex_buffer);
	glBindVertexArray(vertex_array_o);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, vertexes.size() * sizeof(GLfloat), vertexes.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
	GLuint norm_buffer;
	glGenBuffers(1, &norm_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, norm_buffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, norm_buffer);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
	glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	while (!glfwWindowShouldClose(window))
	{
		double t = glfwGetTime();
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glfwPollEvents();
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mat4 proj = glm::perspective(45.0f,
			float(1300) / float(800), 0.1f, 100.0f);
		mat4 view(lookAt(vec3(sin(t), sin(t * 0.912), cos(t)) * 0.6f, vec3(0, 0, 0), vec3(0, 1, 0)));
		glUseProgram(scene_shader);
		glUniformMatrix4fv(glGetUniformLocation(scene_shader, "view_proj_mat"),
			1, GL_FALSE, value_ptr(proj * view));
		glBindVertexArray(vertex_array_o);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);

		glDrawArrays(GL_TRIANGLES, 0, vertexes.size() / 3);

		glUseProgram(skybox_shader);
		glUniformMatrix4fv(glGetUniformLocation(skybox_shader, "view_proj_mat"),
			1, GL_FALSE, value_ptr(proj * view));
		glBindVertexArray(skybox_vao);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		glfwSwapBuffers(window);
	}



	glfwTerminate();
	return 0;
}