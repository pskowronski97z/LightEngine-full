#include "MC_Properties.hlsli"

struct Pixel {
    float4 position : SV_POSITION;
    float4 color : VT3_COLOR;
    float3 uvw : VT3_UVW;
    float3 normal : VT3_NORMAL;
    float3 tangent : VT3_TANGENT;
    float3 bitangent : VT3_BITANGENT;
    float3 world_position : VT3_POSITION;
    float3 depth : VT3_COLOR1;
};

cbuffer LIGHT_POV_CAMERA : register(b0) {
    matrix l_pov_transform_matrix;
    matrix l_pov_projection_matrix;
    float4 l_pov_world_position;
}

cbuffer LIGHT_SOURCE : register(b1) {
    float4 light_direction;
    float4 light_color;
    float4 light_intensity;
}

Texture3D GI_MAP : register(t0);
Texture2D DEPTH_MAP : register(t1);

SamplerState SAMPLER : register(s0);

float4 main(Pixel inputPixel) : SV_TARGET {
    
    float4 l_pov_position = mul(float4(inputPixel.world_position, 1.0), l_pov_transform_matrix);
    
    l_pov_position = mul(l_pov_position, l_pov_projection_matrix);
    l_pov_position /= l_pov_position.w;
    
    float2 uv;
    
    uv.x = mad(l_pov_position.x, 0.5, 0.5);
    uv.y = mad(-l_pov_position.y, 0.5, 0.5);
   
    float sampled_depth = DEPTH_MAP.Sample(SAMPLER, uv); 
    float3 n = normalize(inputPixel.normal.xyz);
    float3 l = normalize(l_pov_world_position.xyz);   
    float shading = max(0.0, dot(n, l)) * light_intensity.x;
    float4 direct_diffuse = inputPixel.color * light_color;
    
    direct_diffuse *= shading;
 
    if (l_pov_position.z > (sampled_depth + 0.001))
        direct_diffuse *= 0.0;

    float4 indirect_diffuse = float4(0.0, 0.0, 0.0, 0.0);
    
    for (int i = 0; i < SAMPLES_COUNT; i++) 
        indirect_diffuse += GI_MAP[uint3(inputPixel.position.xy, i)];
    
    indirect_diffuse /= SAMPLES_COUNT;
   
    return direct_diffuse + indirect_diffuse;
}