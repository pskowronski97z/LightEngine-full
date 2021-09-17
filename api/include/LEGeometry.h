#pragma once
#pragma comment (lib, "tinyobjloader")
#include <memory>
#include <LEShader.h>
#include <wrl.h>
#include <LEData.h>

namespace LightEngine {

	template <class T> 
	class Geometry : private CoreUser {
	private:
		std::string name_;
		Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer_;
		Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer_;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_ptr_;
		D3D11_PRIMITIVE_TOPOLOGY primitive_topology_;
		std::vector<T> vertices_vector_;
		std::vector<unsigned int> indices_vector_;
		UINT stride_;
		UINT offset_;
		static DirectX::XMFLOAT3 calculate_tris_tangent(Vertex3 v1, Vertex3 v2, Vertex3 v3);
		// Performs the Gram-Schmidt orthogonalization of a "v" vector to a "base" vector
		static DirectX::XMVECTOR orthogonalize(DirectX::XMFLOAT3 base, DirectX::XMFLOAT3 v);
	public:
		static std::vector<Geometry<T>> load_from_obj(std::shared_ptr<Core> core_ptr, std::string filename);
		Geometry(std::shared_ptr<Core> core_ptr, std::vector<T> vertices, D3D11_PRIMITIVE_TOPOLOGY topology, std::string name);
		void set_indices(std::vector<unsigned int> indices);
		inline void bind_vertex_buffer();
		inline void bind_index_buffer();
		inline void bind_topology();
		inline void draw(int start_loc);		
		inline void draw_indexed(int start_loc);
	};

	template class __declspec(dllexport) Geometry<Vertex3>;

	// --------------- Work in progress ----------------------------------
	class __declspec(dllexport) LightSource : public ConstantBuffer{
	private:
		struct Parameters {
			float coordinates[4];
			float color[4];
			float additional[4]; // [0] - intensity
		};
		Parameters parameters;
	public:
		LightSource(std::shared_ptr<Core> core_ptr, float type);
		void set_position(float position[3]);
		void set_color(float color[3]);
		void set_intensity(float intensity);
		void update();
	};
	// --------------- Work in progress ----------------------------------
	
	// ------------- For debug only -----------------------------
	class __declspec(dllexport) DEBUG_VSTransform : private CoreUser {
	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> constant_buffer_ptr_;
		DirectX::XMMATRIX rotations_[2]; // x (0), y (1)
		DirectX::XMMATRIX rotations_tr_[2]; // x (0), y (1)
		float angle_x_;
		float angle_y_;
	public:
		DEBUG_VSTransform(std::shared_ptr<Core> core_ptr);
		void rotate_x(float delta_x);
		void rotate_y(float delta_y);
		void update();
	};

	// ------------- For debug only -----------------------------
}
