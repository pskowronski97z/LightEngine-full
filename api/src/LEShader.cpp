#pragma comment(lib,"opencv_world450d")
#include <opencv2/opencv.hpp>
#include <LEShader.h>
#include <d3dcompiler.h>

#define FILE_NOT_FOUND 0x80070002
#define valid_slot(SLOT, MAX_SLOTS) ((SLOT >= 0) && (SLOT <= (MAX_SLOTS - 1)))


LightEngine::ConstantBuffer::ConstantBuffer(std::shared_ptr<Core> core_ptr) : CoreUser(core_ptr) {}

void LightEngine::ConstantBuffer::bind_ps_buffer(short slot) const {
	core_ptr_->get_context_ptr()->PSSetConstantBuffers(slot, 1, constant_buffer_ptr_.GetAddressOf());
}

void LightEngine::ConstantBuffer::bind_vs_buffer(short slot) const {
	core_ptr_->get_context_ptr()->VSSetConstantBuffers(slot, 1, constant_buffer_ptr_.GetAddressOf());
}


template<class T> LightEngine::Shader<T>::Shader(std::shared_ptr<Core> core_ptr) : CoreUser(core_ptr), shader_ptr_(nullptr) {}


LightEngine::VertexShader::VertexShader(std::shared_ptr<Core> core_ptr, std::wstring compiled_shader_path) : Shader(core_ptr) {

	Microsoft::WRL::ComPtr<ID3DBlob> blob_ptr;

	call_result_ = D3DReadFileToBlob(compiled_shader_path.c_str(), &blob_ptr);

	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Cannot load compiled vertex shader> ", "LEShader.cpp",__LINE__, call_result_);


	call_result_ = core_ptr_->get_device_ptr()->CreateVertexShader(
		blob_ptr->GetBufferPointer(),
		blob_ptr->GetBufferSize(),
		nullptr,
		&shader_ptr_);

	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Vertex shader initialization fail> ", "LEShader.cpp",__LINE__, call_result_);
	
	call_result_ = core_ptr_->get_device_ptr()->CreateInputLayout(
		Vertex3::vertex_desc_,
		LE_VERTEX3_COMPONENTS_COUNT,
		blob_ptr->GetBufferPointer(),
		blob_ptr->GetBufferSize(),
		&layout_ptr_);

	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Input layout initialization fail> ", "LEShader.cpp",__LINE__, call_result_);
}

void LightEngine::VertexShader::bind() const {
	core_ptr_->get_context_ptr()->IASetInputLayout(layout_ptr_.Get());
	core_ptr_->get_context_ptr()->VSSetShader(shader_ptr_.Get(), nullptr, 0);
}


LightEngine::PixelShader::PixelShader(std::shared_ptr<Core> core_ptr, std::wstring compiled_shader_path) : Shader(core_ptr) {
	
	Microsoft::WRL::ComPtr<ID3DBlob> blob_ptr;

	call_result_ = D3DReadFileToBlob(compiled_shader_path.c_str(), &blob_ptr);

	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Cannot load compiled pixel shader> ", "LEShader.cpp",__LINE__, call_result_);

	call_result_ = core_ptr_->get_device_ptr()->CreatePixelShader(
		blob_ptr->GetBufferPointer(),
		blob_ptr->GetBufferSize(),
		nullptr,
		&shader_ptr_);

	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Pixel shader initialization fail> ", "LEShader.cpp",__LINE__, call_result_);

}

void LightEngine::PixelShader::bind() const {
	core_ptr_->get_context_ptr()->PSSetShader(shader_ptr_.Get(), nullptr, 0);
}


LightEngine::ComputeShader::ComputeShader(std::shared_ptr<Core> core_ptr, std::wstring compiled_shader_path) : Shader(core_ptr) {
	/*
	static const UINT flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG;
	static const LPCSTR compilation_profile = "cs_5_0";
	static const D3D_SHADER_MACRO shader_macros[] = { "zero", "0", NULL, NULL };

	Microsoft::WRL::ComPtr<ID3DBlob> blob_ptr;
	Microsoft::WRL::ComPtr<ID3DBlob> err_blob_ptr;

	call_result_ = D3DCompileFromFile(
		compiled_shader_path.c_str(),
		shader_macros,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		NULL,
		compilation_profile,
		flags,
		0,
		blob_ptr.GetAddressOf(),
		err_blob_ptr.GetAddressOf());
	
	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Compute shader compilation fail> ", "LEShader.cpp",__LINE__, call_result_);
	*/

	Microsoft::WRL::ComPtr<ID3DBlob> blob_ptr;

	call_result_ = D3DReadFileToBlob(compiled_shader_path.c_str(), &blob_ptr);

	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Cannot load compiled compute shader> ", "LEShader.cpp",__LINE__, call_result_);

	call_result_ = core_ptr_->get_device_ptr()->CreateComputeShader(blob_ptr->GetBufferPointer(), blob_ptr->GetBufferSize(), NULL, shader_ptr_.GetAddressOf());

	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Compute shader initialization fail> ", "LEShader.cpp",__LINE__, call_result_);
}

