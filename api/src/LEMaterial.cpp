#include <LEMaterial.h>
#include <LEException.h>
#include <iostream>

template <class T>
LightEngine::Material<T>::Material(std::shared_ptr<Core> core_ptr, std::string name) : ConstantBuffer(core_ptr), name_(name) {
	/*
	parameters.quadratic_att_ = 0.3;
	parameters.linear_att_ = 0.3;
	parameters.constant_att_ = 1.0;
	parameters.specular_level_ = 1.0;
	parameters.glossiness_ = 400;
	parameters.use_diffuse_map_ = FALSE;
	parameters.use_normal_map_ = FALSE;
	parameters.flip_tb_vectors_ = 0;
	parameters.specular[0] = 1.0;
	parameters.specular[1] = 1.0;
	parameters.specular[2] = 1.0;
	parameters.specular[3] = 0.0;

	parameters.diffuse[0] = 1.0;
	parameters.diffuse[1] = 1.0;
	parameters.diffuse[2] = 1.0;
	parameters.diffuse[3] = 0.0;

	parameters.ambient[0] = 0.0;
	parameters.ambient[1] = 0.0;
	parameters.ambient[2] = 0.0;
	parameters.ambient[3] = 0.0;
	*/
	try{
		create_constant_buffer<T>(std::make_shared<T>(parameters));
	}
	catch (const LightEngine::LECoreException &e) {
		std::cout<<e.what();
		std::wcout<<e.get_message();
		throw LECoreException("	|-> <ERROR> <Material creation failed> ", "LEMaterial.cpp", __LINE__ - 5, call_result_);
	}
	
}

template <class T>
void LightEngine::Material<T>::bind_shaders() const {

	if (vs_ptr_ == nullptr)
		throw LECoreException("	<ERROR> <Cannot bind material without a vertex shader assigned to it> ", "LEMaterial.cpp", __LINE__, call_result_);

	if (ps_ptr_ == nullptr)
		throw LECoreException("	<ERROR> <Cannot bind material without a pixel shader assigned to it> ", "LEMaterial.cpp", __LINE__, call_result_);

	vs_ptr_->bind();
	ps_ptr_->bind();	


}

template <class T>
void LightEngine::Material<T>::assign_vertex_shader(std::shared_ptr<VertexShader> vs_ptr) { vs_ptr_ = vs_ptr;}

template <class T>
void LightEngine::Material<T>::assign_pixel_shader(std::shared_ptr<PixelShader> ps_ptr) { ps_ptr_ = ps_ptr;}

template <class T>
void LightEngine::Material<T>::update() {
	try{
		update_constant_buffer<T>(std::make_shared<T>(parameters));
	}
	catch (const LightEngine::LECoreException &e) {
		std::cout<<e.what();
		std::wcout<<e.get_message();
		throw LECoreException("	|-> <ERROR> <Material (constant buffer) update failed> ", "LEMaterial.cpp", __LINE__ - 5, call_result_);
	}
}

template<class T>
T LightEngine::Material<T>::get_all_parameters() const {
	return parameters;
}

template<class T>
void LightEngine::Material<T>::set_all_parameters(T new_parameters) {
	parameters = new_parameters;
}

template <class T>
std::string LightEngine::Material<T>::get_name() const {return name_;}



LightEngine::DefaultMaterial::DefaultMaterial(std::shared_ptr<Core> core_ptr, std::string name) : Material(core_ptr, name) {}

void LightEngine::DefaultMaterial::bind() const {
	
	bind_shaders();
	
	if(parameters.use_diffuse_map_)
		diffuse_map_ptr_->bind(0);

	if(parameters.use_normal_map_)
		normal_map_ptr_->bind(1);

	bind_ps_buffer(3);
	bind_vs_buffer(3);
}

void LightEngine::DefaultMaterial::assign_diffuse_map(std::shared_ptr<Texture> diffuse_map_ptr) { diffuse_map_ptr_ = diffuse_map_ptr; }

void LightEngine::DefaultMaterial::assign_normal_map(std::shared_ptr<Texture> normal_map_ptr) { normal_map_ptr_ = normal_map_ptr; }

void LightEngine::DefaultMaterial::set_quadratic_att(float value) {
	parameters.quadratic_att_ = value;
}

void LightEngine::DefaultMaterial::set_linear_att(float value) {
	parameters.linear_att_ = value;
}

void LightEngine::DefaultMaterial::set_constant_att(float value) {
	parameters.constant_att_ = value;
}

void LightEngine::DefaultMaterial::set_specular_level(float value) {
	parameters.specular_level_ = value;
}

void LightEngine::DefaultMaterial::set_glossiness(int value) {
	parameters.glossiness_ = value;
}

void LightEngine::DefaultMaterial::set_specular_color(float specular[3]) {
	parameters.specular[0] = specular[0];
	parameters.specular[1] = specular[1];
	parameters.specular[2] = specular[2];
}

