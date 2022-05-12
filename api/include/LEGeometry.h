#pragma once
#pragma comment (lib, "tinyobjloader")
#include <LECamera.h>
#include <LEShader.h>

namespace LightEngine {

	class Camera;

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
		static std::vector<Geometry<T>> load_from_obj(std::shared_ptr<Core> core_ptr, std::string filename, const float **colors_array, const uint32_t colors_array_size);
		Geometry(std::shared_ptr<Core> core_ptr, std::vector<T> vertices, D3D11_PRIMITIVE_TOPOLOGY topology, std::string name);
		// This method stores triangles using three textures. 
		//void to_texture(Texture2D &v0_s, Texture2D &v1_s, Texture2D &v2_s);
		const std::vector<T>& get_vertices_vector() const;
		void set_indices(std::vector<unsigned int> indices);
		inline void bind_vertex_buffer();
		inline void bind_index_buffer();
		inline void bind_topology();
		inline void draw(int start_loc);		
		inline void draw_indexed(int start_loc);
	};

	template class __declspec(dllexport) Geometry<Vertex3>;

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
	
}
