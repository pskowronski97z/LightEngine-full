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


	namespace GeometryTools {
		/// <summary>
		/// This funtion performs an evaluation of triangles which are surrounding a 
		/// single triangle given by its vertices v0, v1, v2.
		/// </summary>
		/// <param name="r"> - Maximum distance between given triangle center and neighbour's center</param>
		/// <param name="triangle_v0"> - First vertex of processed triangle</param>
		/// <param name="triangle_v1"> - Second vertex of processed triangle</param>
		/// <param name="triangle_v2"> - Third vertex od processed triangle</param>
		/// <param name="evaluated_tris"> - Vertices of evaluated triangles (Triangle list packing expected)</param>
		/// <param name="flags"> - Flag vector of size which is equal to the amount of triangles stored in 
		/// the evaluated triangles vector (eval_tris_size/3). If triangle from that vector passes an evaluation, 
		/// a flag at the corresponging index is being set to true.</param>
		/// <returns>No value is returned. This function modifies values of the flags vector. It was designed to
		/// use for iterating over multiple triangles, using the same flags per one object. </returns>
		__declspec(dllexport) void evaluate_tris(
			const float r,
			const Vertex3 &v0,
			const Vertex3 &v1,
			const Vertex3 &v2,
			const std::vector<Vertex3> &evaluated_tris,
			std::vector<bool> &flags);
		
		__declspec(dllexport) std::vector<uint32_t> calculate_neighbouring_geometry(
			const float r,
			const std::vector<Vertex3>& testing_triangles,
			const std::vector<Vertex3>& tested_triangles);

		__declspec(dllexport) uint32_t *generate_lookup_matrix(
			const float r,
			const std::vector<Geometry<Vertex3>> &testing_geometry,
			const std::vector<Vertex3> &tested_geometry,
			uint32_t &width);

	}



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
