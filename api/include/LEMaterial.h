#pragma once
#include <LEShader.h>
#include <LETexture.h>
#include <LEMaterialParameters.h>

namespace LightEngine {

	namespace Materials {

		template <class T>
		class Material : protected ConstantBuffer {
		protected:
			std::string name_;
			T parameters;
			std::shared_ptr<VertexShader> vs_ptr_ = nullptr;
			std::shared_ptr<PixelShader> ps_ptr_ = nullptr;
			std::vector<std::string> maps_names_;
			Material(std::shared_ptr<Core> core_ptr, std::string name);
			void bind_shaders() const;
		public:
			virtual void bind() const = 0;
			void assign_vertex_shader(std::shared_ptr<VertexShader> vs_ptr);
			void assign_pixel_shader(std::shared_ptr<PixelShader> ps_ptr);
			void update();
			T get_all_parameters() const;
			std::string get_name() const;
			virtual std::vector<std::string> get_maps_names() const = 0;
		};

		class __declspec(dllexport) BasicMaterial : public Material<DefaultParameters> {
		private:
			std::shared_ptr<Texture> diffuse_map_ptr_ = nullptr;
			std::shared_ptr<Texture> normal_map_ptr_ = nullptr;
		public:
			BasicMaterial(std::shared_ptr<Core> core_ptr, std::string name);
			void bind() const override;
			void set_diffuse_map(std::shared_ptr<Texture> diffuse_map_ptr);
			void set_normal_map(std::shared_ptr<Texture> normal_map_ptr);
			void set_quadratic_att(float value);
			void set_linear_att(float value);
			void set_constant_att(float value);
			void set_specular_level(float value);
			void set_glossiness(int value);
			void set_specular_color(float specular[3]);
			void set_diffuse_color(float diffuse[3]);
			void set_ambient_color(float ambient[3]);
			void flip_tangent();
			void flip_bitangent();
			std::vector<std::string> get_maps_names() const override;
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
			void set_metalness(float metalness);
			void set_albedo_map(std::shared_ptr<Texture> albedo_map_ptr);
			void set_roughness_map(std::shared_ptr<Texture> roughness_map_ptr);
			void set_metalness_map(std::shared_ptr<Texture> metalness_map_ptr);
			void set_normal_map(std::shared_ptr<Texture> normal_map_ptr);
			void set_ao_map(std::shared_ptr<Texture> ao_map_ptr);
			std::vector<std::string> get_maps_names() const override;
		};
	}
}