#include "material.h"

#include "application.h"

#include <istream>
#include <fstream>
#include <algorithm>


glm::vec3 Material::GetInverseCameraPos(Camera* camera, glm::mat4 model){
	if (use_local_pos) {
		//Compute camera position in local coordinates
		glm::mat4 inverseModel = glm::inverse(model);
		glm::vec4 temp = glm::vec4(camera->eye, 1.0);
		temp = inverseModel * temp;
		return glm::vec3(temp.x / temp.w, temp.y / temp.w, temp.z / temp.w);
	}
	else {
		return camera->eye;
	}
}

FlatMaterial::FlatMaterial(glm::vec4 color)
{
	this->color = color;
	this->shader = Shader::Get("res/shaders/basic.vs", "res/shaders/flat.fs");
}

FlatMaterial::~FlatMaterial() { }

void FlatMaterial::setUniforms(Camera* camera, glm::mat4 model)
{
	//upload node uniforms
	this->shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	this->shader->setUniform("u_camera_position", camera->eye);
	this->shader->setUniform("u_model", model);

	this->shader->setUniform("u_color", this->color);
}

void FlatMaterial::render(Mesh* mesh, glm::mat4 model, Camera* camera)
{
	if (mesh && this->shader) {
		// enable shader
		this->shader->enable();

		// upload uniforms
		setUniforms(camera, model);

		// do the draw call
		mesh->render(GL_TRIANGLES);

		this->shader->disable();
	}
}

void FlatMaterial::renderInMenu()
{
	ImGui::ColorEdit3("Color", (float*)&this->color);
}

WireframeMaterial::WireframeMaterial()
{
	this->color = glm::vec4(1.f);
	this->shader = Shader::Get("res/shaders/basic.vs", "res/shaders/flat.fs");
}

WireframeMaterial::~WireframeMaterial() { }

void WireframeMaterial::render(Mesh* mesh, glm::mat4 model, Camera* camera)
{
	if (this->shader && mesh)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_CULL_FACE);

		//enable shader
		this->shader->enable();

		//upload material specific uniforms
		setUniforms(camera, model);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		glEnable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

StandardMaterial::StandardMaterial(glm::vec4 color)
{
	this->color = color;
	this->base_shader = Shader::Get("res/shaders/basic.vs", "res/shaders/basic.fs");
	this->normal_shader = Shader::Get("res/shaders/basic.vs", "res/shaders/normal.fs");
	this->shader = this->base_shader;
}

StandardMaterial::~StandardMaterial() { }

void StandardMaterial::setUniforms(Camera* camera, glm::mat4 model)
{
	//upload node uniforms
	this->shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	this->shader->setUniform("u_camera_position", camera->eye);
	this->shader->setUniform("u_model", model);

	this->shader->setUniform("u_color", this->color);

	if (this->texture) {
		this->shader->setUniform("u_texture", this->texture);
	}
}

void StandardMaterial::render(Mesh* mesh, glm::mat4 model, Camera* camera)
{
	bool first_pass = true;
	if (mesh && this->shader)
	{
		// enable shader
		this->shader->enable();

		// Multi pass render
		int num_lights = Application::instance->light_list.size();
		for (int nlight = -1; nlight < num_lights; nlight++)
		{
			if (nlight == -1) { nlight++; } // hotfix

			// upload uniforms
			setUniforms(camera, model);

			// upload light uniforms
			if (!first_pass) {
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				glDepthFunc(GL_LEQUAL);
			}
			this->shader->setUniform("u_ambient_light", Application::instance->ambient_light * (float)first_pass);

			if (num_lights > 0) {
				Light* light = Application::instance->light_list[nlight];
				light->setUniforms(this->shader, model);
			}
			else {
				// Set some uniforms in case there is no light
				this->shader->setUniform("u_light_intensity", 1.f);
				this->shader->setUniform("u_light_shininess", 1.f);
				this->shader->setUniform("u_light_color", glm::vec4(0.f));
			}

			// do the draw call
			mesh->render(GL_TRIANGLES);

			first_pass = false;
		}

		// disable shader
		this->shader->disable();
	}
}

void StandardMaterial::renderInMenu()
{
	if (ImGui::Checkbox("Show Normals", &this->show_normals)) {
		if (this->show_normals) {
			this->shader = this->normal_shader;
		}
		else {
			this->shader = this->base_shader;
		}
	}

	if (!this->show_normals) ImGui::ColorEdit3("Color", (float*)&this->color);
}


