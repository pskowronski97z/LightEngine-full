#pragma comment(lib,"d3d11")
#pragma comment(lib,"D3Dcompiler")
#include <iostream>
#include <LETexture.h>
#include <LEException.h>
#include <d3dcompiler.h>


void LightEngine::Core::update_depth_stencil_view(short width, short height, ID3D11DepthStencilView **dsv_pptr) {

	D3D11_TEXTURE2D_DESC depth_texture_desc = {};

	depth_texture_desc.Width = width;
	depth_texture_desc.Height = height;
	depth_texture_desc.MipLevels = 1u;
	depth_texture_desc.ArraySize = 1u;
	depth_texture_desc.Format = DXGI_FORMAT_D32_FLOAT;
	depth_texture_desc.SampleDesc.Count = 1u;
	depth_texture_desc.SampleDesc.Quality = 0u;
	depth_texture_desc.Usage = D3D11_USAGE_DEFAULT;
	depth_texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> depth_texture;

	call_result_ = device_ptr_->CreateTexture2D(&depth_texture_desc, nullptr, &depth_texture);

	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Depth texture creation failed> ", "LECore.cpp", __LINE__, call_result_);

	D3D11_DEPTH_STENCIL_VIEW_DESC view_desc = {};

	view_desc.Format = DXGI_FORMAT_D32_FLOAT;
	view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	view_desc.Texture2D.MipSlice = 0u;

	call_result_ = device_ptr_->CreateDepthStencilView(depth_texture.Get(), &view_desc, dsv_pptr);

	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Stencil view creation failed> ", "LECore.cpp", __LINE__, call_result_);

}

LightEngine::Core::Core(HWND window_handle_, int viewport_width, int viewport_height) {

	const D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_1;
	DXGI_SWAP_CHAIN_DESC swap_chain_desc;

	swap_chain_desc.BufferDesc.Height = viewport_height;
	swap_chain_desc.BufferDesc.Width = viewport_width;
	swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
	swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
	swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	swap_chain_desc.SampleDesc.Count = 1u;
	swap_chain_desc.SampleDesc.Quality = 0u;

	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
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



	// Getting back buffer from swap chain

	Microsoft::WRL::ComPtr<ID3D11Resource> back_buffer;

	call_result_ = swap_chain_ptr_->GetBuffer(0, __uuidof(ID3D11Resource), &back_buffer);

	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Cannot obtain an access to the back buffer> ", "LECore.cpp", __LINE__, call_result_);



	// Using back buffer to create render target to bind to the output-merger

	call_result_ = device_ptr_->CreateRenderTargetView(back_buffer.Get(), nullptr, frame_buffer_rtv_ptr_.GetAddressOf());

	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Render target creation failed> ", "LECore.cpp", __LINE__, call_result_);

	update_depth_stencil_view(viewport_width, viewport_height, &frame_buffer_dsv_ptr_);
	
	//shadow_map_rtv_init(shadow_map_res_width, shadow_map_res_height);
	//update_depth_stencil_view(shadow_map_res_width, shadow_map_res_height, &shadow_map_dsv_ptr_);
	

	context_ptr_->OMSetRenderTargets(1u, frame_buffer_rtv_ptr_.GetAddressOf(), frame_buffer_dsv_ptr_.Get());

	//shadow_map_rtv_init(viewport_width, viewport_height);

	// Depth state (Settings of depth testing)

	/*D3D11_DEPTH_STENCIL_DESC dsDesc;

	{
		// Depth test parameters
		dsDesc.DepthEnable = true;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

		// Stencil test parameters
		dsDesc.StencilEnable = true;
		dsDesc.StencilReadMask = 0xFF;
		dsDesc.StencilWriteMask = 0xFF;

		// Stencil operations if pixel is front-facing
		dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Stencil operations if pixel is back-facing
		dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	}

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depth_stencil_state;

	call_result_ = device_ptr_->CreateDepthStencilState(&dsDesc, &depth_stencil_state);

	if (FAILED(call_result_))
		throw LECoreException("<D3D11 ERROR> <Stencil state creation failed> ", "LECore.cpp",__LINE__, call_result_);

	context_ptr_->OMSetDepthStencilState(depth_stencil_state.Get(),1u);*/

	//context_ptr_->OMSetRenderTargets(1u,back_buffer_rtv_ptr_.GetAddressOf(),depth_view_ptr_.Get());

	// Viewport set up

	viewport_setup(0, 0, viewport_width, viewport_height);

}

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