void LightEngine::ComputeShader::bind() const {
	core_ptr_->get_context_ptr()->CSSetShader(shader_ptr_.Get(), NULL, 0);
}

void LightEngine::ComputeShader::run(uint16_t x_groups_count, uint16_t y_groups_count, uint16_t z_groups_count) const {
	core_ptr_->get_context_ptr()->Dispatch(x_groups_count, y_groups_count, z_groups_count);
	
}


template<class T> 
LightEngine::CBuffer<T>::CBuffer(std::shared_ptr<Core> core_ptr, const std::string& name, const T* const data, const uint32_t size_in_bytes) {

	shader_resource_name_ = name;
	core_ptr_ = core_ptr;

	D3D11_BUFFER_DESC buffer_desc;
	D3D11_SUBRESOURCE_DATA sr_data;

	buffer_desc.ByteWidth = size_in_bytes;
	buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	buffer_desc.MiscFlags = 0u;
	buffer_desc.StructureByteStride = 4u;

	sr_data.pSysMem = data;

	call_result_ = core_ptr_->get_device_ptr()->CreateBuffer(&buffer_desc, &sr_data, &buffer_ptr_);
	// TODO - Fix exceptions for deriving classes
	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Constant buffer creation failed> ", "LEShader.cpp", __LINE__, call_result_);

}

template<class T>
void LightEngine::CBuffer<T>::update(const T* const data, const uint32_t size_in_bytes) {

	D3D11_MAPPED_SUBRESOURCE new_constant_buffer;
	ZeroMemory(&new_constant_buffer, sizeof(new_constant_buffer));

	call_result_ = core_ptr_->get_context_ptr()->Map(buffer_ptr_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &new_constant_buffer);

	if (FAILED(call_result_))
		throw LECoreException("<Shader constant buffer mapping failed> ", "LEShader.cpp", __LINE__, call_result_);

	memcpy(new_constant_buffer.pData, data, size_in_bytes);

	core_ptr_->get_context_ptr()->Unmap(buffer_ptr_.Get(), 0);

}


LightEngine::ShaderResourceManager::ShaderResourceManager(const std::shared_ptr<Core> &core_ptr) : CoreUser(core_ptr) {}

bool LightEngine::ShaderResourceManager::bind_texture_buffer(AbstractTexture &texture, const ShaderType shader_type, const uint8_t slot) const {
	
	if(!valid_slot(slot, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT))
		return false;

	switch(shader_type) {

	case ShaderType::VertexShader :
		core_ptr_->get_context_ptr()->VSSetShaderResources(slot, 1, texture.srv_ptr_.GetAddressOf());
		break;

	case ShaderType::GeometryShader :
		core_ptr_->get_context_ptr()->GSSetShaderResources(slot, 1, texture.srv_ptr_.GetAddressOf());
		break;

	case ShaderType::HullShader :
		core_ptr_->get_context_ptr()->HSSetShaderResources(slot, 1, texture.srv_ptr_.GetAddressOf());
		break;

	case ShaderType::DomainShader :
		core_ptr_->get_context_ptr()->DSSetShaderResources(slot, 1, texture.srv_ptr_.GetAddressOf());
		break;
	
	case ShaderType::PixelShader :	
		core_ptr_->get_context_ptr()->PSSetShaderResources(slot, 1, texture.srv_ptr_.GetAddressOf());
		break;

	case ShaderType::ComputeShader :
		core_ptr_->get_context_ptr()->CSSetShaderResources(slot, 1, texture.srv_ptr_.GetAddressOf());
		break;

	default:
		return false;
		break;
	}

	texture.bound_to_shader_ = shader_type;
	texture.bound_to_slot_ = slot;

	return true;
}

