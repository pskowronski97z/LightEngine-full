#include <iostream>
#include <LECamera.h>
#include <LEException.h>
#include <ostream>
#include <math.h>

#define M_PI 3.14159265358979323846

LightEngine::Camera::Camera(std::shared_ptr<Core> core_ptr) : ConstantBuffer(core_ptr) {

	D3D11_BUFFER_DESC buffer_desc;
	D3D11_SUBRESOURCE_DATA sr_data;

	set_fov(45);
	set_clipping(1.0,10.0);
	set_scaling(16,9);
	reset();
	update_projection();

	try {
		create_constant_buffer<TransformMatrices>(std::make_shared<TransformMatrices>(transform_matrices_));
	}
	catch (const LightEngine::LECoreException &e) {
		std::cout<<e.what();
		std::wcout<<e.get_message();
		throw LECoreException("	|-> <ERROR> <Camera creation failed> ", "LECamera.cpp", __LINE__ - 5, call_result_);
	}

}

void LightEngine::Camera::set_fov(float angle) {
	temp_projection_matrix_[0][0] = 1.0/tan(angle*M_PI/360.0);
	temp_projection_matrix_[1][1] = 1.0/tan(angle*M_PI/360.0);
	need_projection_update = true;
}

void LightEngine::Camera::set_clipping(float near_z, float far_z) {
	temp_projection_matrix_[2][2] = 1.0f / (far_z - near_z);
	temp_projection_matrix_[3][2] = -near_z / (far_z - near_z);
	need_projection_update = true;
}

void LightEngine::Camera::set_scaling(short width, short height) {
	scaling_ = static_cast<float>(height) / static_cast<float>(width);
	need_projection_update = true;
}

void LightEngine::Camera::update_preprocess() {
	if(need_projection_update) 
		update_projection();
}

void LightEngine::Camera::update_projection() {
	transform_matrices_.projection_matrix = DirectX::XMMatrixSet(
		temp_projection_matrix_[0][0]*scaling_, temp_projection_matrix_[0][1], temp_projection_matrix_[0][2], temp_projection_matrix_[0][3],
		temp_projection_matrix_[1][0], temp_projection_matrix_[1][1], temp_projection_matrix_[1][2], temp_projection_matrix_[1][3],
		temp_projection_matrix_[2][0], temp_projection_matrix_[2][1], temp_projection_matrix_[2][2], temp_projection_matrix_[2][3],
		temp_projection_matrix_[3][0], temp_projection_matrix_[3][1], temp_projection_matrix_[3][2], temp_projection_matrix_[3][3]
	);
	need_projection_update = false;
}

void LightEngine::Camera::set_active(short slot) {
	bind_vs_buffer(slot);
	bind_ps_buffer(slot);
}

void LightEngine::Camera::update() {

	update_preprocess();

	TransformMatrices transposed_matrices;

	transposed_matrices.camera_matrix = XMMatrixTranspose(transform_matrices_.camera_matrix);
	transposed_matrices.projection_matrix = XMMatrixTranspose(transform_matrices_.projection_matrix);
		
	try {
		update_constant_buffer<TransformMatrices>(std::make_shared<TransformMatrices>(transposed_matrices));
	}
	catch (const LightEngine::LECoreException &e) {
		std::cout<<e.what();
		std::wcout<<e.get_message();
		throw LECoreException("	|-> <ERROR> <Camera (constant buffer) update failed> ", "LECamera.cpp", __LINE__ - 5, call_result_);
	}
}

void LightEngine::Camera::reset() {
	transform_matrices_.camera_matrix = DirectX::XMMatrixSet(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	);
}

std::shared_ptr<const DirectX::XMMATRIX> LightEngine::Camera::get_camera_matrix() {
	return std::make_shared<const DirectX::XMMATRIX>(transform_matrices_.camera_matrix);
}


LightEngine::FPSCamera::FPSCamera(std::shared_ptr<Core> core_ptr) : Camera(core_ptr) {
	horizontal_angle_ = 0;
	vertical_angle_ = 0;
	position_[0] = 0;
	position_[1] = 0;
	position_[2] = 0;
}

