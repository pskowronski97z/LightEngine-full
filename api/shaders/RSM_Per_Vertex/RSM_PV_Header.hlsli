struct Pixel {
    float4 position : SV_POSITION;
    float4 color : VT3_COLOR;
    float3 normal : VT3_NORMAL;
    float3 world_position : VT3_POSITION;
    float3 depth : VT3_COLOR1;
    float3 indirect : VT3_COLOR2;
};

cbuffer LIGHT_POV_CAMERA : register(b1) {
    matrix l_pov_transform_matrix;
    matrix l_pov_projection_matrix;
    float4 l_pov_world_position;
}

cbuffer LIGHT_SOURCE : register(b2) {
    float4 light_direction;
    float4 light_color;
    float4 light_intensity;
}

SamplerState SAMPLER : register(s0);