template<class T>
bool LightEngine::ShaderResourceManager::bind_constant_buffer(CBuffer<T> &cbuffer, const ShaderType shader_type, const uint8_t slot) const {

	if (!valid_slot(slot, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT))
		return false;

	switch (shader_type) {

	case ShaderType::VertexShader:
		core_ptr_->get_context_ptr()->VSSetConstantBuffers(slot, 1u, cbuffer.buffer_ptr_.GetAddressOf());
		break;

	case ShaderType::GeometryShader:
		core_ptr_->get_context_ptr()->GSSetConstantBuffers(slot, 1u, cbuffer.buffer_ptr_.GetAddressOf());
		break;

	case ShaderType::HullShader:
		core_ptr_->get_context_ptr()->HSSetConstantBuffers(slot, 1u, cbuffer.buffer_ptr_.GetAddressOf());
		break;

	case ShaderType::DomainShader:
		core_ptr_->get_context_ptr()->DSSetConstantBuffers(slot, 1u, cbuffer.buffer_ptr_.GetAddressOf());
		break;

	case ShaderType::PixelShader:
		core_ptr_->get_context_ptr()->PSSetConstantBuffers(slot, 1u, cbuffer.buffer_ptr_.GetAddressOf());
		break;

	case ShaderType::ComputeShader:
		core_ptr_->get_context_ptr()->CSSetConstantBuffers(slot, 1u, cbuffer.buffer_ptr_.GetAddressOf());
		break;

	default:
		return false;
		break;
	}

	cbuffer.bound_to_shader_ = shader_type;
	cbuffer.bound_to_slot_ = slot;

	return true;

}


bool LightEngine::ShaderResourceManager::bind_cs_unordered_access_buffer(AbstractTexture &texture, const uint8_t slot) const {

	if(!valid_slot(slot, D3D11_1_UAV_SLOT_COUNT))
		return false;

	core_ptr_->get_context_ptr()->CSSetUnorderedAccessViews(slot, 1, texture.uav_ptr_.GetAddressOf(), nullptr);
	texture.bound_to_shader_ = ShaderType::ComputeShader;
	texture.bound_to_slot_ = slot;
	return true;
}

bool LightEngine::ShaderResourceManager::unbind_texture_buffer(const ShaderType shader_type, const uint8_t slot) const {
	
	static ID3D11ShaderResourceView* const null_srv = 0;

	if(!valid_slot(slot, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT))
		return false;

	switch(shader_type) {

	case ShaderType::VertexShader :
		core_ptr_->get_context_ptr()->VSSetShaderResources(slot, 1, &null_srv);
		break;

	case ShaderType::GeometryShader :
		core_ptr_->get_context_ptr()->GSSetShaderResources(slot, 1, &null_srv);
		break;

	case ShaderType::HullShader :
		core_ptr_->get_context_ptr()->HSSetShaderResources(slot, 1, &null_srv);
		break;

	case ShaderType::DomainShader :
		core_ptr_->get_context_ptr()->DSSetShaderResources(slot, 1, &null_srv);
		break;
	
	case ShaderType::PixelShader :	
		core_ptr_->get_context_ptr()->PSSetShaderResources(slot, 1, &null_srv);
		break;

	case ShaderType::ComputeShader :
		core_ptr_->get_context_ptr()->CSSetShaderResources(slot, 1, &null_srv);
		break;

	default:
		return false;
		break;
	}

	return true;
}

bool LightEngine::ShaderResourceManager::unbind_cs_unordered_access_buffer(const uint8_t slot) const {
	
	static ID3D11UnorderedAccessView* const null_uav = 0;

	if(!valid_slot(slot, D3D11_1_UAV_SLOT_COUNT))
		return false;

	core_ptr_->get_context_ptr()->CSSetUnorderedAccessViews(slot, 1, &null_uav, nullptr);
	return true;

}

