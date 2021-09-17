#pragma once
#include <Windows.h>

namespace LightEngine {

	struct DefaultParameters {
		float quadratic_att_;
		float linear_att_;
		float constant_att_;
		float specular_level_;
		int glossiness_;
		BOOL use_diffuse_map_;
		BOOL use_normal_map_;
		int flip_tb_vectors_;
		float specular[4];
		float diffuse[4];
		float ambient[4];

		DefaultParameters() {
			quadratic_att_ = 0.3;
			linear_att_ = 0.3;
			constant_att_ = 1.0;
			specular_level_ = 1.0;
			glossiness_ = 400;
			use_diffuse_map_ = FALSE;
			use_normal_map_ = FALSE;
			flip_tb_vectors_ = 0;
			specular[0] = 1.0;
			specular[1] = 1.0;
			specular[2] = 1.0;
			specular[3] = 0.0;

			diffuse[0] = 1.0;
			diffuse[1] = 1.0;
			diffuse[2] = 1.0;
			diffuse[3] = 0.0;

			ambient[0] = 0.0;
			ambient[1] = 0.0;
			ambient[2] = 0.0;
			ambient[3] = 0.0;
		};
	};

	struct PBRParameters {
		float albedo_[4];
		float roughness_;
		float metallic_;
		float ambient_occlusion_;
		BOOL use_albedo_map_;
		BOOL use_roughness_map_;
		BOOL use_metalness_map_;
		BOOL use_normal_map_;
		BOOL use_ao_map_;

		PBRParameters() {
			albedo_[0] = 0.5f;
			albedo_[1] = 0.5f;
			albedo_[2] = 0.5f;
			albedo_[3] = 0.5f;
			roughness_ = 0.0f;
			metallic_ = 0.0f;
			ambient_occlusion_ = 0.0f;
			use_albedo_map_ = FALSE;
			use_roughness_map_ = FALSE;
			use_metalness_map_ = FALSE;
			use_normal_map_ = FALSE;
			use_ao_map_ = FALSE;
		};
	};
}