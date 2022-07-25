#pragma once
#include <memory>
#include <wrl.h>
#include <LEData.h>
#include <LECore.h>
//#include <LETexture.h>
#include <LEException.h>

namespace LightEngine {

	// DEBUG LIGHT STRUCTURE
	struct Light {
		DirectX::XMFLOAT4A position_;
		DirectX::XMFLOAT4A color_;
	};

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
		void run(uint16_t x_groups_count, uint16_t y_groups_count, uint16_t z_groups_count) const;
	};

	

	class ShaderResource : public CoreUser {
	protected:
		std::string shader_resource_name_;
		ShaderType bound_to_shader_;
		uint8_t bound_to_slot_;
		ShaderResource();
	public:
		std::string get_shader_resource_name() const;
		ShaderType get_bound_shader_type() const;
		uint8_t get_bound_slot_number() const;
	};

	template <class T>
	class CBuffer : public ShaderResource {
	friend class ShaderResourceManager;
		Microsoft::WRL::ComPtr<ID3D11Buffer> buffer_ptr_;
	public:
		CBuffer(std::shared_ptr<Core> core_ptr, const std::string &name, const T* const data, const uint32_t size_in_bytes);
		void update(const T* const data, const uint32_t size_in_bytes);
	};

	template class _declspec(dllexport) CBuffer<Light>;
	template class _declspec(dllexport) CBuffer<uint32_t>;

	class _declspec(dllexport) AbstractTexture : public ShaderResource {
		friend class ShaderResourceManager;
		friend class Core;
	protected:
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_ptr_;
		Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> uav_ptr_;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv_ptr_;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> dsv_ptr_;
		AbstractTexture();
	public:
		void generate_mip_maps() const;
	};

	class __declspec(dllexport) Texture2D : public AbstractTexture {
	protected:
		D3D11_TEXTURE2D_DESC descriptor_;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> ptr_;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> depth_texture_ptr_;	
		uint16_t width_;
		uint16_t height_;
	public:
		Texture2D(const std::shared_ptr<Core> &core_ptr, const std::string &texture_path);
		Texture2D(const std::shared_ptr<Core> &core_ptr, const std::string &name, const uint16_t width, const uint16_t height, const float *data);
		Texture2D(const std::shared_ptr<Core>& core_ptr, const std::string& name, const uint16_t width, const uint16_t height, const uint32_t *data);
		// This method packs data from vector of Vertex3 to 2d texture.
		// One texture can contain up to 16 384 vertices.
		// Vertex component storing layout in texture is as follows:
		// ROW 0 - Position
		// ROW 1 - Color
		// ROW 2 - Texture coordinates
		// ROW 3 - Normal vector
		// ROW 4 - Tangent vector
		// ROW 5 - Bitangent vector
		// Each component is a vector of 4 floats
		static Texture2D store_geometry(const std::shared_ptr<Core>& core_ptr, const std::vector<Vertex3>& vertices);
		uint16_t get_width() const;
		uint16_t get_height() const;
	};

	class __declspec(dllexport) Texture3D : public AbstractTexture {
	protected:
		D3D11_TEXTURE3D_DESC descriptor_;
		Microsoft::WRL::ComPtr<ID3D11Texture3D> ptr_;
		uint16_t width_;
		uint16_t height_;
		uint16_t depth_;
	public:
		Texture3D(const std::shared_ptr<Core> &core_ptr, const std::string &name, const uint16_t width, const uint16_t height, const uint16_t depth, const float *data);
		Texture3D(const std::shared_ptr<Core>& core_ptr, const std::string& name, const uint16_t width, const uint16_t height, const uint16_t depth, const int32_t* data);

		uint16_t get_width() const;
		uint16_t get_height() const;
		uint16_t get_depth() const;
	};

	class __declspec(dllexport) ShaderResourceManager : private CoreUser {
	public:
		ShaderResourceManager(const std::shared_ptr<Core> &core_ptr);
		bool bind_texture_buffer(AbstractTexture &texture, const ShaderType shader_type, const uint8_t slot) const;
		bool bind_cs_unordered_access_buffer(AbstractTexture &texture, const uint8_t slot) const;
		template<class T>
		bool bind_constant_buffer(CBuffer<T> &cbuffer, const ShaderType shader_type, const uint8_t slot) const;
		//void bind_sampler_buffer(uint8_t slot);
		bool unbind_texture_buffer(const ShaderType shader_type, const uint8_t slot) const;
		bool unbind_cs_unordered_access_buffer(const uint8_t slot) const;
		bool unbind_constant_buffer(const ShaderType shader_type, const uint8_t slot) const;
		//void unbind_sampler_buffer(uint8_t slot);
	};
	
	template _declspec(dllexport) bool ShaderResourceManager::bind_constant_buffer(CBuffer<Light> &cbuffer, const ShaderType shader_type, const uint8_t slot) const;
	template _declspec(dllexport) bool ShaderResourceManager::bind_constant_buffer(CBuffer<uint32_t>& cbuffer, const ShaderType shader_type, const uint8_t slot) const;


}