bool LightEngine::ShaderResourceManager::unbind_constant_buffer(const ShaderType shader_type, const uint8_t slot) const {

	static ID3D11Buffer* const null_buffer = 0;

	if (!valid_slot(slot, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT))
		return false;

	switch (shader_type) {

	case ShaderType::VertexShader:
		core_ptr_->get_context_ptr()->VSSetConstantBuffers(slot, 1, &null_buffer);
		break;

	case ShaderType::GeometryShader:
		core_ptr_->get_context_ptr()->GSSetConstantBuffers(slot, 1, &null_buffer);
		break;

	case ShaderType::HullShader:
		core_ptr_->get_context_ptr()->HSSetConstantBuffers(slot, 1, &null_buffer);
		break;

	case ShaderType::DomainShader:
		core_ptr_->get_context_ptr()->DSSetConstantBuffers(slot, 1, &null_buffer);
		break;

	case ShaderType::PixelShader:
		core_ptr_->get_context_ptr()->PSSetConstantBuffers(slot, 1, &null_buffer);
		break;

	case ShaderType::ComputeShader:
		core_ptr_->get_context_ptr()->CSSetConstantBuffers(slot, 1, &null_buffer);
		break;

	default:
		return false;
		break;
	}

	return true;


}


LightEngine::ShaderResource::ShaderResource() : shader_resource_name_(""), bound_to_slot_(255), bound_to_shader_(ShaderType::Null) {}

std::string LightEngine::ShaderResource::get_shader_resource_name() const {	return shader_resource_name_; }

LightEngine::ShaderType LightEngine::ShaderResource::get_bound_shader_type() const { return bound_to_shader_; }

uint8_t LightEngine::ShaderResource::get_bound_slot_number() const { return bound_to_slot_; }


LightEngine::AbstractTexture::AbstractTexture() : srv_ptr_(nullptr), uav_ptr_(nullptr), rtv_ptr_(nullptr) {}

void LightEngine::AbstractTexture::generate_mip_maps() const {
	core_ptr_->get_context_ptr()->GenerateMips(srv_ptr_.Get());
}


LightEngine::Texture2D::Texture2D(const std::shared_ptr<Core> &core_ptr, const std::string& texture_path) {

	core_ptr_ = core_ptr;

	cv::Mat texture_buffer_ = cv::imread(texture_path);
	if (texture_buffer_.data == nullptr)
		throw LECoreException("<ERROR> <Texture cannot be loaded. File is missing, no permission or invalid format.>", "LEShader.cpp", __LINE__ - 2, FILE_NOT_FOUND);

	shader_resource_name_ = "";
	int path_str_last_char = texture_path.size() - 1;
	char readen;

	do {
		readen = texture_path[path_str_last_char];
		shader_resource_name_.insert(shader_resource_name_.begin(), readen);
		path_str_last_char--;
	} while ((texture_path[path_str_last_char] != '\\') && (path_str_last_char != -1));

	cv::Mat converted;

	cv::cvtColor(texture_buffer_, converted, cv::COLOR_BGR2BGRA);

	descriptor_.Width = texture_buffer_.cols;
	descriptor_.Height = texture_buffer_.rows;
	descriptor_.MipLevels = 0u;
	descriptor_.ArraySize = 1u;
	descriptor_.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	descriptor_.SampleDesc.Count = 1u;
	descriptor_.SampleDesc.Quality = 0u;
	descriptor_.Usage = D3D11_USAGE_DEFAULT;
	descriptor_.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;
	descriptor_.CPUAccessFlags = 0;
	descriptor_.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	call_result_ = core_ptr->get_device_ptr()->CreateTexture2D(&descriptor_, nullptr, &ptr_);
	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Texture2D creation failed>", "LEShader.cpp", __LINE__ - 2, call_result_);

	core_ptr->get_context_ptr()->UpdateSubresource(ptr_.Get(), 0u, nullptr, converted.data, texture_buffer_.cols * 4 * sizeof(char), 0);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
	srvd.Format = descriptor_.Format;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvd.Texture2D.MostDetailedMip = 0;
	srvd.Texture2D.MipLevels = -1;

	call_result_ = core_ptr->get_device_ptr()->CreateShaderResourceView(ptr_.Get(), &srvd, &srv_ptr_);
	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Texture2D resource view creation failed>", "LEShader.cpp", __LINE__ - 2, call_result_);
	
	D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};

	uav_desc.Buffer.FirstElement = 0;
	uav_desc.Buffer.Flags = 0;
	uav_desc.Buffer.NumElements = texture_buffer_.cols * texture_buffer_.rows;
	uav_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	
	call_result_ = core_ptr_->get_device_ptr()->CreateUnorderedAccessView(ptr_.Get(), &uav_desc, uav_ptr_.GetAddressOf());
	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Texture2D unordered access view creation failed>", "LEShader.cpp", __LINE__ - 2, call_result_);

	call_result_ = core_ptr_->get_device_ptr()->CreateRenderTargetView(ptr_.Get(), nullptr, rtv_ptr_.GetAddressOf());
	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Texture2D render target view creation failed>", "LEShader.cpp", __LINE__ - 2, call_result_);

}

