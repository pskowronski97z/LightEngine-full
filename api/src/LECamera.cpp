#include <iostream>
#include <LECamera.h>
#include <LEException.h>
#include <ostream>
#include <math.h>

LightEngine::Camera::Camera(std::shared_ptr<Core> core_ptr) : ConstantBuffer(core_ptr) {

	D3D11_BUFFER_DESC buffer_desc;
	D3D11_SUBRESOURCE_DATA sr_data;

	fov_ = DirectX::XMConvertToRadians(45.0f);
	aspect_ratio_ = 16.0f / 9.0f;
	near_z_ = 5.0f;
	far_z_ = 400.0f;
	is_ortho_ = false;
	ortho_scaling_ = 20.0;
	camera_world_position_[0] = 0.0;
	camera_world_position_[1] = 0.0;
	camera_world_position_[2] = 0.0;

	update_projection_matrix();
	reset();

	try {
		create_constant_buffer<TransformMatrices>(std::make_shared<TransformMatrices>(transform_matrices_));
	}
	catch (const LightEngine::LECoreException &e) {
		std::cout<<e.what();
		std::wcout<<e.get_message();
		throw LECoreException("	|-> <ERROR> <Camera creation failed> ", "LECamera.cpp", __LINE__ - 5, call_result_);
	}

}

void LightEngine::Camera::set_fov(float angle_in_degrees) {	fov_ = DirectX::XMConvertToRadians(angle_in_degrees); }

void LightEngine::Camera::set_aspect_ratio(float aspect_ratio) { aspect_ratio_ = aspect_ratio; }

void LightEngine::Camera::set_orthographic_scaling(float scaling) { ortho_scaling_= scaling; }

void LightEngine::Camera::set_clipping_near(float near_z) { near_z_ = near_z; }

void LightEngine::Camera::set_clipping_far(float far_z) { far_z_ = far_z; }

void LightEngine::Camera::update_projection_matrix() {
	if(is_ortho_)
		transform_matrices_.projection_matrix = DirectX::XMMatrixOrthographicLH(aspect_ratio_*ortho_scaling_, ortho_scaling_, near_z_, far_z_);
	else
		transform_matrices_.projection_matrix = DirectX::XMMatrixPerspectiveFovLH(fov_, aspect_ratio_, near_z_, far_z_);
}

void LightEngine::Camera::bind(short slot) {
	bind_vs_buffer(slot);
	//bind_ps_buffer(slot);
}

void LightEngine::Camera::set_perspective_projection() { is_ortho_ = false; }

void LightEngine::Camera::set_orthographic_projection() { is_ortho_ = true; }

void LightEngine::Camera::update() {

	TransformMatrices transposed_matrices;

	transposed_matrices.view_matrix = XMMatrixTranspose(transform_matrices_.view_matrix);
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
	transform_matrices_.view_matrix = DirectX::XMMatrixSet(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	);
}

std::shared_ptr<const DirectX::XMMATRIX> LightEngine::Camera::get_camera_matrix() {
	return std::make_shared<const DirectX::XMMATRIX>(transform_matrices_.view_matrix);
}

void LightEngine::Camera::update_view_matrix() {}

LightEngine::FPSCamera::FPSCamera(std::shared_ptr<Core> core_ptr) : Camera(core_ptr) {
	horizontal_angle_ = 0;
	vertical_angle_ = 0;
	camera_world_position_[0] = 0;
	camera_world_position_[1] = 0;
	camera_world_position_[2] = 0;
}

void LightEngine::FPSCamera::update_view_matrix() {

	transform_matrices_.view_matrix = DirectX::XMMatrixSet(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		camera_world_position_[0], camera_world_position_[1], camera_world_position_[2], 1
	);

	transform_matrices_.view_matrix *= DirectX::XMMatrixRotationY(vertical_angle_);
	transform_matrices_.view_matrix *= DirectX::XMMatrixRotationX(horizontal_angle_);

}

