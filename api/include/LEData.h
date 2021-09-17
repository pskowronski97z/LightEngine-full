#pragma once
#include <d3d11.h>
#include <DirectXMath.h>

constexpr auto LE_VERTEX3_COMPONENTS_COUNT = 6;

namespace LightEngine {

	struct Vertex3 {
		DirectX::XMFLOAT3 position_;
		DirectX::XMFLOAT4 color_;		
		DirectX::XMFLOAT3 texcoords_;
		DirectX::XMFLOAT3 normal_;	
		DirectX::XMFLOAT3 tangent_;	
		DirectX::XMFLOAT3 bitangent_;	
		static const D3D11_INPUT_ELEMENT_DESC vertex_desc_[LE_VERTEX3_COMPONENTS_COUNT];
	};


	struct ViewBuffer {
		float view_matrix[4][4];
		
	};
	
}
