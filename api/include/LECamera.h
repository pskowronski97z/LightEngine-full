#pragma once
#include <DirectXMath.h>
#include <LECore.h>
#include <memory>
#include <LEShader.h>

namespace LightEngine {

	//TODO: fix virtual destructor, copy constructor, etc.
	class __declspec(dllexport) Camera : public ConstantBuffer {	
	protected:
		float fov_;
		float aspect_ratio_;
		float near_z_;
		float far_z_;
		struct TransformMatrices {
			DirectX::XMMATRIX view_matrix;
			DirectX::XMMATRIX projection_matrix;
		};
		TransformMatrices transform_matrices_;	
	
		Camera(std::shared_ptr<Core> core_ptr);

	public:
		/// <summary>
		/// Passing camera and projection matrices as constant buffer to GPU's r<slot> register. 
		/// </summary>
		/// <param name="slot">Number of the register.</param>
		void bind(short slot);
		void update();
		void set_clipping_near(float near_z);
		void set_clipping_far(float far_z);
		void set_fov(float angle_in_degrees);
		void set_aspect_ratio(float aspect_ratio);
		virtual void reset();
		std::shared_ptr<const DirectX::XMMATRIX> get_camera_matrix();
		virtual void update_view_matrix();
		void update_projection_matrix();

	};

	class __declspec(dllexport) FPSCamera : public Camera {
	private:
		float horizontal_angle_;
		float vertical_angle_;
		float position_[3];

	public:
		FPSCamera(std::shared_ptr<Core> core_ptr);
		void update_view_matrix() override;
		void modify_horizontal_angle(float delta_angle);
		void modify_vertical_angle(float delta_angle);
		void move_relative_z(float delta);
		void move_relative_x(float delta);
		void move_y(float delta);
		void reset() override;
	};

	class __declspec(dllexport) ArcballCamera : public Camera {
	private:
		float horizontal_angle_;
		float vertical_angle_;
		float radius_;
		float center_[3];

	public:
		ArcballCamera(std::shared_ptr<Core> core_ptr, float radius);
		void update_view_matrix() override;
		void modify_horizontal_angle(float delta_angle);
		void modify_vertical_angle(float delta_angle);
		void modify_radius(float delta_radius);
		void modify_center(float delta_x, float delta_y, float delta_z);
		void pan_horizontal(float delta);
		void pan_vertical(float delta);
		void reset() override;
	};
	
}
