#pragma once
#include <memory>
#include <wrl.h>
#include <d3d11.h>
#include <LECore.h>
#include <LEException.h>

namespace LightEngine {

	class __declspec(dllexport) ConstantBuffer : protected CoreUser {
	protected:
		Microsoft::WRL::ComPtr<ID3D11Buffer> constant_buffer_ptr_;
		template<class T>
		void update_constant_buffer(std::shared_ptr<T> buffer_content) {

			D3D11_MAPPED_SUBRESOURCE new_constant_buffer;
			ZeroMemory(&new_constant_buffer, sizeof(new_constant_buffer));

			call_result_ = core_ptr_->get_context_ptr()->Map(constant_buffer_ptr_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &new_constant_buffer);

			if (FAILED(call_result_))
				throw LECoreException("<Shader constant buffer mapping failed> ", "LEShader.cpp", __LINE__, call_result_);

			memcpy(new_constant_buffer.pData, buffer_content.get(), sizeof(T));

			core_ptr_->get_context_ptr()->Unmap(constant_buffer_ptr_.Get(), 0);
		};
		template<class T>
		void create_constant_buffer(std::shared_ptr<T> buffer_content) {

			D3D11_BUFFER_DESC buffer_desc;
			D3D11_SUBRESOURCE_DATA sr_data;

			buffer_desc.ByteWidth = sizeof(T);
			buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
			buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			buffer_desc.MiscFlags = 0;
			buffer_desc.StructureByteStride = sizeof(float);

			sr_data.pSysMem = buffer_content.get();

			call_result_ = core_ptr_->get_device_ptr()->CreateBuffer(&buffer_desc, &sr_data, &constant_buffer_ptr_);
			// TODO - Fix exceptions for deriving classes
			if (FAILED(call_result_))
				throw LECoreException("<D3D11 ERROR> <Shader constant buffer creation failed> ", "LEShader.cpp", __LINE__, call_result_);

		};
		ConstantBuffer(std::shared_ptr<Core> core_ptr);
	public:
		void bind_ps_buffer(short slot) const;
		void bind_vs_buffer(short slot) const;
	};

	class __declspec(dllexport) VertexShader : private CoreUser {
	private:
		Microsoft::WRL::ComPtr<ID3D11VertexShader> shader_ptr_;	
		Microsoft::WRL::ComPtr<ID3D11InputLayout> layout_ptr_;
	public:
		/// <summary>
		/// Constructor (currently supports Vertex3 structure only)
		/// </summary>
		/// <param name="core_ptr"></param>
		/// <param name="compiled_shader_path"></param>
		VertexShader(std::shared_ptr<Core> core_ptr, std::wstring compiled_shader_path);
		void bind() const;
	};

	class __declspec(dllexport) PixelShader : private CoreUser {
	private:
		Microsoft::WRL::ComPtr<ID3D11PixelShader> shader_ptr_;
	public:
		PixelShader(std::shared_ptr<Core> core_ptr, std::wstring compiled_shader_path);
		void bind() const;
	
	};
}