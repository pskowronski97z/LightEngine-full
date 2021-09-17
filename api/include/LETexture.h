#pragma once
#pragma comment(lib,"opencv_world450d")
#include <LECore.h>
#include <opencv2/opencv.hpp>
#include <memory>


namespace LightEngine {

	class __declspec(dllexport) Texture : private CoreUser {
	private:
		std::string name_;
		cv::Mat texture_buffer_;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture_ptr_;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> resource_view_ptr_;

	public:
		Texture(std::shared_ptr<Core> core_ptr, std::string texture_path);
		void bind(short slot) const;
		std::string get_name() const;
	};


	class __declspec(dllexport) Sampler : private CoreUser {
	private:
		D3D11_SAMPLER_DESC sampler_desc_;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_state_ptr_;
	public:
		enum class Filtering {
			NEAREST = D3D11_FILTER_MIN_MAG_MIP_POINT,
			BILINEAR  = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT,
			TRILINEAR = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
			ANISOTROPIC = D3D11_FILTER_ANISOTROPIC
		};
		Sampler(std::shared_ptr<Core> core_ptr, Filtering filtering);
		void bind(short slot) const;		
	};
}