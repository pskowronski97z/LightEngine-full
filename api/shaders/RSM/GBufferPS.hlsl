struct PixelShaderInput {
    float4 position_ : SV_POSITION;
    float4 color_ : VT3_COLOR;
    float3 uvw_ : VT3_UVW;
    float3 normal_ : VT3_NORMAL;
    float3 tangent_ : VT3_TANGENT;
    float3 bitangent_ : VT3_BITANGENT;
    float3 world_position_ : VT3_POSITION;
    float3 depth_ : VT3_COLOR1;
};

struct PixelShaderOutput {
    float4 world_position_ : SV_Target0;
    float4 normal_ : SV_Target1;
    float4 depth_ : SV_Target2;
    float4 flux_ : SV_Target3;
};

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

PixelShaderOutput main(PixelShaderInput input_pixel) {
    
    PixelShaderOutput output;

    output.world_position_ = float4(input_pixel.world_position_, 1.0);
    output.normal_ = float4(normalize(input_pixel.normal_), 1.0);
    output.depth_ = float4(input_pixel.depth_, 1.0);
    
    float3 l = normalize(l_pov_world_position_.xyz);
    output.flux_ = max(0.0, dot(output.normal_.xyz, l)) * input_pixel.color_ * light_color * light_intensity.x;
    output.flux_.w = 1.0;
    
    return output;
}