void LightEngine::Core::update_frame_buffer(int new_width, int new_height) {

	if (rendering_to_frame_buffer_) {
		context_ptr_->OMSetRenderTargets(0, 0, 0);

		frame_buffer_rtv_ptr_->Release();

		call_result_ = swap_chain_ptr_->ResizeBuffers(0, new_width, new_height, DXGI_FORMAT_UNKNOWN, 0);

		if (FAILED(call_result_))
			throw LECoreException("<D3D11 ERROR> <Buffers resizing failed>", "LECore.cpp", __LINE__, call_result_);

		Microsoft::WRL::ComPtr<ID3D11Resource> buffer;

		call_result_ = swap_chain_ptr_->GetBuffer(0, __uuidof(ID3D11Resource), &buffer);
		if (FAILED(call_result_))
			throw LECoreException("<D3D11 ERROR> <Cannot obtain an access to the back buffer>", "LECore.cpp", __LINE__, call_result_);

		call_result_ = device_ptr_->CreateRenderTargetView(buffer.Get(), NULL, &frame_buffer_rtv_ptr_);
		if (FAILED(call_result_))
			throw LECoreException("<D3D11 ERROR> <Render target creation failed>", "LECore.cpp", __LINE__, call_result_);


		update_depth_stencil_view(new_width, new_height, &frame_buffer_dsv_ptr_);

		context_ptr_->OMSetRenderTargets(1u, frame_buffer_rtv_ptr_.GetAddressOf(), frame_buffer_dsv_ptr_.Get());





	}
	else
		std::cout<<"<LOG> <update_frame_buffer(...)> Cannot update frame buffer in rendering to texture mode"<<std::endl;

	/*
	static ID3D11ShaderResourceView *const null_srv = {0};

	if(rendering_to_frame_buffer_){
		context_ptr_->OMSetRenderTargets(1u, frame_buffer_rtv_ptr_.GetAddressOf(), frame_buffer_dsv_ptr_.Get());
		context_ptr_->PSSetShaderResources(5, 1, shadow_map_srv_ptr_.GetAddressOf());
	}
	else {
		context_ptr_->PSSetShaderResources(5, 1, &null_srv);
		shadow_map_rtv_init(new_width, new_height);
		context_ptr_->OMSetRenderTargets(1u, shadow_map_rtv_ptr_.GetAddressOf(), shadow_map_dsv_ptr_.Get());
	*/
}

void LightEngine::Core::render_to_frame_buffer() {
	rendering_to_frame_buffer_ = true;
	context_ptr_->OMSetRenderTargets(1u, frame_buffer_rtv_ptr_.GetAddressOf(), frame_buffer_dsv_ptr_.Get());
	context_ptr_->RSSetViewports(1u, &viewport_);
}

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

void LightEngine::Core::clear_frame_buffer(float r, float g, float b, float a) const {
	static float color[] = { r, g, b, a };
	context_ptr_->ClearRenderTargetView(frame_buffer_rtv_ptr_.Get(), color);
	context_ptr_->ClearDepthStencilView(frame_buffer_dsv_ptr_.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
}

Microsoft::WRL::ComPtr<ID3D11Device> LightEngine::Core::get_device_ptr() { return device_ptr_; }

Microsoft::WRL::ComPtr<ID3D11DeviceContext> LightEngine::Core::get_context_ptr() { return context_ptr_; }



