#pragma comment(lib,"d3d11")
#pragma comment(lib,"D3Dcompiler")
#include <iostream>
//#include <LETexture.h>
//#include <LEException.h>
#include <LEShader.h>
#include <d3dcompiler.h>

LightEngine::Core::Core(HWND window_handle_, int viewport_width, int viewport_height) {

	msaa_samples_count_ = 4u;
	static const D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_0;
	static DXGI_SWAP_CHAIN_DESC swap_chain_desc = { 0 };

	swap_chain_desc.BufferDesc.Height = viewport_height;
	swap_chain_desc.BufferDesc.Width = viewport_width;
	swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
	swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
	swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	swap_chain_desc.SampleDesc.Count = msaa_samples_count_;
	swap_chain_desc.SampleDesc.Quality = 0u;

	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
	swap_chain_desc.BufferCount = 1;
	swap_chain_desc.OutputWindow = window_handle_;
	swap_chain_desc.Windowed = true;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swap_chain_desc.Flags = 0;

	// Creating swap chain

	call_result_ = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		D3D11_CREATE_DEVICE_DEBUG,
		&feature_level,
		1,
		D3D11_SDK_VERSION,
		&swap_chain_desc,
		swap_chain_ptr_.GetAddressOf(),
		device_ptr_.GetAddressOf(),
		nullptr,
		context_ptr_.GetAddressOf());

	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Device and swap chain creation failed> ", "LECore.cpp", __LINE__, call_result_);

	clear_textures_to_render();
	setup_frame_buffer(viewport_width, viewport_height, true);
	render_to_frame_buffer();

	viewport_setup(0, 0, viewport_width, viewport_height);
}


void LightEngine::Core::setup_frame_buffer(const uint16_t width, const uint16_t height, const bool init) {

	if(swap_chain_ptr_ == nullptr)
		throw LECoreException("<D3D11 ERROR> <Cannot setup a frame buffer. Swap chain is not initialized.> ", "LECore.cpp", __LINE__, call_result_);

	if(!rendering_to_frame_buffer_)
		throw LECoreException("<D3D11 ERROR> <Cannot update frame buffer in rendering to texture mode.> ", "LECore.cpp", __LINE__, call_result_);

	if (!init) {

		static ID3D11UnorderedAccessView *const null_uav_ptr = {0};
		release_render_targets();
		context_ptr_->CSSetUnorderedAccessViews(0u, 1u, &null_uav_ptr, nullptr);
		frame_buffer_dsv_ptr_->Release();
		frame_buffer_rtv_ptr_->Release();
		frame_buffer_ptr_->Release();
		depth_texture_ms_ptr_->Release();
		//depth_texture_ptr_->Release();
		//frame_buffer_uav_ptr_->Release();

		call_result_ = swap_chain_ptr_->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
	}

	// Retrieve back buffer from swap chain

	call_result_ = swap_chain_ptr_->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(frame_buffer_ptr_.GetAddressOf()));

	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Cannot obtain an access to the back buffer> ", "LECore.cpp", __LINE__, call_result_);


	// Creating render target view with previously extracted texture

	call_result_ = device_ptr_->CreateRenderTargetView(frame_buffer_ptr_.Get(), nullptr, frame_buffer_rtv_ptr_.GetAddressOf());

	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Render target creation failed> ", "LECore.cpp", __LINE__, call_result_);

	// Creating multisampled depth texture

	static D3D11_TEXTURE2D_DESC depth_texture_desc = { 0 };

	depth_texture_desc.Width = width;
	depth_texture_desc.Height = height;
	depth_texture_desc.MipLevels = 1u;
	depth_texture_desc.ArraySize = 1u;
	depth_texture_desc.Format = DXGI_FORMAT_D32_FLOAT;
	depth_texture_desc.SampleDesc.Count = msaa_samples_count_;
	depth_texture_desc.SampleDesc.Quality = 0u;
	depth_texture_desc.Usage = D3D11_USAGE_DEFAULT;
	depth_texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	call_result_ = device_ptr_->CreateTexture2D(&depth_texture_desc, nullptr, depth_texture_ms_ptr_.GetAddressOf());

	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Depth texture creation failed> ", "LECore.cpp", __LINE__, call_result_);

	static D3D11_DEPTH_STENCIL_VIEW_DESC view_desc = {};

	view_desc.Format = DXGI_FORMAT_D32_FLOAT;
	view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	view_desc.Texture2D.MipSlice = 0u;

	// Creating depth stencil view using previously created texture

	call_result_ = device_ptr_->CreateDepthStencilView(depth_texture_ms_ptr_.Get(), &view_desc, frame_buffer_dsv_ptr_.GetAddressOf());

	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Stencil view creation failed> ", "LECore.cpp", __LINE__, call_result_);


	//Creating default depth texture
	/*
	depth_texture_desc.SampleDesc.Count = 1u;

	call_result_ = device_ptr_->CreateTexture2D(&depth_texture_desc, nullptr, depth_texture_ptr_.GetAddressOf());

	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Depth texture creation failed> ", "LECore.cpp", __LINE__, call_result_);

	view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	// Creating depth stencil view using previously created texture

	call_result_ = device_ptr_->CreateDepthStencilView(depth_texture_ptr_.Get(), &view_desc, default_dsv_ptr_.GetAddressOf());

	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Stencil view creation failed> ", "LECore.cpp", __LINE__, call_result_);

	*/
	// Creating unordered access view

	/*static D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};

	uav_desc.Texture2D.MipSlice = 0;
	uav_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;

	call_result_ = device_ptr_->CreateUnorderedAccessView(back_buffer_ptr_.Get(), &uav_desc, frame_buffer_uav_ptr_.GetAddressOf());

	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Unordered access view creation failed> ", "LECore.cpp", __LINE__, call_result_);*/

	// Binding required "frame buffer connected" views to output merger 

	//context_ptr_->OMSetRenderTargets(1u, frame_buffer_rtv_ptr_.GetAddressOf(), frame_buffer_dsv_ptr_.Get());

}