void LightEngine::DefaultMaterial::set_diffuse_color(float diffuse[3]) {
	parameters.diffuse[0] = diffuse[0];
	parameters.diffuse[1] = diffuse[1];
	parameters.diffuse[2] = diffuse[2];
}

void LightEngine::DefaultMaterial::set_ambient_color(float ambient[3]) {
	parameters.ambient[0] = ambient[0];
	parameters.ambient[1] = ambient[1];
	parameters.ambient[2] = ambient[2];
}

void LightEngine::DefaultMaterial::select_diffuse_map_usage() {
	if(diffuse_map_ptr_ != nullptr)
		parameters.use_diffuse_map_ = TRUE;
}

void LightEngine::DefaultMaterial::select_normal_map_usage() {
	if(normal_map_ptr_ != nullptr)
		parameters.use_normal_map_ = TRUE;
}

void LightEngine::DefaultMaterial::unselect_diffuse_map_usage() {parameters.use_diffuse_map_ = FALSE;}

void LightEngine::DefaultMaterial::unselect_normal_map_usage() {parameters.use_normal_map_ = FALSE;}

bool LightEngine::DefaultMaterial::is_diffuse_map_assigned() const {
	if (diffuse_map_ptr_ != nullptr)
		return true;
	return false;
}

bool LightEngine::DefaultMaterial::is_normal_map_assigned() const {
	if (normal_map_ptr_ != nullptr)
		return true;
	return false;
}

void LightEngine::DefaultMaterial::flip_tangent() { parameters.flip_tb_vectors_ = (0xFF00 & parameters.flip_tb_vectors_) | ((~parameters.flip_tb_vectors_& 0xFF) & 0x2);}

void LightEngine::DefaultMaterial::flip_bitangent() { parameters.flip_tb_vectors_ = (0x00FF & parameters.flip_tb_vectors_) | ((~parameters.flip_tb_vectors_& 0xFF00) & (0x2<<8));}



LightEngine::PBRMaterial::PBRMaterial(std::shared_ptr<Core> core_ptr, std::string name) : Material(core_ptr, name) {}

void LightEngine::PBRMaterial::bind() const {
	bind_shaders();
	bind_ps_buffer(3);
	bind_vs_buffer(3);

	if(albedo_map_ptr_)
		albedo_map_ptr_->bind(0);

	if(roughness_map_ptr_)
		roughness_map_ptr_->bind(1);

	if(metalness_map_ptr_)
		metalness_map_ptr_->bind(2);

	if(normal_map_ptr_)
		normal_map_ptr_->bind(3);

	if(ao_map_ptr_)
		ao_map_ptr_->bind(4);
}

void LightEngine::PBRMaterial::set_albedo(float albedo[3]) {
	parameters.albedo_[0] = albedo[0];
	parameters.albedo_[1] = albedo[1];
	parameters.albedo_[2] = albedo[2];
}

void LightEngine::PBRMaterial::set_roughness(float roughness) { parameters.roughness_ = roughness; }

void LightEngine::PBRMaterial::set_metallic(float metallic) { parameters.metallic_ = metallic; }

void LightEngine::PBRMaterial::assign_albedo_map(std::shared_ptr<Texture> albedo_map_ptr) { albedo_map_ptr_ = albedo_map_ptr; parameters.use_albedo_map_ = TRUE; }

void LightEngine::PBRMaterial::assign_roughness_map(std::shared_ptr<Texture> roughness_map_ptr) { roughness_map_ptr_ = roughness_map_ptr; parameters.use_roughness_map_ = TRUE; }

void LightEngine::PBRMaterial::assign_metalness_map(std::shared_ptr<Texture> metalness_map_ptr) { metalness_map_ptr_ = metalness_map_ptr; parameters.use_metalness_map_ = TRUE; }

void LightEngine::PBRMaterial::assign_normal_map(std::shared_ptr<Texture> normal_map_ptr) { normal_map_ptr_ = normal_map_ptr; parameters.use_normal_map_ = TRUE; }

void LightEngine::PBRMaterial::assign_ao_map(std::shared_ptr<Texture> ao_map_ptr) { ao_map_ptr_ = ao_map_ptr; parameters.use_ao_map_ = TRUE; }

void LightEngine::PBRMaterial::set_maps_usage(bool flags[5]) {
	parameters.use_albedo_map_ = albedo_map_ptr_ && flags[0];
	parameters.use_roughness_map_ = roughness_map_ptr_ && flags[1];
	parameters.use_metalness_map_ = metalness_map_ptr_ && flags[2];
	parameters.use_normal_map_ = normal_map_ptr_ && flags[3];
	parameters.use_ao_map_ = ao_map_ptr_ && flags[4];
}




