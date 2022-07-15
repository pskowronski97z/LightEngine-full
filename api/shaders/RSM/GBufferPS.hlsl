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
};


PixelShaderOutput main(PixelShaderInput input_pixel) {
    
    PixelShaderOutput output;

    output.world_position_ = float4(input_pixel.world_position_, 1.0);
    output.normal_ = float4(normalize(input_pixel.normal_), 1.0);
    output.depth_ = float4(input_pixel.depth_, 1.0);
    
    return output;
}