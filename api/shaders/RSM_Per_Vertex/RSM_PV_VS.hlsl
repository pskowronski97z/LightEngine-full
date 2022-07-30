#include "RSM_PV_Header.hlsli"

#define SAMPLING_RADIUS 0.2
#define SAMPLE_COUNT 50
#define INDIRECT_INTENSITY 10.0
#define ATT_QUADRATIC 1.0
#define ATT_LINEAR 0.0
#define ATT_CONSTANT 1.0

struct Vertex {
    float3 position : VT3_POSITION;
    float4 color : VT3_COLOR;
    float3 uvw : VT3_UVW;
    float3 normal : VT3_NORMAL;
    float3 tangent : VT3_TANGENT;
    float3 bitangent : VT3_BITANGENT;
};

cbuffer CAMERA : register(b0) {
    matrix camera_matrix_;
    matrix projection_matrix_;
}

Texture2D W_POSITION_MAP : register(t0);
Texture2D NORMAL_MAP : register(t1);
Texture2D FLUX_MAP : register(t2);
Texture2D PATTERN : register(t3);

float3 getIndirectDiffuse(Pixel pixel, float3 pixelLightPosition, float3 pixelLightNormal, float3 pixelLightFlux) {
    
    float3 l = pixelLightPosition - pixel.world_position;
    float distance = length(l);
    
    l = normalize(l);
    
    float shading = ceil(max(0.0, dot(-l, pixelLightNormal))) *
                    (1.0 / (ATT_QUADRATIC * distance * distance + ATT_LINEAR * distance + ATT_CONSTANT)) *
                    max(0.0, dot(pixel.normal, l));

    l = pixel.color.xyz * pixelLightFlux;
    
    return l * shading;
    
}

Pixel main(Vertex inputVertex) {
	
    Pixel result;
    
    result.position = mul(float4(inputVertex.position, 1.0f), camera_matrix_);
    result.position = mul(result.position, projection_matrix_);
    result.color = inputVertex.color;
    result.normal = inputVertex.normal;
    result.world_position = inputVertex.position;
    float depth = result.position.z / result.position.w;
    result.depth = float3(depth, depth, depth);
    
    
    float4 l_pov_position = mul(float4(result.world_position, 1.0), l_pov_transform_matrix);
    l_pov_position = mul(l_pov_position, l_pov_projection_matrix);
    l_pov_position /= l_pov_position.w;
    
    uint gbuffer_w;
    uint gbuffer_h;
        
    W_POSITION_MAP.GetDimensions(gbuffer_w, gbuffer_h);
    
    uint sampling_radius_x = SAMPLING_RADIUS * gbuffer_w;
    uint sampling_radius_y = SAMPLING_RADIUS * gbuffer_h;
    
    gbuffer_w *= 0.5;
    gbuffer_h *= 0.5;
    
    uint2 tx_coords;
    
    tx_coords.x = mad(l_pov_position.x, gbuffer_w, gbuffer_w);
    tx_coords.y = mad(-l_pov_position.y, gbuffer_h, gbuffer_h);
  
    float3 indirect_diffuse;

    for (int i = 0; i < SAMPLE_COUNT; i++) {
       
        float4 sample_offset = PATTERN.Load(int3(i, 0, 0));
        int3 sample_uvw = int3(mad(sample_offset.x, sampling_radius_x, tx_coords.x),
                               mad(sample_offset.y, sampling_radius_y, tx_coords.y),
                               0);
          
        float4 pixelLightPosition = W_POSITION_MAP.Load(sample_uvw);
        float4 pixelLightNormal = NORMAL_MAP.Load(sample_uvw);
        float4 pixelLightFlux = FLUX_MAP.Load(sample_uvw);
        
        indirect_diffuse += getIndirectDiffuse(result,
                                        pixelLightPosition.xyz,
                                        pixelLightNormal.xyz,
                                        pixelLightFlux.xyz) * sample_offset.z;
    }
    
    result.indirect = INDIRECT_INTENSITY * indirect_diffuse;
    
    return result;
}