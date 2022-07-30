#include "PixelShader.hlsli"

float4 main(PS_INPUT input) : SV_TARGET {
    
    float3 lighting = float3(0.0, 0.0, 0.0); 
    float3 sample = DIFFUSE_MAP.Sample(DEFAULT_SAMPLER, float2(input.uvw[0], input.uvw[1]));
    
    //lighting = input.gouraud_shading * sample + sample*ambient;
    
    lighting = saturate(lighting);
    
    return float4(lighting, 1.0);

}