//void LightEngine::Core::cs_bind_frame_buffer(const uint16_t slot) {
//	release_render_targets();
//	//context_ptr_->CSSetUnorderedAccessViews(slot, 1u, frame_buffer_uav_ptr_.GetAddressOf(), nullptr);
//}
//
//void LightEngine::Core::cs_unbind_frame_buffer(const uint16_t slot) {
//	static ID3D11UnorderedAccessView *const null_uav_ptr = {0};
//	context_ptr_->CSSetUnorderedAccessViews(slot, 1u, &null_uav_ptr, nullptr);
//}

void LightEngine::Core::present_frame() {
	call_result_ = swap_chain_ptr_->Present(1u, 0u);

	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Frame rendering failed> ", "LECore.cpp", __LINE__, call_result_);
}

void LightEngine::Core::vertex_buffer_setup(Vertex3* vertex_buffer, int buffer_size) {

	D3D11_BUFFER_DESC buffer_desc;
	D3D11_SUBRESOURCE_DATA sr_data;
	Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;

	buffer_desc.ByteWidth = sizeof(Vertex3) * buffer_size;
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = sizeof(Vertex3);

	sr_data.pSysMem = vertex_buffer;

	call_result_ = device_ptr_->CreateBuffer(&buffer_desc, &sr_data, &buffer);

	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Vertex buffer creation failed> ", "LECore.cpp", __LINE__, call_result_);

	UINT stride = sizeof(Vertex3);
	UINT offset = 0;

	context_ptr_->IASetVertexBuffers(0, 1, buffer.GetAddressOf(), &stride, &offset);
}