LightEngine::Texture2D::Texture2D(const std::shared_ptr<Core> &core_ptr, const std::string& name, const uint16_t width, const uint16_t height, const float* data) : 
	width_(width), height_(height) {

	core_ptr_ = core_ptr;
	shader_resource_name_ = name;

	descriptor_ = {0};

	descriptor_.Width = width_;
	descriptor_.Height = height_;
	descriptor_.MipLevels = 0u; 
	descriptor_.ArraySize = 1u;
	descriptor_.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	descriptor_.SampleDesc.Count = 1u;
	descriptor_.SampleDesc.Quality = 0u;
	descriptor_.Usage = D3D11_USAGE_DEFAULT;
	descriptor_.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_RENDER_TARGET;
	descriptor_.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	descriptor_.CPUAccessFlags = 0;

	call_result_ = core_ptr_->get_device_ptr()->CreateTexture2D(&descriptor_, nullptr, &ptr_);
	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Texture2D creation failed>", "LETexture.cpp", __LINE__ - 2, call_result_);

	core_ptr->get_context_ptr()->UpdateSubresource(ptr_.Get(), 0u, nullptr, data, static_cast<uint32_t>(width_) * 4 * sizeof(float), 0);

	static const D3D11_SHADER_RESOURCE_VIEW_DESC srvd = { 
		descriptor_.Format,
		D3D11_SRV_DIMENSION_TEXTURE2D,
		{0,-1}
	};

	call_result_ = core_ptr->get_device_ptr()->CreateShaderResourceView(ptr_.Get(), &srvd, &srv_ptr_);
	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Texture2D resource view creation failed>", "LETexture.cpp", __LINE__ - 2, call_result_);

	static D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;

	uav_desc.Buffer.FirstElement = 0;
	uav_desc.Buffer.Flags = 0;
	uav_desc.Buffer.NumElements = width_ * height_;
	uav_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	
	call_result_ = core_ptr_->get_device_ptr()->CreateUnorderedAccessView(ptr_.Get(), &uav_desc, uav_ptr_.GetAddressOf());
	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Texture2D unordered access view creation failed>", "LETexture.cpp", __LINE__ - 2, call_result_);

	call_result_ = core_ptr_->get_device_ptr()->CreateRenderTargetView(ptr_.Get(), nullptr, rtv_ptr_.GetAddressOf());
	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Texture2D render target view creation failed>", "LEShader.cpp", __LINE__ - 2, call_result_);

}

LightEngine::Texture2D::Texture2D(const std::shared_ptr<Core>& core_ptr, const std::string& name, const uint16_t width, const uint16_t height, const uint32_t *data) :
	width_(width), height_(height) {

	core_ptr_ = core_ptr;
	shader_resource_name_ = name;

	descriptor_ = { 0 };

	descriptor_.Width = width_;
	descriptor_.Height = height_;
	descriptor_.MipLevels = 0u;
	descriptor_.ArraySize = 1u;
	descriptor_.Format = DXGI_FORMAT_R32_UINT;
	descriptor_.SampleDesc.Count = 1u;
	descriptor_.SampleDesc.Quality = 0u;
	descriptor_.Usage = D3D11_USAGE_DEFAULT;
	descriptor_.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_RENDER_TARGET;
	descriptor_.CPUAccessFlags = 0;

	call_result_ = core_ptr_->get_device_ptr()->CreateTexture2D(&descriptor_, nullptr, &ptr_);
	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Texture2D creation failed>", "LETexture.cpp", __LINE__ - 2, call_result_);

	core_ptr->get_context_ptr()->UpdateSubresource(
		ptr_.Get(), 
		0u, 
		nullptr, data, 
		static_cast<uint32_t>(width_) * sizeof(uint32_t), 0);

	static const D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {
		descriptor_.Format,
		D3D11_SRV_DIMENSION_TEXTURE2D,
		{0,-1}
	};

	call_result_ = core_ptr->get_device_ptr()->CreateShaderResourceView(ptr_.Get(), &srvd, &srv_ptr_);
	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Texture2D resource view creation failed>", "LETexture.cpp", __LINE__ - 2, call_result_);

	static D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;

	uav_desc.Buffer.FirstElement = 0;
	uav_desc.Buffer.Flags = 0;
	uav_desc.Buffer.NumElements = width_ * height_;
	uav_desc.Format = DXGI_FORMAT_R32_UINT;
	uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;

	call_result_ = core_ptr_->get_device_ptr()->CreateUnorderedAccessView(ptr_.Get(), &uav_desc, uav_ptr_.GetAddressOf());
	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Texture2D unordered access view creation failed>", "LETexture.cpp", __LINE__ - 2, call_result_);

	call_result_ = core_ptr_->get_device_ptr()->CreateRenderTargetView(ptr_.Get(), nullptr, rtv_ptr_.GetAddressOf());
	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Texture2D render target view creation failed>", "LEShader.cpp", __LINE__ - 2, call_result_);

}

