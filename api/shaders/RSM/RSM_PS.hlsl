#define SAMPLING_UV_RADIUS 0.1
#define SAMPLE_COUNT 25 
#define ATT_QUADRATIC 0.4
#define ATT_LINEAR 0.0
#define ATT_CONSTANT 1.0

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

cbuffer CAMERA : register(b0) {
    matrix transform_matrix_;
    matrix projection_matrix_;
    float4 world_position_;
}

cbuffer LIGHT_POV_CAMERA : register(b1) {
    matrix l_pov_transform_matrix_;
    matrix l_pov_projection_matrix_;
    float4 l_pov_world_position_;
}

cbuffer LIGHT_SOURCE : register(b2) {
    float4 light_direction;
    float4 light_color;
    float4 light_intensity;
}

Texture2D W_POSITION_MAP : register(t0);
Texture2D NORMAL_MAP : register(t1);
Texture2D DEPTH_MAP : register(t2);
Texture2D FLUX_MAP : register(t3);
Texture2D PATTERN : register(t4);

SamplerState SAMPLER : register(s0);


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
    
    float4 l_pov_position = mul(float4(inputPixel.world_position, 1.0), l_pov_transform_matrix_);
    l_pov_position = mul(l_pov_position, l_pov_projection_matrix_); 
    l_pov_position /= l_pov_position.w;
    
  
    float2 uv;
    uv.x = l_pov_position.x * 0.5 + 0.5;
    uv.y = -l_pov_position.y * 0.5 + 0.5;
   
    float sampled_depth = DEPTH_MAP.Sample(SAMPLER, uv);
    
    float3 n = normalize(inputPixel.normal.xyz);
    float3 l = normalize(l_pov_world_position_.xyz);
    
    float shading = max(0.0, dot(n, l)) * light_intensity.x;
    float4 direct_diffuse =  inputPixel.color * light_color ;
    direct_diffuse *= shading;
 
    if (l_pov_position.z > (sampled_depth + 0.001))
        direct_diffuse *= 0.0;

    float3 indirect_diffuse;

    for (int i = 0; i < SAMPLE_COUNT; i++) {

        float4 sample_offset = PATTERN.Load(int3(i, 0, 0));
        float2 sample_uv = float2(uv.x + sample_offset.x * SAMPLING_UV_RADIUS,
                                  uv.y + sample_offset.y * SAMPLING_UV_RADIUS);
        
        float4 pixelLightPosition = W_POSITION_MAP.Sample(SAMPLER, sample_uv);
        float4 pixelLightNormal = NORMAL_MAP.Sample(SAMPLER, sample_uv);
        float4 pixelLightFlux = FLUX_MAP.Sample(SAMPLER, sample_uv);
        
        indirect_diffuse += getIndirectDiffuse(inputPixel,
                                        pixelLightPosition.xyz, 
                                        pixelLightNormal.xyz, 
                                        pixelLightFlux.xyz) * sample_offset.z;
    }
    
    return direct_diffuse + float4(indirect_diffuse, 0.0);
    
}