void LightEngine::FPSCamera::update_preprocess() {
	Camera::update_preprocess();

	transform_matrices_.camera_matrix = DirectX::XMMatrixSet(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		position_[0], position_[1], position_[2], 1
	);

	transform_matrices_.camera_matrix *= DirectX::XMMatrixRotationY(vertical_angle_ * M_PI/180.0f);
	transform_matrices_.camera_matrix *= DirectX::XMMatrixRotationX(horizontal_angle_ * M_PI/180.0f);
}

void LightEngine::FPSCamera::modify_horizontal_angle(float delta_angle) {
	horizontal_angle_ += delta_angle;
}

void LightEngine::FPSCamera::modify_vertical_angle(float delta_angle) {
	vertical_angle_ += delta_angle;
}

void LightEngine::FPSCamera::move_relative_z(float delta) {
	position_[0] += delta * sin(-vertical_angle_* M_PI / 180.0f);
	position_[2] += delta * cos(-vertical_angle_* M_PI / 180.0f);
}

void LightEngine::FPSCamera::move_relative_x(float delta) {
	position_[0] += delta * cos(vertical_angle_* M_PI / 180.0f);
	position_[2] += delta * sin(vertical_angle_* M_PI / 180.0f);
}

void LightEngine::FPSCamera::move_y(float delta) {
	position_[1] += delta;
}



LightEngine::ArcballCamera::ArcballCamera(std::shared_ptr<Core> core_ptr, float radius) : Camera(core_ptr) {
	horizontal_angle_= 0;
	vertical_angle_ = 0;
	radius_ = radius;
	center_[0] = 0.0;
	center_[1] = 0.0;
	center_[2] = 0.0;

	update();
}

void LightEngine::ArcballCamera::update_preprocess() {
	
	Camera::update_preprocess();

	transform_matrices_.camera_matrix = DirectX::XMMatrixSet(
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	-center_[0], -center_[1], -center_[2], 1.0);
	
	transform_matrices_.camera_matrix = DirectX::XMMatrixMultiply(transform_matrices_.camera_matrix, DirectX::XMMatrixRotationY(vertical_angle_* M_PI / 180.0f));
	transform_matrices_.camera_matrix = DirectX::XMMatrixMultiply(transform_matrices_.camera_matrix, DirectX::XMMatrixRotationX(horizontal_angle_* M_PI / 180.0f));
	
	transform_matrices_.camera_matrix += DirectX::XMMatrixSet(
	0.0, 0.0, 0.0, 0.0, 
	0.0, 0.0, 0.0, 0.0,
	0.0, 0.0, 0.0, 0.0,
	0, 0, radius_, 0.0);


	
}

void LightEngine::ArcballCamera::modify_horizontal_angle(float delta_angle) {
	horizontal_angle_ += delta_angle;
}

void LightEngine::ArcballCamera::modify_vertical_angle(float delta_angle) {
	vertical_angle_ += delta_angle;
}

void LightEngine::ArcballCamera::modify_radius(float delta_radius) {
	radius_ += delta_radius;
}

void LightEngine::ArcballCamera::modify_center(float delta_x, float delta_y, float delta_z) {
	center_[0] += delta_x;
	center_[1] += delta_y;
	center_[2] += delta_z;
}

void LightEngine::ArcballCamera::pan_horizontal(float delta) {
	center_[0] += delta * cos(vertical_angle_* M_PI / 180.0f);
	center_[2] += delta * sin(vertical_angle_* M_PI / 180.0f);
}

void LightEngine::ArcballCamera::pan_vertical(float delta) {
	center_[1] += delta * cos(-horizontal_angle_* M_PI / 180.0f);
	center_[2] += delta * sin(-horizontal_angle_* M_PI / 180.0f);
}

void LightEngine::ArcballCamera::reset() {
	horizontal_angle_= 0;
	vertical_angle_ = 0;
	radius_ = 0;
	center_[0] = 0.0;
	center_[1] = 0.0;
	center_[2] = 0.0;
}

