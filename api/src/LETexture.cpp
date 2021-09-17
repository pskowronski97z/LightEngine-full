#include "LETexture.h"
#include <LEException.h>

#define FILE_NOT_FOUND 0x80070002

LightEngine::Texture::Texture(std::shared_ptr<Core> core_ptr, std::string texture_path) : CoreUser(core_ptr) {
	texture_buffer_ = cv::imread(texture_path);
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

	D3D11_TEXTURE2D_DESC texture_descriptor = { 0 };	
	cv::Mat converted;

	cv::cvtColor(texture_buffer_, converted, cv::COLOR_BGR2BGRA);

	texture_descriptor.Width = texture_buffer_.cols;
	texture_descriptor.Height = texture_buffer_.rows;
	texture_descriptor.MipLevels = 0;
	texture_descriptor.ArraySize = 1;
	texture_descriptor.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	texture_descriptor.SampleDesc.Count = 1u;
	texture_descriptor.SampleDesc.Quality = 0u;
	texture_descriptor.Usage = D3D11_USAGE_DEFAULT;
	texture_descriptor.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texture_descriptor.CPUAccessFlags = 0;
	texture_descriptor.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	call_result_ = core_ptr->get_device_ptr()->CreateTexture2D(&texture_descriptor, nullptr, &texture_ptr_);
	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Texture resource creation failed>", "LETexture.cpp", __LINE__ - 2, call_result_);

	core_ptr->get_context_ptr()->UpdateSubresource(texture_ptr_.Get(),0u,nullptr,converted.data,texture_buffer_.cols * 4 * sizeof(char),0);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
	srvd.Format = texture_descriptor.Format;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvd.Texture2D.MostDetailedMip = 0;
	srvd.Texture2D.MipLevels = -1;

	call_result_ = core_ptr->get_device_ptr()->CreateShaderResourceView(texture_ptr_.Get(), &srvd, &resource_view_ptr_);
	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Shader resource view creation failed>", "LETexture.cpp", __LINE__ - 2, call_result_);

	core_ptr_->get_context_ptr()->GenerateMips(resource_view_ptr_.Get());

}

void LightEngine::Texture::bind(short slot) const {
	core_ptr_->get_context_ptr()->PSSetShaderResources(slot, 1, resource_view_ptr_.GetAddressOf());
}

std::string LightEngine::Texture::get_name() const { return name_;}



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

