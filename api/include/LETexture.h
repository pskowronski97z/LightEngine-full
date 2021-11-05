#pragma once
#pragma comment(lib,"opencv_world450d")
#include <LECore.h>
#include <opencv2/opencv.hpp>
#include <memory>


namespace LightEngine {

	class __declspec(dllexport) Texture : protected CoreUser {
	protected:
		std::string name_;
		short binding_slot_;
		D3D11_TEXTURE2D_DESC texture_descriptor_;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture_ptr_;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture_srv_ptr_;
		Texture(std::shared_ptr<Core> &core_ptr, std::string name);
	public:
		void bind(short slot);
		void unbind() const;
		std::string get_name() const;
		int get_width() const;
		int get_height() const;
	};

	class __declspec(dllexport) StaticTexture : public Texture {
	public:
		StaticTexture(std::shared_ptr<Core> core_ptr, std::string texture_path);
	};

	class __declspec(dllexport) RenderableTexture : public Texture {
	private:
		float clear_color[4];
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> texture_rtv_ptr_;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> texture_dsv_ptr_;
	public:
		RenderableTexture(std::shared_ptr<Core> &core_ptr, std::string name, int width, int height);
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> get_rtv_ptr() const;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> get_dsv_ptr() const;
		void clear() const;
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