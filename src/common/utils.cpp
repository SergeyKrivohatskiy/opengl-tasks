#include "utils.h"
#include <glm/glm.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <vector>
#include <limits>


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
	glViewport(0, 0, 1300, 800);
	return window;
}

std::pair<std::vector<GLfloat>, std::vector<GLfloat>> load_scene(std::string filename)
{
	Assimp::Importer importer;

	const aiScene *scene = importer.ReadFile(filename, aiProcessPreset_TargetRealtime_Quality);
	assert(scene);
	std::vector<GLfloat> vertexes;
	std::vector<GLfloat> normals;
	vec3 box_bottom(std::numeric_limits<float>::max(),
		std::numeric_limits<float>::max(),
		std::numeric_limits<float>::max());
	vec3 box_top(-box_bottom);

	for (size_t mesh_idx = 0; mesh_idx < scene->mNumMeshes; ++mesh_idx)
	{
		const aiMesh *mesh = scene->mMeshes[mesh_idx];
		assert(mesh->HasNormals());
		for (size_t face_idx = 0; face_idx < mesh->mNumFaces; ++face_idx)
		{
			const aiFace &face = mesh->mFaces[face_idx];
			// Should be triangulated on preprocessing. see aiProcessPreset_TargetRealtime_Quality
			assert(face.mNumIndices <= 3);
			if (face.mNumIndices != 3) {
				continue;
			}
			for (size_t v_idx = 0; v_idx < face.mNumIndices; ++v_idx)
			{
				size_t idx = face.mIndices[v_idx];
				vertexes.push_back(mesh->mVertices[idx].x);
				box_bottom.x = min(box_bottom.x, mesh->mVertices[idx].x);
				box_top.x = max(box_top.x, mesh->mVertices[idx].x);
				vertexes.push_back(mesh->mVertices[idx].y);
				box_bottom.y = min(box_bottom.y, mesh->mVertices[idx].y);
				box_top.y = max(box_top.y, mesh->mVertices[idx].y);
				vertexes.push_back(mesh->mVertices[idx].z);
				box_bottom.z = min(box_bottom.z, mesh->mVertices[idx].z);
				box_top.z = max(box_top.z, mesh->mVertices[idx].z);
				normals.push_back(mesh->mNormals[idx].x);
				normals.push_back(mesh->mNormals[idx].y);
				normals.push_back(mesh->mNormals[idx].z);
			}
		}
	}

	vec3 center = (box_top + box_bottom) / 2.0f;
	vec3 sizes = box_top - box_bottom;
	float multiplier = 1.0f / max(max(sizes.x, sizes.y), sizes.z);

	for (size_t idx = 0; idx < vertexes.size(); idx += 3)
	{
		vertexes[idx] = (vertexes[idx] - center.x) * multiplier;
		vertexes[idx + 1] = (vertexes[idx + 1] - center.y) * multiplier;
		vertexes[idx + 2] = (vertexes[idx + 2] - center.z) * multiplier;
	}


	return { vertexes, normals };
}

GLuint create_texture(GLint internal_format, GLenum format, GLenum type)
{
	GLuint texture_id;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, 1300, 800, 0, format, type, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	return texture_id;
}