LightEngine::Texture2D LightEngine::Texture2D::store_geometry(const std::shared_ptr<Core>& core_ptr, const std::vector<Vertex3>& vertices) {

	static std::vector<float> processed_data;
	const uint32_t vertices_count = vertices.size();
	const uint32_t row_width = vertices_count * 4;

	processed_data.resize( row_width * 6, 0.0);

	const uint32_t texture_coords_row_index = 2 * row_width;
	const uint32_t normal_vector_row_index = 3 * row_width;
	const uint32_t tangent_vector_row_index = 4 * row_width;
	const uint32_t bitangent_vector_row_index = 5 * row_width;

	uint32_t itr = 0;

	for (auto& vertex : vertices) {

		processed_data.at(itr) = vertex.position_.x;
		processed_data.at(itr + 1) = vertex.position_.y;
		processed_data.at(itr + 2) = vertex.position_.z;
		processed_data.at(itr + 3) = 0.0;

		processed_data.at(row_width + itr) = vertex.color_.x;
		processed_data.at(row_width + itr + 1) = vertex.color_.y;
		processed_data.at(row_width + itr + 2) = vertex.color_.z;
		processed_data.at(row_width + itr + 3) = 0.0;

		processed_data.at(texture_coords_row_index  + itr) = vertex.texcoords_.x;
		processed_data.at(texture_coords_row_index  + itr + 1) = vertex.texcoords_.y;
		processed_data.at(texture_coords_row_index  + itr + 2) = vertex.texcoords_.z;
		processed_data.at(texture_coords_row_index  + itr + 3) = 0.0;

		processed_data.at(normal_vector_row_index  + itr) = vertex.normal_.x;
		processed_data.at(normal_vector_row_index  + itr + 1) = vertex.normal_.y;
		processed_data.at(normal_vector_row_index  + itr + 2) = vertex.normal_.z;
		processed_data.at(normal_vector_row_index  + itr + 3) = 0.0;

		processed_data.at(tangent_vector_row_index  + itr) = vertex.tangent_.x;
		processed_data.at(tangent_vector_row_index  + itr + 1) = vertex.tangent_.y;
		processed_data.at(tangent_vector_row_index  + itr + 2) = vertex.tangent_.z;
		processed_data.at(tangent_vector_row_index  + itr + 3) = 0.0;

		processed_data.at(bitangent_vector_row_index  + itr) = vertex.bitangent_.x;
		processed_data.at(bitangent_vector_row_index  + itr + 1) = vertex.bitangent_.y;
		processed_data.at(bitangent_vector_row_index  + itr + 2) = vertex.bitangent_.z;
		processed_data.at(bitangent_vector_row_index  + itr + 3) = 0.0;

		itr += 4;
	}

	return Texture2D(core_ptr, "Geometry data", vertices_count, 6u, processed_data.data());

}

uint16_t LightEngine::Texture2D::get_width() const { return width_; }

uint16_t LightEngine::Texture2D::get_height() const { return height_; }


