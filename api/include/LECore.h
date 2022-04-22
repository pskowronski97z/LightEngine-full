#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <wrl.h>
#include <vector>
#include <LEData.h>
#include <string>
#include <memory>

namespace LightEngine {

	class RenderableTexture;
	template <class T> class Geometry;
	class Camera;
	class AbstractTexture;
	struct TransformMatrices;
	
	class __declspec(dllexport) Core {
	private:
		Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain_ptr_;
		Microsoft::WRL::ComPtr<ID3D11Device> device_ptr_;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_ptr_;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> frame_buffer_rtv_ptr_;
		ID3D11RenderTargetView *render_targets[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> frame_buffer_dsv_ptr_;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> default_dsv_ptr_;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> frame_buffer_ptr_;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> depth_texture_ms_ptr_;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> depth_texture_ptr_;
		uint8_t msaa_samples_count_;
		D3D11_VIEWPORT viewport_;
		HRESULT call_result_;
		bool rendering_to_frame_buffer_ = true;
	public:
		Core(HWND window_handle_, int viewport_width, int viewport_height);
		Core(const Core&) = delete;	// deleting copy constructor (optional)
		Core& operator=(const Core&) = delete; // deleting assignment operator (optional)
		void add_texture_to_render(AbstractTexture &texture, uint8_t slot);
		void clear_textures_to_render();
		void release_render_targets();
		void clear_frame_buffer(float r, float g, float b, float a) const;
		void flush_render_targets() const;
		void clear_frame_buffer(float clear_color[4]) const;
		void present_frame();
		void vertex_buffer_setup(Vertex3 *vertex_buffer, int buffer_size);
		void viewport_setup(int x, int y, int width, int height);
		void render_to_frame_buffer();
		void render_to_textures();
		// All resources bound to frame buffer render target must be released before setting new 
		//void render_to_texture(RenderableTexture &renderable_texture);
		Microsoft::WRL::ComPtr<ID3D11Device> get_device_ptr();
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> get_context_ptr();
		// Render targets should be bound again after calling this!
		void setup_frame_buffer(const uint16_t width, const uint16_t height, const bool init);
		// CAUTION! - This method unbinds all render targets! 
		/*void cs_bind_frame_buffer(const uint16_t slot);
		void cs_unbind_frame_buffer(const uint16_t slot);*/
	};

	class CoreUser {
	protected:
		std::shared_ptr<Core> core_ptr_;
		HRESULT call_result_;
		CoreUser(const std::shared_ptr<Core> &core_ptr) : core_ptr_(core_ptr) {};		
		CoreUser();
	};

}
