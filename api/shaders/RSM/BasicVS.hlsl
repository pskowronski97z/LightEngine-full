cbuffer CAMERA : register(b0) {
    matrix camera_matrix_;
    matrix projection_matrix_;
}

struct Vertex {
    float3 position : VT3_POSITION;
    float4 color : VT3_COLOR;
    float3 uvw : VT3_UVW;
    float3 normal : VT3_NORMAL;
    float3 tangent : VT3_TANGENT;
    float3 bitangent : VT3_BITANGENT;
};

struct Pixel {
    float4 position : SV_POSITION; 
    float4 color : VT3_COLOR;
    float3 uvw : VT3_UVW;
    float3 normal : VT3_NORMAL;
    float3 tangent : VT3_TANGENT;
    float3 bitangent : VT3_BITANGENT;
    float3 world_position : VT3_POSITION;
    float3 depth: VT3_COLOR1;
};

Pixel main(Vertex inputVertex) {
	
    Pixel result;
    
    result.position = mul(float4(inputVertex.position, 1.0f), camera_matrix_);
    result.position = mul(result.position, projection_matrix_);  
    result.color = inputVertex.color;
    result.uvw = inputVertex.uvw;
    result.normal = inputVertex.normal;
    result.tangent = inputVertex.tangent;
    result.bitangent = inputVertex.bitangent;
    result.world_position = inputVertex.position;
    float depth = result.position.z / result.position.w;
    result.depth = float3(depth, depth, depth);
    
    return result;
}