void LightEngine::Core::render_to_frame_buffer() {
	rendering_to_frame_buffer_ = true;
	context_ptr_->OMSetRenderTargets(1u, frame_buffer_rtv_ptr_.GetAddressOf(), frame_buffer_dsv_ptr_.Get());
	//context_ptr_->RSSetViewports(1u, &viewport_);
}
/*
void LightEngine::Core::render_to_texture(RenderableTexture& renderable_texture) {
	rendering_to_frame_buffer_ = false;

	static D3D11_VIEWPORT texture_rendering_viewport = {0};
	texture_rendering_viewport.Width = renderable_texture.get_width();
	texture_rendering_viewport.Height = renderable_texture.get_height();
	texture_rendering_viewport.MinDepth = 0.0f;
	texture_rendering_viewport.MaxDepth = 1.0f;
	texture_rendering_viewport.TopLeftX = 0;
	texture_rendering_viewport.TopLeftY = 0;

	context_ptr_->RSSetViewports(1, &texture_rendering_viewport);
	context_ptr_->OMSetRenderTargets(1u, renderable_texture.get_rtv_ptr().GetAddressOf(), renderable_texture.get_dsv_ptr().Get());
}
*/
void LightEngine::Core::viewport_setup(int x, int y, int width, int height) {
	
	viewport_.Width = width;
	viewport_.Height = height;
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;
	viewport_.TopLeftX = x;
	viewport_.TopLeftY = y;

	context_ptr_->RSSetViewports(1, &viewport_);
}

void LightEngine::Core::clear_frame_buffer(float clear_color[4]) const {
	context_ptr_->ClearRenderTargetView(frame_buffer_rtv_ptr_.Get(), clear_color);
	context_ptr_->ClearDepthStencilView(frame_buffer_dsv_ptr_.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
}

void LightEngine::Core::add_texture_to_render(Texture2D &texture, uint8_t slot) {
	render_targets[slot] = texture.rtv_ptr_.Get();

	static D3D11_TEXTURE2D_DESC depth_texture_desc = { 0 };

	depth_texture_desc.Width = texture.get_width();
	depth_texture_desc.Height = texture.get_height();
	depth_texture_desc.MipLevels = 1u;
	depth_texture_desc.ArraySize = 1u;
	depth_texture_desc.Format = DXGI_FORMAT_D32_FLOAT;
	depth_texture_desc.SampleDesc.Count = 1u;
	depth_texture_desc.SampleDesc.Quality = 0u;
	depth_texture_desc.Usage = D3D11_USAGE_DEFAULT;
	depth_texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	call_result_ = device_ptr_->CreateTexture2D(&depth_texture_desc, nullptr, depth_texture_ptr_.ReleaseAndGetAddressOf());

	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Depth texture creation failed> ", "LECore.cpp", __LINE__, call_result_);

	static D3D11_DEPTH_STENCIL_VIEW_DESC view_desc = {};

	view_desc.Format = DXGI_FORMAT_D32_FLOAT;
	view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	view_desc.Texture2D.MipSlice = 0u;

	// Creating depth stencil view using previously created texture

	call_result_ = device_ptr_->CreateDepthStencilView(depth_texture_ptr_.Get(), &view_desc, texture_dsv_ptr_.GetAddressOf());

	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Stencil view creation failed> ", "LECore.cpp", __LINE__, call_result_);

}

void LightEngine::Core::clear_textures_to_render() {
	static ID3D11RenderTargetView* const null_rtv = { 0 };

	for (int i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		render_targets[i] = null_rtv;
}

void LightEngine::Core::release_render_targets() {
	context_ptr_->OMSetRenderTargets(0u, nullptr, nullptr);
}

void LightEngine::Core::render_to_textures() {
	rendering_to_frame_buffer_ = false;
	context_ptr_->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, render_targets, texture_dsv_ptr_.Get());
}

void LightEngine::Core::clear_frame_buffer(float r, float g, float b, float a) const {
	static float color[] = { r, g, b, a };
	context_ptr_->ClearRenderTargetView(frame_buffer_rtv_ptr_.Get(), color);
	context_ptr_->ClearDepthStencilView(frame_buffer_dsv_ptr_.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
	
}

void LightEngine::Core::flush_render_targets() const {

	static const float clear_values[4] = { 0 };

	for (int i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		if(render_targets[i] != nullptr)
			context_ptr_->ClearRenderTargetView(render_targets[i], clear_values);

	context_ptr_->ClearDepthStencilView(texture_dsv_ptr_.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
}

Microsoft::WRL::ComPtr<ID3D11Device> LightEngine::Core::get_device_ptr() { return device_ptr_; }

Microsoft::WRL::ComPtr<ID3D11DeviceContext> LightEngine::Core::get_context_ptr() { return context_ptr_; }

LightEngine::CoreUser::CoreUser() {
	core_ptr_ = nullptr;
	call_result_ = 0;
}
