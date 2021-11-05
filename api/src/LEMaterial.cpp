#include <LEMaterial.h>
#include <LEException.h>
#include <iostream>

template <class T>
LightEngine::Materials::Material<T>::Material(std::shared_ptr<Core> core_ptr, std::string name)
	: ConstantBuffer(core_ptr), name_(name), maps_names_(std::vector<std::string>(0)) {

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
void LightEngine::Materials::Material<T>::bind_shaders() const {

	if (vs_ptr_ == nullptr)
		throw LECoreException("	<ERROR> <Cannot bind material without a vertex shader assigned to it> ", "LEMaterial.cpp", __LINE__, call_result_);

	if (ps_ptr_ == nullptr)
		throw LECoreException("	<ERROR> <Cannot bind material without a pixel shader assigned to it> ", "LEMaterial.cpp", __LINE__, call_result_);

	vs_ptr_->bind();
	ps_ptr_->bind();	
}

template <class T>
void LightEngine::Materials::Material<T>::assign_vertex_shader(std::shared_ptr<VertexShader> vs_ptr) { vs_ptr_ = vs_ptr;}

template <class T>
void LightEngine::Materials::Material<T>::assign_pixel_shader(std::shared_ptr<PixelShader> ps_ptr) { ps_ptr_ = ps_ptr;}

template <class T>
void LightEngine::Materials::Material<T>::update() {
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
T LightEngine::Materials::Material<T>::get_all_parameters() const {
	return parameters;
}

template <class T>
std::string LightEngine::Materials::Material<T>::get_name() const {return name_;}



LightEngine::Materials::BasicMaterial::BasicMaterial(std::shared_ptr<Core> core_ptr, std::string name) : Material(core_ptr, name) {
	maps_names_ = std::vector<std::string>(2);
	maps_names_.at(0) = "Set diffuse map";
	maps_names_.at(1) = "Set normal map";
}

void LightEngine::Materials::BasicMaterial::bind() const {
	
	bind_shaders();
	
	if(parameters.use_diffuse_map_)
		diffuse_map_ptr_->bind(0);

	if(parameters.use_normal_map_)
		normal_map_ptr_->bind(1);

	bind_ps_buffer(3);
	bind_vs_buffer(3);
}

void LightEngine::Materials::BasicMaterial::set_diffuse_map(std::shared_ptr<StaticTexture> diffuse_map_ptr) { 
	diffuse_map_ptr_ = diffuse_map_ptr; 

	if(diffuse_map_ptr_ != nullptr) {
		maps_names_.at(0) = diffuse_map_ptr_->get_name();
		parameters.use_diffuse_map_ = TRUE;
	}
	else {
		maps_names_.at(0) = "Set diffuse map";
		parameters.use_diffuse_map_ = FALSE;
	}

}

void LightEngine::Materials::BasicMaterial::set_normal_map(std::shared_ptr<StaticTexture> normal_map_ptr) { 
	normal_map_ptr_ = normal_map_ptr; 

	if(normal_map_ptr_ != nullptr) {
		maps_names_.at(1) = normal_map_ptr_->get_name();
		parameters.use_normal_map_ = TRUE;
	}
	else {
		maps_names_.at(1) = "Set normal map";
		parameters.use_normal_map_ = FALSE;
	}
}

void LightEngine::Materials::BasicMaterial::set_quadratic_att(float value) {
	parameters.quadratic_att_ = value;
}

void LightEngine::Materials::BasicMaterial::set_linear_att(float value) {
	parameters.linear_att_ = value;
}

void LightEngine::Materials::BasicMaterial::set_constant_att(float value) {
	parameters.constant_att_ = value;
}

void LightEngine::Materials::BasicMaterial::set_specular_level(float value) {
	parameters.specular_level_ = value;
}

void LightEngine::Materials::BasicMaterial::set_glossiness(int value) {
	parameters.glossiness_ = value;
}

void LightEngine::Materials::BasicMaterial::set_specular_color(float specular[3]) {
	parameters.specular[0] = specular[0];
	parameters.specular[1] = specular[1];
	parameters.specular[2] = specular[2];
}

void LightEngine::Materials::BasicMaterial::set_diffuse_color(float diffuse[3]) {
	parameters.diffuse[0] = diffuse[0];
	parameters.diffuse[1] = diffuse[1];
	parameters.diffuse[2] = diffuse[2];
}

void LightEngine::Materials::BasicMaterial::set_ambient_color(float ambient[3]) {
	parameters.ambient[0] = ambient[0];
	parameters.ambient[1] = ambient[1];
	parameters.ambient[2] = ambient[2];
}

void LightEngine::Materials::BasicMaterial::flip_tangent() { 
	parameters.flip_tb_vectors_ = (0xFF00 & parameters.flip_tb_vectors_) | ((~parameters.flip_tb_vectors_& 0xFF) & 0x2);
}

void LightEngine::Materials::BasicMaterial::flip_bitangent() { 
	parameters.flip_tb_vectors_ = (0x00FF & parameters.flip_tb_vectors_) | ((~parameters.flip_tb_vectors_& 0xFF00) & (0x2<<8));
}

std::vector<std::string> LightEngine::Materials::BasicMaterial::get_maps_names() const {
	return maps_names_;
}



LightEngine::Materials::PBRMaterial::PBRMaterial(std::shared_ptr<Core> core_ptr, std::string name) : Material(core_ptr, name) {
	maps_names_ = std::vector<std::string>(5);

	maps_names_.at(0) = "Set albedo map";
	maps_names_.at(1) = "Set roughness map";
	maps_names_.at(2) = "Set metalness map";
	maps_names_.at(3) = "Set normal map";
	maps_names_.at(4) = "Set ambient occlusion map";
}

void LightEngine::Materials::PBRMaterial::bind() const {
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

void LightEngine::Materials::PBRMaterial::set_albedo(float albedo[3]) {
	parameters.albedo_[0] = albedo[0];
	parameters.albedo_[1] = albedo[1];
	parameters.albedo_[2] = albedo[2];
}

void LightEngine::Materials::PBRMaterial::set_roughness(float roughness) { parameters.roughness_ = roughness; }

void LightEngine::Materials::PBRMaterial::set_metalness(float metalness) { parameters.metalness_ = metalness; }

void LightEngine::Materials::PBRMaterial::set_albedo_map(std::shared_ptr<StaticTexture> albedo_map_ptr) { 
	albedo_map_ptr_ = albedo_map_ptr;

	if(albedo_map_ptr_ != nullptr) {
		maps_names_.at(0) = albedo_map_ptr_->get_name();
		parameters.use_albedo_map_ = TRUE;
	}	
	else {
		maps_names_.at(0) = "Set albedo map";
		parameters.use_albedo_map_ = FALSE;
	}		
}

void LightEngine::Materials::PBRMaterial::set_roughness_map(std::shared_ptr<StaticTexture> roughness_map_ptr) {
	roughness_map_ptr_ = roughness_map_ptr;

	if(roughness_map_ptr_ != nullptr) {
		maps_names_.at(1) = roughness_map_ptr_->get_name();
		parameters.use_roughness_map_ = TRUE;
	}	
	else {
		maps_names_.at(1) = "Set roughness map";
		parameters.use_roughness_map_ = FALSE;
	}	
}

void LightEngine::Materials::PBRMaterial::set_metalness_map(std::shared_ptr<StaticTexture> metalness_map_ptr) { 
	metalness_map_ptr_ = metalness_map_ptr; 

	if(metalness_map_ptr_ != nullptr) {
		maps_names_.at(2) = metalness_map_ptr_->get_name();
		parameters.use_metalness_map_ = TRUE;
	}	
	else {
		maps_names_.at(2) = "Set metalness map";
		parameters.use_metalness_map_ = FALSE;
	}
}

void LightEngine::Materials::PBRMaterial::set_normal_map(std::shared_ptr<StaticTexture> normal_map_ptr) { 
	normal_map_ptr_ = normal_map_ptr; 

	if(normal_map_ptr_ != nullptr) {
		maps_names_.at(3) = normal_map_ptr_->get_name();
		parameters.use_normal_map_ = TRUE;
	}	
	else {
		maps_names_.at(3) = "Set normal map";
		parameters.use_normal_map_ = FALSE;
	}
}

void LightEngine::Materials::PBRMaterial::set_ao_map(std::shared_ptr<StaticTexture> ao_map_ptr) { 
	ao_map_ptr_ = ao_map_ptr; 

	if(ao_map_ptr_ != nullptr) {
		maps_names_.at(4) = ao_map_ptr_->get_name();
		parameters.use_ao_map_ = TRUE;
	}	
	else {
		maps_names_.at(4) = "Set ambient occlusion map";
		parameters.use_ao_map_ = FALSE;
	}
}

std::vector<std::string> LightEngine::Materials::PBRMaterial::get_maps_names() const {
	return maps_names_;
}





