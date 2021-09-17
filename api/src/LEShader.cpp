#include <LEShader.h>
#include <d3dcompiler.h>

LightEngine::ConstantBuffer::ConstantBuffer(std::shared_ptr<Core> core_ptr) : CoreUser(core_ptr) {}

void LightEngine::ConstantBuffer::bind_ps_buffer(short slot) const {
	core_ptr_->get_context_ptr()->PSSetConstantBuffers(slot, 1, constant_buffer_ptr_.GetAddressOf());
}

void LightEngine::ConstantBuffer::bind_vs_buffer(short slot) const {
	core_ptr_->get_context_ptr()->VSSetConstantBuffers(slot, 1, constant_buffer_ptr_.GetAddressOf());
}

LightEngine::VertexShader::VertexShader(std::shared_ptr<Core> core_ptr, std::wstring compiled_shader_path) : CoreUser(core_ptr) {

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

LightEngine::PixelShader::PixelShader(std::shared_ptr<Core> core_ptr, std::wstring compiled_shader_path) : CoreUser(core_ptr) {
	
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
