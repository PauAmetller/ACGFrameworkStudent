#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/matrix.hpp>

#include "../framework/camera.h"
#include "mesh.h"
#include "texture.h"
#include "shader.h"

class Material {
public:

	Shader* shader = NULL;
	Texture* texture = NULL;
	glm::vec4 color;
	bool use_local_pos = true;

	glm::vec3 GetInverseCameraPos(Camera* camera, glm::mat4 model);

	virtual void setUniforms(Camera* camera, glm::mat4 model) = 0;
	virtual void render(Mesh* mesh, glm::mat4 model, Camera* camera) = 0;
	virtual void renderInMenu() = 0;
};

class FlatMaterial : public Material {
public:

	FlatMaterial(glm::vec4 color = glm::vec4(1.f));
	~FlatMaterial();

	void setUniforms(Camera* camera, glm::mat4 model);
	void render(Mesh* mesh, glm::mat4 model, Camera* camera);
	void renderInMenu();
};

class WireframeMaterial : public FlatMaterial {
public:

	WireframeMaterial();
	~WireframeMaterial();

	void render(Mesh* mesh, glm::mat4 model, Camera* camera);
};

class StandardMaterial : public Material {
public:

	bool first_pass = false;

	bool show_normals = false;
	Shader* base_shader = NULL;
	Shader* normal_shader = NULL;

	StandardMaterial(glm::vec4 color = glm::vec4(1.f));
	~StandardMaterial();

	void setUniforms(Camera* camera, glm::mat4 model);
	void render(Mesh* mesh, glm::mat4 model, Camera* camera);
	void renderInMenu();
};

class VolumeMaterial : public Material
{
public:

	enum eShaderType { ABSORPTION, EMISSION_ABSORPTION};
	enum eVolumeType { HOMOGENEOUS , HOTEROGENEOUS};

	eShaderType shader_type;
	eVolumeType volume_type;

	//Homogenous
	glm::vec4 background_color;
	float absorption_coefficient;

	//Heterogeneous
	float step_length;
	float noise_scale;
	float noise_detail;

	//Emission-absorption
	glm::vec4 emitted_color;
	int emitted_intensity;


	Shader* absorption_shader;
	Shader* emissive_absorption_shader;

	VolumeMaterial(glm::vec4 background_color_);
	~VolumeMaterial();

	void setUniforms(Camera* camera, glm::mat4 model);
	void render(Mesh* mesh, glm::mat4 model, Camera* camera);
	void renderInMenu();

	void assignShader();
};