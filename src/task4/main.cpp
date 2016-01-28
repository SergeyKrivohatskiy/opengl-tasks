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


int main() 
{
	GLFWwindow *window = init();
	if (!window)
	{
		return -1;
	}

	GLuint fill_buffers_shader = LoadShaders("task4_fill_buffers.vertexshader", "task4_fill_buffers.fragmentshader");
	GLuint lights_shader = LoadShaders("task4_lights.vertexshader", "task4_lights.fragmentshader");
	// Used to draw point light positions
	GLuint draw_lines_shader = LoadShaders("task4_lines.vertexshader", "task4_lines.fragmentshader");

	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	GLuint framebuffer_depth;
	glGenRenderbuffers(1, &framebuffer_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, framebuffer_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1300, 800);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebuffer_depth);

	GLuint base_color_texture = create_texture(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, base_color_texture, 0);

	GLuint normals_texture = create_texture(GL_RGB32F, GL_RGB, GL_FLOAT);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, normals_texture, 0);

	GLuint position_texture = create_texture(GL_RGB32F, GL_RGB, GL_FLOAT);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, position_texture, 0);

	GLuint attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	std::pair<std::vector<GLfloat>, std::vector<GLfloat>> vert_and_normals =
		load_scene("spider.obj");

	std::vector<GLfloat> &vertexes = vert_and_normals.first;
	std::vector<GLfloat> &normals = vert_and_normals.second;
	GLuint vertex_buffer, vertex_array_o;
	{
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
	}

	GLuint vao;
	{
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
			0.0f, 0.0f,
			1.0f, 0.0f,
			1.0f, 1.0f,
			1.0f, 1.0f,
			0.0f, 1.0f,
			0.0f, 0.0f
		};
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

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
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		// 2nd attribute buffer : uv's
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
		glBindVertexArray(0);
	}

	GLuint cross_vao;
	{
		static const GLfloat g_vertex_buffer_data[] =
		{
			-1.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, -1.0f,
			0.0f, 0.0f, 1.0f,
		};
		glGenVertexArrays(1, &cross_vao);
		glBindVertexArray(cross_vao);

		GLuint vertexbuffer;
		glGenBuffers(1, &vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
		
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	}

	typedef struct {
		vec3 center;
		vec3 color;
		GLfloat radius;
	} point_light;
	std::vector<point_light> point_lights(2);
	// Normalized normal
	vec3 directional_light(1, 0, 0);
	vec3 directional_light_color(1.00, 1.0, 0);
	
	mat4 proj = perspective(45.0f, float(1300) / float(800), 0.1f, 5.0f);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	while (!glfwWindowShouldClose(window))
	{
		double t = glfwGetTime();
		vec3 camepa_pos = vec3(sin(t * 0.3), sin(t * 0.912 * 0.3) / 3 + 0.6, cos(t * 0.3)) * 0.6f;
		mat4 view(lookAt(camepa_pos,
				vec3(0, 0, 0), vec3(0, 1, 0)));
		mat4 proj_view_mat(proj * view);

		point_lights[0] = { vec3(-0.2, 0.3, sin(t) * 0.2), 
				vec3(0.0, sin(t * 2) * 0.5 + 0.5, 0.0), 0.55f };
		point_lights[1] = { vec3(0.4, 0.2, cos(t * 0.3) * 0.2), 
				vec3(0.0, 0.0, 0.5 + sin(t) * 0.5), 0.4f };

		glfwPollEvents();
		{
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glUseProgram(fill_buffers_shader);
			glUniform1i(glGetUniformLocation(fill_buffers_shader, "base_color"), 0);
			glUniform1i(glGetUniformLocation(fill_buffers_shader, "normal"), 1);
			glUniform1i(glGetUniformLocation(fill_buffers_shader, "position"), 2);
			glUniformMatrix4fv(glGetUniformLocation(fill_buffers_shader, "proj_view_mat"),
				1, GL_FALSE, value_ptr(proj_view_mat));
			glBindVertexArray(vertex_array_o);
			glDrawArrays(GL_TRIANGLES, 0, vertexes.size() / 3);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glUseProgram(lights_shader);

			glUniform1i(glGetUniformLocation(lights_shader, "base_color"), 3);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, base_color_texture);

			glUniform1i(glGetUniformLocation(lights_shader, "normal"), 4);
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, normals_texture);

			glUniform1i(glGetUniformLocation(lights_shader, "position"), 5);
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, position_texture);


			glUniform3f(glGetUniformLocation(lights_shader, "directional_light"),
				directional_light.x, directional_light.y, directional_light.z);
			glUniform3f(glGetUniformLocation(lights_shader, "directional_light_color"),
				directional_light_color.x, directional_light_color.y, directional_light_color.z);

			glUniform3fv(glGetUniformLocation(lights_shader, "to_camera_modelspace"),
				1, value_ptr(normalize(camepa_pos)));
			for (size_t idx = 0; idx < point_lights.size(); ++idx)
			{
				point_light const &p_light = point_lights[idx];
				std::string idx_str(std::to_string(idx));
				glUniform3f(glGetUniformLocation(lights_shader,
					("points_center[" + idx_str + "]").c_str()),
					p_light.center.x, p_light.center.y, p_light.center.z);
				glUniform3f(glGetUniformLocation(lights_shader,
					("points_color[" + idx_str + "]").c_str()),
					p_light.color.x, p_light.color.y, p_light.color.z);
				glUniform1f(glGetUniformLocation(lights_shader,
					("points_radius[" + idx_str + "]").c_str()),
					p_light.radius);
			}
			glUniform1i(glGetUniformLocation(lights_shader,"lights_count"),
				point_lights.size());

			glBindVertexArray(vao);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		{
			glDisable(GL_DEPTH_TEST); // to draw all point ligths
			glUseProgram(draw_lines_shader);
			glUniformMatrix4fv(glGetUniformLocation(draw_lines_shader, "view_proj_mat"),
				1, GL_FALSE, value_ptr(proj_view_mat));

			glBindVertexArray(vao);

			for (point_light const &p_light : point_lights)
			{
				glUniform3f(glGetUniformLocation(draw_lines_shader, "center"),
					p_light.center.x, p_light.center.y, p_light.center.z);
				glUniform3f(glGetUniformLocation(draw_lines_shader, "color"),
					p_light.color.x, p_light.color.y, p_light.color.z);
				glUniform1f(glGetUniformLocation(draw_lines_shader, "radius"),
					p_light.radius);
				glBindVertexArray(cross_vao);
				glDrawArrays(GL_LINES, 0, 6);
			}
		}


		glfwSwapBuffers(window);
	}



	glfwTerminate();
	return 0;
}