void LightEngine::FPSCamera::modify_horizontal_angle(float delta_angle) {
	horizontal_angle_ += DirectX::XMConvertToRadians(delta_angle);
}

void LightEngine::FPSCamera::modify_vertical_angle(float delta_angle) {
	vertical_angle_ += DirectX::XMConvertToRadians(delta_angle);
}

void LightEngine::FPSCamera::move_relative_z(float delta) {
	camera_world_position_[0] += delta * DirectX::XMScalarSin(-vertical_angle_);
	camera_world_position_[2] += delta * DirectX::XMScalarCos(-vertical_angle_);
}

void LightEngine::FPSCamera::move_relative_x(float delta) {
	camera_world_position_[0] += delta * DirectX::XMScalarCos(vertical_angle_);
	camera_world_position_[2] += delta * DirectX::XMScalarSin(vertical_angle_);
}

void LightEngine::FPSCamera::move_y(float delta) {
	camera_world_position_[1] += delta;
}

void LightEngine::FPSCamera::reset() {
	horizontal_angle_ = 0;
	vertical_angle_ = 0;
	camera_world_position_[0] = 0;
	camera_world_position_[1] = 0;
	camera_world_position_[2] = 0;

	update_view_matrix();
}

LightEngine::ArcballCamera::ArcballCamera(std::shared_ptr<Core> core_ptr, float radius) : Camera(core_ptr) {
	horizontal_angle_= 0;
	vertical_angle_ = 0;
	radius_ = radius;
	center_[0] = 0.0;
	center_[1] = 0.0;
	center_[2] = 0.0;

	camera_world_position_[2] = -radius;

	update_view_matrix();
	update();
}

void LightEngine::ArcballCamera::update_view_matrix() {
	
	static DirectX::XMVECTOR camera_world_position; 

	camera_world_position.m128_f32[0] = 0.0;
	camera_world_position.m128_f32[1] = 0.0;
	camera_world_position.m128_f32[2] = -radius_;
	camera_world_position.m128_f32[3] = 1.0;

	transform_matrices_.view_matrix = DirectX::XMMatrixSet(
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	-center_[0], -center_[1], -center_[2], 1.0);
	
	transform_matrices_.view_matrix *= DirectX::XMMatrixRotationY(vertical_angle_);
	transform_matrices_.view_matrix *= DirectX::XMMatrixRotationX(horizontal_angle_);
	transform_matrices_.view_matrix.r[3].m128_f32[2] += radius_;

	camera_world_position = DirectX::XMVector4Transform(camera_world_position, DirectX::XMMatrixRotationX(-horizontal_angle_));
	camera_world_position = DirectX::XMVector4Transform(camera_world_position, DirectX::XMMatrixRotationY(-vertical_angle_));

	camera_world_position_[0] = camera_world_position.m128_f32[0] + center_[0];
	camera_world_position_[1] = camera_world_position.m128_f32[1] + center_[1];
	camera_world_position_[2] = camera_world_position.m128_f32[2] + center_[2];

}

void LightEngine::ArcballCamera::modify_horizontal_angle(float delta_angle) {
	horizontal_angle_ += DirectX::XMConvertToRadians(delta_angle);
}

void LightEngine::ArcballCamera::modify_vertical_angle(float delta_angle) {
	vertical_angle_ += DirectX::XMConvertToRadians(delta_angle);
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
	center_[0] += delta * DirectX::XMScalarCos(vertical_angle_);
	center_[2] += delta * DirectX::XMScalarSin(vertical_angle_);
}

void LightEngine::ArcballCamera::pan_vertical(float delta) {
	center_[1] += delta * DirectX::XMScalarCos(-horizontal_angle_);
	center_[2] += delta * DirectX::XMScalarSin(-horizontal_angle_);
}

void LightEngine::ArcballCamera::reset() {
	horizontal_angle_= 0;
	vertical_angle_ = 0;
	radius_ = 0;
	center_[0] = 0.0;
	center_[1] = 0.0;
	center_[2] = 0.0;

	update_view_matrix();
}

