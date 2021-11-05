#include "LETexture.h"
#include <LEException.h>

#define FILE_NOT_FOUND 0x80070002

LightEngine::StaticTexture::StaticTexture(std::shared_ptr<Core> core_ptr, std::string texture_path) : Texture(core_ptr, ""){
	cv::Mat texture_buffer_ = cv::imread(texture_path);
	if (texture_buffer_.data == nullptr)
		throw LECoreException("<ERROR> <Texture cannot be loaded. File is missing, no permission or invalid format.>", "LETexture.cpp", __LINE__ - 2, FILE_NOT_FOUND);

	name_ = "";
	int path_str_last_char = texture_path.size() - 1;
	char readen;

	do {
		readen = texture_path[path_str_last_char];
		name_.insert(name_.begin(), readen);
		path_str_last_char--;
	} while ((texture_path[path_str_last_char] != '\\') && (path_str_last_char != -1));

	cv::Mat converted;

	cv::cvtColor(texture_buffer_, converted, cv::COLOR_BGR2BGRA);

	texture_descriptor_.Width = texture_buffer_.cols;
	texture_descriptor_.Height = texture_buffer_.rows;
	texture_descriptor_.MipLevels = 0;
	texture_descriptor_.ArraySize = 1;
	texture_descriptor_.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	texture_descriptor_.SampleDesc.Count = 1u;
	texture_descriptor_.SampleDesc.Quality = 0u;
	texture_descriptor_.Usage = D3D11_USAGE_DEFAULT;
	texture_descriptor_.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texture_descriptor_.CPUAccessFlags = 0;
	texture_descriptor_.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	call_result_ = core_ptr->get_device_ptr()->CreateTexture2D(&texture_descriptor_, nullptr, &texture_ptr_);
	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Texture resource creation failed>", "LETexture.cpp", __LINE__ - 2, call_result_);

	core_ptr->get_context_ptr()->UpdateSubresource(texture_ptr_.Get(),0u,nullptr,converted.data,texture_buffer_.cols * 4 * sizeof(char),0);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
	srvd.Format = texture_descriptor_.Format;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvd.Texture2D.MostDetailedMip = 0;
	srvd.Texture2D.MipLevels = -1;

	call_result_ = core_ptr->get_device_ptr()->CreateShaderResourceView(texture_ptr_.Get(), &srvd, &texture_srv_ptr_);
	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Shader resource view creation failed>", "LETexture.cpp", __LINE__ - 2, call_result_);

	core_ptr_->get_context_ptr()->GenerateMips(texture_srv_ptr_.Get());

}

LightEngine::Sampler::Sampler(std::shared_ptr<Core> core_ptr, Filtering filtering) : CoreUser(core_ptr){
	D3D11_SAMPLER_DESC sampler_desc_ = {};
	sampler_desc_.Filter = (D3D11_FILTER)filtering;
	sampler_desc_.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc_.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc_.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc_.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
	sampler_desc_.MipLODBias = 0.0f;
	sampler_desc_.MinLOD = 0.0f;
	sampler_desc_.MaxLOD = D3D11_REQ_MIP_LEVELS;

	call_result_ = core_ptr_->get_device_ptr()->CreateSamplerState(&sampler_desc_, &sampler_state_ptr_);
	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Sampler state creation failed>", "LETexture.cpp", __LINE__ - 2, call_result_);
}

void LightEngine::Sampler::bind(short slot) const {
	core_ptr_->get_context_ptr()->PSSetSamplers(slot, 1, sampler_state_ptr_.GetAddressOf());
}

LightEngine::Texture::Texture(std::shared_ptr<Core>& core_ptr, std::string name)
	: CoreUser(core_ptr),
	name_(name),
	binding_slot_(0),
	texture_descriptor_({0}),
	texture_ptr_(nullptr),
	texture_srv_ptr_(nullptr) {}

void LightEngine::Texture::unbind() const {
	static ID3D11ShaderResourceView* const null_srv = { 0 };
	core_ptr_->get_context_ptr()->PSSetShaderResources(binding_slot_, 1, &null_srv);
}

std::string LightEngine::Texture::get_name() const { return name_; }

