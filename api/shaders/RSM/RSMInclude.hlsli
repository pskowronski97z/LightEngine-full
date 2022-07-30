#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 800
#define SAMPLING_UV_RADIUS 0.2
#define SAMPLE_COUNT 200
#define ATT_QUADRATIC 1.0
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

SamplerState SAMPLER : register(s0);