/// Volume Material
VolumeMaterial::VolumeMaterial(glm::vec4 background_color_) {
	this->color = glm::vec4(1.f, 1.f, 1.f, 0.8f);
	this->background_color = background_color_;

	this->emitted_color = glm::vec4(1.f, 0.8f, 0.2f, 1.f);
	this->absorption_coefficient = 0.955f;
	this->step_length = 0.004f;
	this->noise_detail = 5;
	this->noise_scale = 1.54f;
	this->emitted_intensity = 1;

	this->shader_type = ABSORPTION;
	this->volume_type = HOMOGENEOUS;

	this->absorption_shader = Shader::Get("res/shaders/basic.vs", "res/shaders/absorption.fs");
	this->emissive_absorption_shader = Shader::Get("res/shaders/basic.vs", "res/shaders/emissive_absorption.fs");

	this->use_local_pos = true;
	assignShader();
}

VolumeMaterial::~VolumeMaterial() { }

//This three functions have to be addapted to volume material
void VolumeMaterial::setUniforms(Camera* camera, glm::mat4 model)
{
	//upload node uniforms
	this->shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	glm::vec3 camera_pos = GetInverseCameraPos(camera, model);
	this->shader->setUniform("u_camera_position", camera_pos);
	this->shader->setUniform("u_model", model);
	this->shader->setUniform("u_background_color", this->background_color);
	this->shader->setUniform("u_step_length", this->step_length);
	this->shader->setUniform("u_absorption_coefficient", this->absorption_coefficient);
	if (this->shader_type == eShaderType::EMISSION_ABSORPTION || this->volume_type == eVolumeType::HOTEROGENEOUS) {
		this->shader->setUniform("u_noise_scale", this->noise_scale);
		this->shader->setUniform("u_noise_detail", this->noise_detail);
	}
	this->shader->setUniform("u_use_noise", this->volume_type == eVolumeType::HOTEROGENEOUS);

	if (this->shader_type == eShaderType::EMISSION_ABSORPTION) {
		this->shader->setUniform("u_emitted_color", this->emitted_color);
		this->shader->setUniform("u_emitted_intensity", this->emitted_intensity);
	}

	this->shader->setUniform("u_color", this->color);

	if (this->texture) {
		this->shader->setUniform("u_texture", this->texture);
	}
}

void VolumeMaterial::render(Mesh* mesh, glm::mat4 model, Camera* camera)
{
	if (mesh && this->shader) {

		glEnable(GL_BLEND); //Since it has alpha lower than 1, but since is the only object renderized it wouldn't be necessary
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

		// enable shader
		this->shader->enable();

		// upload uniforms
		setUniforms(camera, model);

		// do the draw call
		mesh->render(GL_TRIANGLES);

		this->shader->disable();

		glDisable(GL_BLEND);
	}
}

void VolumeMaterial::renderInMenu()
{
	if (ImGui::Combo("Shader Type", (int*)&shader_type, "ABSORPTION\0EMISSION_ABSORPTION\0")) {
		assignShader();
	}
	if (this->shader_type == eShaderType::EMISSION_ABSORPTION) {
		ImGui::ColorEdit3("Emitted color", (float*)&this->emitted_color);
		ImGui::SliderInt("Emitted intensity", (int*)&this->emitted_intensity, 1, 20);
	}
	ImGui::SliderFloat("Absorbsion Coeficient", (float*)&this->absorption_coefficient, 0.001f, 3.0f);
	ImGui::Combo("Volume Type", (int*)&volume_type, "HOMOGENEOUS\0HOTEROGENEOUS\0");
	if (!(this->volume_type == HOMOGENEOUS && this->shader_type == eShaderType::ABSORPTION)) {
		ImGui::SliderFloat("Step Lenght", (float*)&this->step_length, 0.001f, 0.2f);
	}
	if (this->volume_type == eVolumeType::HOTEROGENEOUS) {
		ImGui::SliderFloat("Noise Scale", (float*)&this->noise_scale, 0.001f, 5.0f);
		ImGui::SliderFloat("Noise Detail", (float*)&this->noise_detail, 1.0f, 5.0f);
	}

}

void VolumeMaterial::assignShader() 
{
	if (this->shader_type == ABSORPTION) {
		this->shader = this->absorption_shader;
	}
	else if (this->shader_type == EMISSION_ABSORPTION) {
		this->shader = this->emissive_absorption_shader;
	}
}
