struct PS_INPUT {
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

Texture2D<float4> SHADOW_MAP : register(t0);
SamplerState SAMPLER : register(s0);

float4 main(PS_INPUT input) : SV_TARGET {
    
    float4 l_pov_position = mul(float4(input.world_position, 1.0), l_pov_transform_matrix_);
    l_pov_position = mul(l_pov_position, l_pov_projection_matrix_); 
    l_pov_position /= l_pov_position.w;
    
    float2 uv;
    uv.x = l_pov_position.x * 0.5 + 0.5;
    uv.y = -l_pov_position.y * 0.5 + 0.5;
    float sampled_depth = SHADOW_MAP.Sample(SAMPLER, uv);
    
    float3 n = normalize(input.normal.xyz);
    float3 l = normalize(l_pov_world_position_.xyz);
    float4 diffuse = max(0.0, dot(n, l)) * input.color * light_color * light_intensity[0];
 
    if (l_pov_position.z > (sampled_depth + 0.001))
        diffuse *= 0.0;

    return diffuse;
    
}