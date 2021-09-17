#pragma once
#include <DirectXMath.h>
#include <LECore.h>
#include <memory>
#include <LEShader.h>

namespace LightEngine {

	//TODO: fix virtual destructor, copy constructor, etc.
	class __declspec(dllexport) Camera : public ConstantBuffer {
	public:
		/// <summary>
		/// Passing camera and projection matrices as constant buffer to GPU's r<slot> register. 
		/// </summary>
		/// <param name="slot">Number of the register.</param>
		void set_active(short slot);
		void update();
		void set_clipping(float near_z, float far_z);
		void set_fov(float angle);
		void set_scaling(short width, short height);
		virtual void reset();
		std::shared_ptr<const DirectX::XMMATRIX> get_camera_matrix();

	protected:
		bool need_projection_update = false;
		struct TransformMatrices {
			DirectX::XMMATRIX camera_matrix;
			DirectX::XMMATRIX projection_matrix;
		};
		TransformMatrices transform_matrices_;
		float temp_projection_matrix_[4][4] {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 1,
			0, 0, 1, 0
		};
		float scaling_ = 1;

		virtual void update_preprocess();
		void update_projection();
		Camera(std::shared_ptr<Core> core_ptr);
	};

	class __declspec(dllexport) FPSCamera : public Camera {
	private:
		float horizontal_angle_;
		float vertical_angle_;
		float position_[3];

	public:
		FPSCamera(std::shared_ptr<Core> core_ptr);
		void update_preprocess() override;
		void modify_horizontal_angle(float delta_angle);
		void modify_vertical_angle(float delta_angle);
		void move_relative_z(float delta);
		void move_relative_x(float delta);
		void move_y(float delta);
	};

	class __declspec(dllexport) ArcballCamera : public Camera {
	private:
		float horizontal_angle_;
		float vertical_angle_;
		float radius_;
		float center_[3];

	public:
		ArcballCamera(std::shared_ptr<Core> core_ptr, float radius);
		void update_preprocess() override;
		void modify_horizontal_angle(float delta_angle);
		void modify_vertical_angle(float delta_angle);
		void modify_radius(float delta_radius);
		void modify_center(float delta_x, float delta_y, float delta_z);
		void pan_horizontal(float delta);
		void pan_vertical(float delta);
		void reset() override;
	};
	
}