LightEngine::Texture3D::Texture3D(
	const std::shared_ptr<Core> &core_ptr, 
	const std::string& name, 
	const uint16_t width,
	const uint16_t height, 
	const uint16_t depth, 
	const float* data) : 
	width_(width), height_(height), depth_(depth) {

	core_ptr_ = core_ptr;
	shader_resource_name_ = name;

	descriptor_ = {0};

	descriptor_.Width = width_;
	descriptor_.Height = height_;
	descriptor_.Depth = depth_;
	descriptor_.MipLevels = 0u; 
	descriptor_.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	descriptor_.Usage = D3D11_USAGE_DEFAULT;
	descriptor_.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_RENDER_TARGET ;
	descriptor_.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	descriptor_.CPUAccessFlags = 0;

	call_result_ = core_ptr_->get_device_ptr()->CreateTexture3D(&descriptor_, nullptr, &ptr_);
	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Texture resource creation failed>", "LEShader.cpp", __LINE__ - 2, call_result_);

	core_ptr->get_context_ptr()->UpdateSubresource(ptr_.Get(),
		0u,
		nullptr,
		data,
		static_cast<uint32_t>(width_) * 4 * sizeof(float),
		static_cast<uint32_t>(width_) * static_cast<uint32_t>(height_) * 4 * sizeof(float));

	static const D3D11_SHADER_RESOURCE_VIEW_DESC srvd = { 
		descriptor_.Format,
		D3D11_SRV_DIMENSION_TEXTURE3D,
		{0,-1}
	};

	call_result_ = core_ptr->get_device_ptr()->CreateShaderResourceView(ptr_.Get(), &srvd, &srv_ptr_);
	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Shader resource view creation failed>", "LEShader.cpp", __LINE__ - 2, call_result_);

	static D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;

	uav_desc.Texture3D.FirstWSlice = 0u;
	uav_desc.Texture3D.MipSlice = 0u;
	uav_desc.Texture3D.WSize = depth_;
	uav_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
	
	call_result_ = core_ptr_->get_device_ptr()->CreateUnorderedAccessView(ptr_.Get(), &uav_desc, uav_ptr_.GetAddressOf());
	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <UAV creation failed>", "LEShader.cpp", __LINE__ - 2, call_result_);
}

LightEngine::Texture3D::Texture3D(
	const std::shared_ptr<Core>& core_ptr, 
	const std::string& name, 
	const uint16_t width,
	const uint16_t height, 
	const uint16_t depth, 
	const int32_t* data) :
	width_(width), height_(height), depth_(depth) {

	core_ptr_ = core_ptr;
	shader_resource_name_ = name;

	descriptor_ = { 0 };

	descriptor_.Width = width_;
	descriptor_.Height = height_;
	descriptor_.Depth = depth_;
	descriptor_.MipLevels = 0u;
	descriptor_.Format = DXGI_FORMAT_R32_SINT;
	descriptor_.Usage = D3D11_USAGE_DEFAULT;
	descriptor_.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_RENDER_TARGET;
	//descriptor_.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	descriptor_.CPUAccessFlags = 0;

	call_result_ = core_ptr_->get_device_ptr()->CreateTexture3D(&descriptor_, nullptr, &ptr_);
	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Texture resource creation failed>", "LEShader.cpp", __LINE__ - 2, call_result_);

	core_ptr->get_context_ptr()->UpdateSubresource(ptr_.Get(),
		0u,
		nullptr,
		data,
		static_cast<uint32_t>(width_) * sizeof(int32_t),
		static_cast<uint32_t>(width_) * static_cast<uint32_t>(height_) * sizeof(int32_t));

	static const D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {
		descriptor_.Format,
		D3D11_SRV_DIMENSION_TEXTURE3D,
		{0,-1}
	};

	call_result_ = core_ptr->get_device_ptr()->CreateShaderResourceView(ptr_.Get(), &srvd, &srv_ptr_);
	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Shader resource view creation failed>", "LEShader.cpp", __LINE__ - 2, call_result_);

	static D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;

	uav_desc.Texture3D.FirstWSlice = 0u;
	uav_desc.Texture3D.MipSlice = 0u;
	uav_desc.Texture3D.WSize = depth_;
	uav_desc.Format = DXGI_FORMAT_R32_SINT;
	uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;

	call_result_ = core_ptr_->get_device_ptr()->CreateUnorderedAccessView(ptr_.Get(), &uav_desc, uav_ptr_.GetAddressOf());
	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <UAV creation failed>", "LEShader.cpp", __LINE__ - 2, call_result_);
}

uint16_t LightEngine::Texture3D::get_width() const { return width_; }

uint16_t LightEngine::Texture3D::get_height() const { return height_; }

uint16_t LightEngine::Texture3D::get_depth() const { return depth_; }
