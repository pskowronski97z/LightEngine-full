#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <wrl.h>
#include <vector>
#include <LEData.h>
#include <string>
#include <memory>

namespace LightEngine {

	template <class T> class Geometry;
	class Camera;
	struct TransformMatrices;
	
	class __declspec(dllexport) Core {
	private:
		Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain_ptr_;
		Microsoft::WRL::ComPtr<ID3D11Device> device_ptr_;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_ptr_;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> render_target_ptr_;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depth_view_ptr_;					
		HRESULT call_result_;
		
	public:
		Core(HWND window_handle_, int viewport_width, int viewport_height);
		Core(const Core&) = delete;	// deleting copy constructor (optional)
		Core& operator=(const Core&) = delete; // deleting assignment operator (optional)
		void clear_back_buffer(float r, float g, float b, float a) const;
		void clear_back_buffer(float clear_color[4]) const;
		void present_frame();
		void vertex_buffer_setup(Vertex3 *vertex_buffer, int buffer_size);
		void update_frame_buffer(int new_width, int new_height);
		void viewport_setup(int x, int y, int width, int height);
		Microsoft::WRL::ComPtr<ID3D11Device> get_device_ptr();
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> get_context_ptr();
	};

	class CoreUser {
	protected:
		std::shared_ptr<Core> core_ptr_;
		HRESULT call_result_;
		CoreUser(std::shared_ptr<Core> core_ptr) : core_ptr_(core_ptr) {};		
	};

}
