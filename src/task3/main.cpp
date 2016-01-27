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

using namespace glm;

GLint FILTER = GL_LINEAR_MIPMAP_LINEAR;
bool DYNAMIC = false;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (action != GLFW_PRESS)
	{
		return;
	}
	switch (key)
	{
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, GL_TRUE);
		return;
	case GLFW_KEY_1:
		FILTER = GL_NEAREST;
		return;
	case GLFW_KEY_2:
		FILTER = GL_LINEAR;
		return;
	case GLFW_KEY_3:
		FILTER = GL_LINEAR_MIPMAP_LINEAR;
		return;
	case GLFW_KEY_4:
		DYNAMIC = !DYNAMIC;
		return;
	}
}

int main() 
{
	GLFWwindow *window = init();
	if (!window)
	{
		return -1;
	}
	glfwSetKeyCallback(window, key_callback);

	GLuint projector_shader = 
		LoadShaders("task3_projector.vertexshader", "task3_projector.fragmentshader");
	GLuint scene_shader =
		LoadShaders("task3_scene.vertexshader", "task3_scene.fragmentshader");
	GLuint camera_shader =
		LoadShaders("task3_camera.vertexshader", "task3_camera.fragmentshader");


	gli::texture texture_smile = gli::load("smile.dds");
	GLuint smile_tex;
	glGenTextures(1, &smile_tex);
	// Assign texture to ID
	glBindTexture(GL_TEXTURE_2D, smile_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_smile.dimensions().x,
		texture_smile.dimensions().y,
		0, GL_RGB, GL_UNSIGNED_BYTE, texture_smile.data(0, 0, 0));

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

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
	
	GLuint projector_buffer;
	glGenFramebuffers(1, &projector_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, projector_buffer);
	GLuint projector_texture;
	glGenTextures(1, &projector_texture);
	glBindTexture(GL_TEXTURE_2D, projector_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1300, 800, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	GLuint projector_depth;
	glGenRenderbuffers(1, &projector_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, projector_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1300, 800);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, projector_depth);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, projector_texture, 0);

	GLuint attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	vec3 near_tr(1, 1, 0.1f);
	vec3 near_tl(1, -1, 0.1f);
	vec3 near_br(-1, 1, 0.1f);
	vec3 near_bl(-1, -1, 0.1f);
	vec3 far_tr(1, 1, 5);
	vec3 far_tl(1, -1, 5);
	vec3 far_br(-1, 1, 5);
	vec3 far_bl(-1, -1, 5);

	vec3 camera_lines[24] = {
		// near rect
		near_tr, near_tl,
		near_tr, near_br,
		near_tl, near_bl,
		near_br, near_bl,

		// far rect
		far_tr, far_tl,
		far_tr, far_br,
		far_tl, far_bl,
		far_br, far_bl,

		//near to far
		near_tr, far_tr,
		near_tl, far_tl,
		near_br, far_br,
		near_bl, far_bl
	};

	GLuint cam_vao, cam_vbo;

	glGenVertexArrays(1, &cam_vao);
	glGenBuffers(1, &cam_vbo);

	glBindVertexArray(cam_vao);
	glBindBuffer(GL_ARRAY_BUFFER, cam_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(camera_lines), &camera_lines, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(0);

	glEnable(GL_DEPTH_TEST);
	mat4 proj_cam = perspective(45.0f,
		float(1300) / float(800), 0.1f, 5.0f);
	mat4 proj_projector = glm::perspective(6.0f,
		float(1300) / float(800), 0.1f, 5.0f);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	while (!glfwWindowShouldClose(window))
	{
		double t = glfwGetTime();
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glfwPollEvents();

		mat4 view_cam(lookAt(vec3(sin(t), sin(t * 0.912) / 3 + 0.6, cos(t)) * 0.6f, 
				vec3(0, 0, 0), vec3(0, 1, 0)));
		mat4 view_projector(lookAt(vec3(0, 1, 0) * 0.6f,
			vec3(0, 0, 0), vec3(sin(t / 8.2), 0, cos(t / 8.2))));
		vec3 to_projector(0, 1, 0);
		mat4 view_proj_cam = proj_cam * view_cam;
		mat4 view_proj_projector = proj_projector * view_projector;

		glBindFramebuffer(GL_FRAMEBUFFER, projector_buffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(projector_shader);
		glUniform1i(glGetUniformLocation(projector_shader, "output_buffer"), 0);
		glUniformMatrix4fv(glGetUniformLocation(projector_shader, "view_proj_mat"),
			1, GL_FALSE, value_ptr(view_proj_cam));
		glBindVertexArray(vertex_array_o);
		glDrawArrays(GL_TRIANGLES, 0, vertexes.size() / 3);
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(scene_shader);
		glUniformMatrix4fv(glGetUniformLocation(scene_shader, "view_proj_mat"),
			1, GL_FALSE, value_ptr(view_proj_cam));
		glUniformMatrix4fv(glGetUniformLocation(scene_shader, "view_proj_mat_projector"),
			1, GL_FALSE, value_ptr(view_proj_projector));
		glUniform3f(glGetUniformLocation(scene_shader, "to_projector"),
			to_projector.x, to_projector.y, to_projector.z);
		glBindVertexArray(vertex_array_o);
		GLint tex_loc = glGetUniformLocation(scene_shader, "projector_texture");
		glUniform1i(tex_loc, 3);
		glActiveTexture(GL_TEXTURE3);
		if (DYNAMIC)
		{
			glBindTexture(GL_TEXTURE_2D, projector_texture);
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, FILTER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, FILTER);
		} else {
			glBindTexture(GL_TEXTURE_2D, smile_tex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, FILTER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, FILTER);
		}
		glDrawArrays(GL_TRIANGLES, 0, vertexes.size() / 3);
		glBindVertexArray(0);
		glActiveTexture(GL_TEXTURE0);


		glUseProgram(camera_shader);

		glUniformMatrix4fv(glGetUniformLocation(camera_shader, "view_proj_mat"), 
				1, GL_FALSE, value_ptr(view_proj_cam));
		glUniformMatrix4fv(glGetUniformLocation(camera_shader, "view_proj_mat_projector_inv"), 
				1, GL_FALSE, value_ptr(inverse(view_proj_projector)));
		glBindVertexArray(cam_vao);
		glDrawArrays(GL_LINES, 0, 24);

		glfwSwapBuffers(window);
	}



	glfwTerminate();
	return 0;
}