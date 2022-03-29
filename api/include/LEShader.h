#pragma once
#include <memory>
#include <wrl.h>
#include <d3d11.h>
#include <LETexture.h>
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

	enum class ShaderType {
		Null,
		VertexShader,
		GeometryShader,
		DomainShader,
		HullShader,
		PixelShader,
		ComputeShader
	};

	template<class T> 
	class Shader : protected CoreUser {
	protected:
		Microsoft::WRL::ComPtr<T> shader_ptr_;
	public:
		Shader(std::shared_ptr<Core> core_ptr);
		virtual void bind() const = 0;
	};

	class __declspec(dllexport) VertexShader : public Shader<ID3D11VertexShader> {
	private:
		Microsoft::WRL::ComPtr<ID3D11InputLayout> layout_ptr_;
	public:
		/// <summary>
		/// Constructor (currently supports Vertex3 structure only)
		/// </summary>
		/// <param name="core_ptr"></param>
		/// <param name="compiled_shader_path"></param>
		VertexShader(std::shared_ptr<Core> core_ptr, std::wstring compiled_shader_path);
		void bind() const override;
	};

	class __declspec(dllexport) PixelShader : public Shader<ID3D11PixelShader> {
	public:
		PixelShader(std::shared_ptr<Core> core_ptr, std::wstring compiled_shader_path);
		void bind() const override;
	
	};

	class __declspec(dllexport) ComputeShader : public Shader<ID3D11ComputeShader> {
	public:
		ComputeShader(std::shared_ptr<Core> core_ptr, std::wstring compiled_shader_path);
		void bind() const override;
		void run() const;
	};

	class __declspec(dllexport) ShaderResourceManager : private CoreUser {
	private:
		bool slot_valid(uint8_t slot, uint8_t max_slots);
	public:
		ShaderResourceManager(std::shared_ptr<Core> core_ptr);
		bool bind_texture_buffer(Texture &texture, ShaderType shader_type, uint8_t slot);
		bool bind_cs_unordered_access_buffer(Texture &texture, uint8_t slot);
		void bind_constant_buffer(uint8_t slot);
		void bind_sampler_buffer(uint8_t slot);
		bool unbind_texture_buffer(ShaderType shader_type, uint8_t slot);
		bool unbind_cs_unordered_access_buffer(uint8_t slot);
		void unbind_constant_buffer(uint8_t slot);
		void unbind_sampler_buffer(uint8_t slot);
	};
}