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

void Material::loadVDB(std::string file_path)
{
	easyVDB::OpenVDBReader* vdbReader = new easyVDB::OpenVDBReader();
	vdbReader->read(file_path);

	// now, read the grid from the vdbReader and store the data in a 3D texture
	estimate3DTexture(vdbReader);
}

void Material::estimate3DTexture(easyVDB::OpenVDBReader* vdbReader)
{
	int resolution = 128;
	float radius = 2.0;

	int convertedGrids = 0;
	int convertedVoxels = 0;

	int totalGrids = vdbReader->gridsSize;
	int totalVoxels = totalGrids * pow(resolution, 3);

	float resolutionInv = 1.0f / resolution;
	int resolutionPow2 = pow(resolution, 2);
	int resolutionPow3 = pow(resolution, 3);

	// read all grids data and convert to texture
	for (unsigned int i = 0; i < totalGrids; i++) {
		easyVDB::Grid& grid = vdbReader->grids[i];
		float* data = new float[resolutionPow3];
		memset(data, 0, sizeof(float) * resolutionPow3);

		// Bbox
		easyVDB::Bbox bbox = easyVDB::Bbox();
		bbox = grid.getPreciseWorldBbox();
		glm::vec3 target = bbox.getCenter();
		glm::vec3 size = bbox.getSize();
		glm::vec3 step = size * resolutionInv;

		grid.transform->applyInverseTransformMap(step);
		target = target - (size * 0.5f);
		grid.transform->applyInverseTransformMap(target);
		target = target + (step * 0.5f);

		int x = 0;
		int y = 0;
		int z = 0;

		for (unsigned int j = 0; j < resolutionPow3; j++) {
			int baseX = x;
			int baseY = y;
			int baseZ = z;
			int baseIndex = baseX + baseY * resolution + baseZ * resolutionPow2;

			if (target.x >= 40 && target.y >= 40.33 && target.z >= 10.36) {
				int a = 0;
			}

			float value = grid.getValue(target);

			int cellBleed = radius;

			if (cellBleed) {
				for (int sx = -cellBleed; sx < cellBleed; sx++) {
					for (int sy = -cellBleed; sy < cellBleed; sy++) {
						for (int sz = -cellBleed; sz < cellBleed; sz++) {
							if (x + sx < 0.0 || x + sx >= resolution ||
								y + sy < 0.0 || y + sy >= resolution ||
								z + sz < 0.0 || z + sz >= resolution) {
								continue;
							}

							int targetIndex = baseIndex + sx + sy * resolution + sz * resolutionPow2;

							float offset = std::max(0.0, std::min(1.0, 1.0 - std::hypot(sx, sy, sz) / (radius / 2.0)));
							float dataValue = offset * value * 255.f;

							data[targetIndex] += dataValue;
							data[targetIndex] = std::min((float)data[targetIndex], 255.f);
						}
					}
				}
			}
			else {
				float dataValue = value * 255.f;

				data[baseIndex] += dataValue;
				data[baseIndex] = std::min((float)data[baseIndex], 255.f);
			}

			convertedVoxels++;

			if (z >= resolution) {
				break;
			}

			x++;
			target.x += step.x;

			if (x >= resolution) {
				x = 0;
				target.x -= step.x * resolution;

				y++;
				target.y += step.y;
			}

			if (y >= resolution) {
				y = 0;
				target.y -= step.y * resolution;

				z++;
				target.z += step.z;
			}

			// yield
		}

		// now we create the texture with the data
		// use this: https://www.khronos.org/opengl/wiki/OpenGL_Type
		// and this: https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexImage3D.xhtml
		this->texture = new Texture();
		this->texture->create3D(resolution, resolution, resolution, GL_RED, GL_FLOAT, false, data, GL_R8);
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
	this->absorption_coefficient = 1.467f;
	this->step_length = 0.04f;
	this->noise_detail = 5;
	this->noise_scale = 1.54f;
	this->emitted_intensity = 1;
	this->scaterring_coefficient = 1.f;
	this->Henyey_Greenstein_g = 0.f;

	this->shaderType = eShaderType::ABSORPTION;
	this->densityType = eDensityType::CONSTANT;

	this->absorption_shader = Shader::Get("res/shaders/basic.vs", "res/shaders/absorption.fs");
	this->emissive_absorption_shader = Shader::Get("res/shaders/basic.vs", "res/shaders/emissive_absorption.fs");
	this->emissive_scatter_absorption_shader = Shader::Get("res/shaders/basic.vs", "res/shaders/emissive_scatter_absorption.fs");

	this->use_jittering = false;
	this->use_phase_function = false;
	this->use_local_pos = true;
	this->assignShader();
}

VolumeMaterial::VolumeMaterial(glm::vec4 background_color_, std::string file_path) {
	this->color = glm::vec4(1.f, 1.f, 1.f, 0.8f);
	this->background_color = background_color_;

	this->emitted_color = glm::vec4(1.f, 0.8f, 0.2f, 1.f);
	this->absorption_coefficient = 1.467f;
	this->step_length = 0.04f;
	this->noise_detail = 5;
	this->noise_scale = 1.54f;
	this->emitted_intensity = 1;
	this->scaterring_coefficient = 1.f;
	this->Henyey_Greenstein_g = 0.f;

	this->shaderType = eShaderType::ABSORPTION;
	this->densityType = eDensityType::CONSTANT;

	this->absorption_shader = Shader::Get("res/shaders/basic.vs", "res/shaders/absorption.fs");
	this->emissive_absorption_shader = Shader::Get("res/shaders/basic.vs", "res/shaders/emissive_absorption.fs");
	this->emissive_scatter_absorption_shader = Shader::Get("res/shaders/basic.vs", "res/shaders/emissive_scatter_absorption.fs");

	this->use_jittering = false;
	this->use_phase_function = false;
	this->use_local_pos = true;
	this->assignShader();

	this->loadVDB(file_path);
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

	this->shader->setUniform("u_density_type", (int)this->densityType);
	this->shader->setUniform("u_use_jittering", this->use_jittering);

	if (this->densityType == eDensityType::NOISE_3D) {
		this->shader->setUniform("u_noise_scale", this->noise_scale);
		this->shader->setUniform("u_noise_detail", this->noise_detail);
	}

	if (!(this->shaderType == eShaderType::ABSORPTION)) {
		this->shader->setUniform("u_emitted_color", this->emitted_color);
		this->shader->setUniform("u_emitted_intensity", this->emitted_intensity);
	}

	if (this->densityType == eDensityType::VDB_FILE) {
		if (this->texture) {
			this->shader->setUniform("u_texture", this->texture, 0);
		}
	}

	if (this->shaderType == eShaderType::EMISSION_SCATTER_ABSORPTION) {
		this->shader->setUniform("u_scattering_coefficient", this->scaterring_coefficient);
		this->shader->setUniform("u_use_phase_function", this->use_phase_function);
		if (this->use_phase_function) {
			this->shader->setUniform("u_g", this->Henyey_Greenstein_g);
		}
		Application::instance->light_list[0]->setUniforms(this->shader, model);
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
	if (ImGui::Combo("Shader Type", (int*)&shaderType, "ABSORPTION\0EMISSION_ABSORPTION\0EMISSION_SCATTER_ABSORPTION\0")) {
		this->assignShader();
	}

	if (!(this->densityType == CONSTANT && this->shaderType == eShaderType::ABSORPTION)) {
		ImGui::SliderFloat("Step Lenght", (float*)&this->step_length, 0.001f, 0.2f);
		ImGui::Checkbox("Use jittering filter", &this->use_jittering);
	}

	ImGui::SliderFloat("Absorbsion Coeficient", (float*)&this->absorption_coefficient, 0.001f, 3.0f);

	if (!(this->shaderType == eShaderType::ABSORPTION)) {
		ImGui::ColorEdit3("Emitted color", (float*)&this->emitted_color);
		ImGui::SliderInt("Emitted intensity", (int*)&this->emitted_intensity, 1, 20);
	}

	if (this->shaderType == eShaderType::EMISSION_SCATTER_ABSORPTION) {
		ImGui::SliderFloat("Scattering Coeficient", (float*)&this->scaterring_coefficient, 0.001f, 3.0f);
		ImGui::Checkbox("Use phase function", &this->use_phase_function);
		if(this->use_phase_function){
			ImGui::SliderFloat("G for (isotropy/anisotropy)", (float*)&this->Henyey_Greenstein_g, -1.0f, 1.0f);
		}
	}

	ImGui::Combo("Density Type", (int*)&densityType, "CONSTANT\0NOISE 3D\0VDB FILE\0");

	if (this->densityType == eDensityType::NOISE_3D) {
		ImGui::SliderFloat("Noise Scale", (float*)&this->noise_scale, 0.001f, 5.0f);
		ImGui::SliderFloat("Noise Detail", (float*)&this->noise_detail, 1.0f, 5.0f);
	}
}

void VolumeMaterial::assignShader()
{
	if (this->shaderType == eShaderType::ABSORPTION) {
		this->shader = this->absorption_shader;
	}
	else if (this->shaderType == eShaderType::EMISSION_ABSORPTION) {
		this->shader = this->emissive_absorption_shader;
	}
	else if (this->shaderType == eShaderType::EMISSION_SCATTER_ABSORPTION) {
		this->shader = this->emissive_scatter_absorption_shader;
	}
}

IsosurfaceMaterial::IsosurfaceMaterial(glm::vec4 color_, glm::vec4 background_color_, std::string file_path) {
	this->color = color_;
	this->background_color = background_color_;

	this->kd = glm::vec3(0.2f, 0.3f, 0.8f);
	this->ks = glm::vec3(0.2f, 0.3f, 0.2f);
	this->alpha = 50.0f;

	this->ambient_term = glm::vec3(0.1f);

	this->step_length = 0.04f;
	this->noise_detail = 5;
	this->noise_scale = 1.54f;

	this->threshold = 0.1f;
	this->rate_of_change = 0.005f;

	this->densityType = eDensityType::CONSTANT;
	this->activate_illumination = false;
	this->shader = Shader::Get("res/shaders/basic.vs", "res/shaders/isosurface.fs");

	this->use_jittering = false;
	this->use_local_pos = true;
	this->loadVDB(file_path);
}

IsosurfaceMaterial::~IsosurfaceMaterial() { }

void IsosurfaceMaterial::setUniforms(Camera* camera, glm::mat4 model)
{
	//upload node uniforms
	this->shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	glm::vec3 camera_pos = GetInverseCameraPos(camera, model);
	this->shader->setUniform("u_camera_position", camera_pos);
	this->shader->setUniform("u_model", model);
	this->shader->setUniform("u_background_color", this->background_color);
	this->shader->setUniform("u_step_length", this->step_length);
	this->shader->setUniform("u_use_jittering", this->use_jittering);

	this->shader->setUniform("u_density_type", (int)this->densityType);
	this->shader->setUniform("u_threshold", (float)this->threshold);
	this->shader->setUniform("u_illumination_activated", this->activate_illumination);

	if (this->densityType == eDensityType::VDB_FILE) {
		if (this->texture) {
			this->shader->setUniform("u_texture", this->texture, 0);
		}
	}
	if (this->densityType == eDensityType::NOISE_3D) {
		this->shader->setUniform("u_noise_scale", this->noise_scale);
		this->shader->setUniform("u_noise_detail", this->noise_detail);
	}

	if (this->activate_illumination == true) {
		Application::instance->light_list[0]->setUniforms(this->shader, model);
		this->shader->setUniform("u_kd", this->kd);          // Diffuse coefficient
		this->shader->setUniform("u_ks", this->ks);          // Specular coefficient
		this->shader->setUniform("u_alpha", this->alpha);    // Shininess exponent
		this->shader->setUniform("u_h", (float)this->rate_of_change);
		this->shader->setUniform("u_ambient_term", this->ambient_term);
	}
	else {
		this->shader->setUniform("u_color", this->color);
	}
}

void IsosurfaceMaterial::render(Mesh* mesh, glm::mat4 model, Camera* camera)
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

void IsosurfaceMaterial::renderInMenu()
{
	if (activate_illumination) {
		ImGui::InputFloat3("Diffuse (kd)", glm::value_ptr(this->kd), "%.3f");
		ImGui::InputFloat3("Specular (ks)", glm::value_ptr(this->ks), "%.3f");
		ImGui::SliderFloat("Shininess (alpha)", (float*)&this->alpha, 10.0f, 200.0f);
		ImGui::SliderFloat("Rate of change (h)", (float*)&this->rate_of_change, 0.001f, 0.04f);
		ImGui::InputFloat3("Abient Term", glm::value_ptr(this->ambient_term), "%.3f");
	}
	else {
		ImGui::ColorEdit3("Color", (float*)&this->color);
	}
	ImGui::SliderFloat("Step Lenght", (float*)&this->step_length, 0.001f, 0.2f);
	ImGui::Checkbox("Use jittering filter", &this->use_jittering);

	ImGui::Checkbox("Activate Illumination", &this->activate_illumination);

	ImGui::Combo("Density Type", (int*)&densityType, "CONSTANT\0NOISE 3D\0VDB FILE\0");

	if (this->densityType == eDensityType::NOISE_3D) {
		ImGui::SliderFloat("Noise Scale", (float*)&this->noise_scale, 0.001f, 5.0f);
		ImGui::SliderFloat("Noise Detail", (float*)&this->noise_detail, 1.0f, 5.0f);
	}

	ImGui::SliderFloat("Density Threshold", (float*)&this->threshold, 0.001f, 0.5f);
}