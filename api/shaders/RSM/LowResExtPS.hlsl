#include "RSMInclude.hlsli"
#define RANDOM_ROTATION

Texture2D W_POSITION_MAP : register(t0);
Texture2D NORMAL_MAP : register(t1);
Texture2D FLUX_MAP : register(t2);
Texture2D PATTERN : register(t3);
Texture2D NOISE_MAP : register(t4);

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

float4 main(Pixel inputPixel) : SV_TARGET {
    
    inputPixel.normal = normalize(inputPixel.normal);
    
    float4 l_pov_position = mul(float4(inputPixel.world_position, 1.0), l_pov_transform_matrix);
    l_pov_position = mul(l_pov_position, l_pov_projection_matrix);
    l_pov_position /= l_pov_position.w;
    
    float2 uv;
    uv.x = mad(l_pov_position.x, 0.5, 0.5);
    uv.y = mad(-l_pov_position.y, 0.5, 0.5);
  
    float3 indirect_diffuse;

    #ifdef RANDOM_ROTATION
     
    float4 random_float4 = NOISE_MAP.Load(uint3(inputPixel.position.xy, 0.0));
    float random_angle = 2.0 * random_float4.x;
    float2x2 rotation_2D = float2x2(cos(random_angle), sin(random_angle),
                                   -sin(random_angle), cos(random_angle)); 
    #endif
    
    for (int i = 0; i < SAMPLE_COUNT; i++) {
       
        float4 sample_offset = PATTERN.Load(int3(i, 0, 0));   
        
        #ifdef RANDOM_ROTATION        
        
        sample_offset.xy = mul(sample_offset.xy, rotation_2D);
        
        #endif
        
        float2 sample_uv = float2(mad(sample_offset.x, SAMPLING_UV_RADIUS, uv.x),
                                  mad(sample_offset.y, SAMPLING_UV_RADIUS, uv.y));     
        float4 pixelLightPosition = W_POSITION_MAP.Sample(SAMPLER, sample_uv);
        float4 pixelLightNormal = NORMAL_MAP.Sample(SAMPLER, sample_uv);
        float4 pixelLightFlux = FLUX_MAP.Sample(SAMPLER, sample_uv);
        
        indirect_diffuse += getIndirectDiffuse(inputPixel,
                                        pixelLightPosition.xyz,
                                        pixelLightNormal.xyz,
                                        pixelLightFlux.xyz) * sample_offset.z;
    }
    
   
    return float4(indirect_diffuse * INDIRECT_LIGHT_INTENSITY, 1.0);
    
}