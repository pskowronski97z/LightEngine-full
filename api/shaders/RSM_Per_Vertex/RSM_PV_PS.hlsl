#include "RSM_PV_Header.hlsli"

Texture2D<float4> DEPTH_MAP : register(t0);

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
    float3 direct_diffuse = inputPixel.color * light_color;
    
    direct_diffuse *= shading;
 
    if (l_pov_position.z > (sampled_depth + 0.001))
        direct_diffuse *= 0.0;

    return float4(direct_diffuse + inputPixel.indirect, 1.0);
    
}