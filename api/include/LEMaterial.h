#pragma once
#include <LEShader.h>
#include <LETexture.h>
#include <LEMaterialParameters.h>

namespace LightEngine {

	template <class T>
	class Material : protected ConstantBuffer {
	protected:		
		std::string name_;
		T parameters;
		std::shared_ptr<VertexShader> vs_ptr_ = nullptr;
		std::shared_ptr<PixelShader> ps_ptr_ = nullptr;
		Material(std::shared_ptr<Core> core_ptr, std::string name);	
		void bind_shaders() const;
	public:
		virtual void bind() const = 0;
		void assign_vertex_shader(std::shared_ptr<VertexShader> vs_ptr);
		void assign_pixel_shader(std::shared_ptr<PixelShader> ps_ptr);
		void update();	
		std::string get_name() const;	
	};

	class __declspec(dllexport) DefaultMaterial : public Material<DefaultParameters> {
	private:	
		std::shared_ptr<Texture> diffuse_map_ptr_ = nullptr;
		std::shared_ptr<Texture> normal_map_ptr_ = nullptr;
	public:
		DefaultMaterial(std::shared_ptr<Core> core_ptr, std::string name);
		void bind() const override;
		void assign_diffuse_map(std::shared_ptr<Texture> diffuse_map_ptr);
		void assign_normal_map(std::shared_ptr<Texture> normal_map_ptr);
		void set_quadratic_att(float value);
		void set_linear_att(float value);
		void set_constant_att(float value);
		void set_specular_level(float value);
		void set_glossiness(int value);
		void set_specular_color(float specular[3]);
		void set_diffuse_color(float diffuse[3]);
		void set_ambient_color(float ambient[3]);
		void select_diffuse_map_usage();
		void select_normal_map_usage();
		void unselect_diffuse_map_usage();
		void unselect_normal_map_usage();
		bool is_diffuse_map_assigned() const;
		bool is_normal_map_assigned() const;
		void flip_tangent();
		void flip_bitangent();
	};

	class __declspec(dllexport) PBRMaterial : public Material<PBRParameters> {
	private:
		std::shared_ptr<Texture> albedo_map_ptr_ = nullptr;
		std::shared_ptr<Texture> roughness_map_ptr_ = nullptr;
		std::shared_ptr<Texture> metalness_map_ptr_ = nullptr;
		std::shared_ptr<Texture> normal_map_ptr_ = nullptr;
		std::shared_ptr<Texture> ao_map_ptr_ = nullptr;
	public:
		PBRMaterial(std::shared_ptr<Core> core_ptr, std::string name);
		void bind() const override;
		void set_albedo(float albedo[3]);
		void set_roughness(float roughness);
		void set_metallic(float metallic);
		void assign_albedo_map(std::shared_ptr<Texture> albedo_map_ptr);
		void assign_roughness_map(std::shared_ptr<Texture> roughness_map_ptr);
		void assign_metalness_map(std::shared_ptr<Texture> metalness_map_ptr);
		void assign_normal_map(std::shared_ptr<Texture> normal_map_ptr);
		void assign_ao_map(std::shared_ptr<Texture> ao_map_ptr);
		/// <summary>
		/// Enables usage of particular maps (if loaded)
		/// </summary>
		/// <param name="flags"> - An array of 5 flags significating usage of:</param>
		/// <param name="flags[0]"> - Albedo map </param>	
		/// <param name="flags[1]"> - Roughness map </param>	
		/// <param name="flags[2]"> - Metalness map </param>	
		/// <param name="flags[3]"> - Normal map </param>	
		/// <param name="flags[4]"> - Ambient occlusion map </param>	
		void set_maps_usage(bool flags[5]);
	};
}