int LightEngine::Texture::get_width() const { return texture_descriptor_.Width; }

int LightEngine::Texture::get_height() const { return texture_descriptor_.Height; }

void LightEngine::Texture::bind(short slot) {
	binding_slot_ = slot;
	core_ptr_->get_context_ptr()->PSSetShaderResources(binding_slot_, 1, texture_srv_ptr_.GetAddressOf());
}

LightEngine::RenderableTexture::RenderableTexture(std::shared_ptr<Core>& core_ptr, std::string name, int width, int height)
	: Texture(core_ptr, name){

	clear_color[0] = 0.0;
	clear_color[1] = 0.0;
	clear_color[2] = 0.0;
	clear_color[3] = 0.0;

	texture_descriptor_.Width = width;
	texture_descriptor_.Height = height;
	texture_descriptor_.MipLevels = 0u;
	texture_descriptor_.ArraySize = 1u;
	texture_descriptor_.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texture_descriptor_.SampleDesc.Count = 1u;
	texture_descriptor_.SampleDesc.Quality = 0u;
	texture_descriptor_.Usage = D3D11_USAGE_DEFAULT;
	texture_descriptor_.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texture_descriptor_.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture_ptr;

	call_result_ = core_ptr_->get_device_ptr()->CreateTexture2D(&texture_descriptor_, NULL, &texture_ptr);

	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Renderable texture creation failed> ", "LETexture.cpp", __LINE__, call_result_);


	call_result_ = core_ptr_->get_device_ptr()->CreateRenderTargetView(texture_ptr.Get(), nullptr, texture_rtv_ptr_.GetAddressOf());

	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Renderable texture target view creation failed> ", "LETexture.cpp", __LINE__, call_result_);


	D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
	srvd.Format = texture_descriptor_.Format;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvd.Texture2D.MostDetailedMip = 0;
	srvd.Texture2D.MipLevels = -1;

	call_result_ = core_ptr_->get_device_ptr()->CreateShaderResourceView(texture_ptr.Get(), &srvd, &texture_srv_ptr_);
	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Shader resource view creation failed>", "LETexture.cpp", __LINE__ - 2, call_result_);

	core_ptr_->get_context_ptr()->GenerateMips(texture_srv_ptr_.Get());



	D3D11_TEXTURE2D_DESC depth_texture_desc = {};

	depth_texture_desc.Width = width;
	depth_texture_desc.Height = height;
	depth_texture_desc.MipLevels = 0u;
	depth_texture_desc.ArraySize = 1u;
	depth_texture_desc.Format = DXGI_FORMAT_D32_FLOAT;
	depth_texture_desc.SampleDesc.Count = 1u;
	depth_texture_desc.SampleDesc.Quality = 0u;
	depth_texture_desc.Usage = D3D11_USAGE_DEFAULT;
	depth_texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> depth_texture;

	call_result_ = core_ptr_->get_device_ptr()->CreateTexture2D(&depth_texture_desc, nullptr, &depth_texture);

	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Depth texture creation failed> ", "LETexture.cpp", __LINE__, call_result_);

	D3D11_DEPTH_STENCIL_VIEW_DESC view_desc = {};

	view_desc.Format = DXGI_FORMAT_D32_FLOAT;
	view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	view_desc.Texture2D.MipSlice = 0u;

	call_result_ = core_ptr_->get_device_ptr()->CreateDepthStencilView(depth_texture.Get(), &view_desc, &texture_dsv_ptr_);

	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Stencil view creation failed> ", "LETexture.cpp", __LINE__, call_result_);

}

Microsoft::WRL::ComPtr<ID3D11RenderTargetView> LightEngine::RenderableTexture::get_rtv_ptr() const { return texture_rtv_ptr_; }

Microsoft::WRL::ComPtr<ID3D11DepthStencilView> LightEngine::RenderableTexture::get_dsv_ptr() const { return texture_dsv_ptr_; }


void LightEngine::RenderableTexture::clear() const {
	core_ptr_->get_context_ptr()->ClearDepthStencilView(texture_dsv_ptr_.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
	core_ptr_->get_context_ptr()->ClearRenderTargetView(texture_rtv_ptr_.Get(), clear_color);
}
