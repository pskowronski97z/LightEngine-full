#include <LEShader.h>
#include <d3dcompiler.h>

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

void LightEngine::ComputeShader::run() const {
	core_ptr_->get_context_ptr()->Dispatch(500, 500, 1);
}


LightEngine::ShaderResourceManager::ShaderResourceManager(std::shared_ptr<Core> core_ptr) : CoreUser(core_ptr) {}

bool LightEngine::ShaderResourceManager::slot_valid(uint8_t slot, uint8_t max_slots) { return ((slot >= 0) && (slot <= (max_slots - 1)));};

bool LightEngine::ShaderResourceManager::bind_texture_buffer(Texture& texture, ShaderType shader_type, uint8_t slot) {

	if(!slot_valid(slot, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT))
		return false;

	switch(shader_type) {

	case ShaderType::VertexShader :
		core_ptr_->get_context_ptr()->VSSetShaderResources(slot, 1, texture.get_srv_ptr().GetAddressOf());
		break;

	case ShaderType::GeometryShader :
		core_ptr_->get_context_ptr()->GSSetShaderResources(slot, 1, texture.get_srv_ptr().GetAddressOf());
		break;

	case ShaderType::HullShader :
		core_ptr_->get_context_ptr()->HSSetShaderResources(slot, 1, texture.get_srv_ptr().GetAddressOf());
		break;

	case ShaderType::DomainShader :
		core_ptr_->get_context_ptr()->DSSetShaderResources(slot, 1, texture.get_srv_ptr().GetAddressOf());
		break;
	
	case ShaderType::PixelShader :	
		core_ptr_->get_context_ptr()->PSSetShaderResources(slot, 1, texture.get_srv_ptr().GetAddressOf());
		break;

	case ShaderType::ComputeShader :
		core_ptr_->get_context_ptr()->CSSetShaderResources(slot, 1, texture.get_srv_ptr().GetAddressOf());
		break;

	default:
		return false;
		break;
	}

	return true;
}

bool LightEngine::ShaderResourceManager::bind_cs_unordered_access_buffer(Texture& texture, uint8_t slot) {

	if(!slot_valid(slot, D3D11_1_UAV_SLOT_COUNT))
		return false;

	core_ptr_->get_context_ptr()->CSSetUnorderedAccessViews(slot, 1, texture.get_uav_ptr().GetAddressOf(), nullptr);
	return true;
}

bool LightEngine::ShaderResourceManager::unbind_texture_buffer(ShaderType shader_type, uint8_t slot) {
	
	static ID3D11ShaderResourceView* const null_srv = 0;

	if(!slot_valid(slot, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT))
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

bool LightEngine::ShaderResourceManager::unbind_cs_unordered_access_buffer(uint8_t slot) {
	
	static ID3D11UnorderedAccessView* const null_uav = 0;

	if(!slot_valid(slot, D3D11_1_UAV_SLOT_COUNT))
		return false;

	core_ptr_->get_context_ptr()->CSSetUnorderedAccessViews(slot, 1, &null_uav, nullptr);
	return true;

}
