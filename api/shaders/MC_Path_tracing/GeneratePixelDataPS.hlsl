struct PixelShaderInput {
    float4 position_ : SV_POSITION;
    float4 color_ : VT3_COLOR;
    float3 uvw_ : VT3_UVW;
    float3 normal_ : VT3_NORMAL;
    float3 tangent_ : VT3_TANGENT;
    float3 bitangent_ : VT3_BITANGENT;
    float3 world_position_ : VT3_POSITION;
};

struct PixelShaderOutput {
    float4 world_position_ : SV_Target0;
    float4 normal_ : SV_Target1;
    float4 tangent_ : SV_Target2;
    float4 bitangent_ : SV_Target3;
    float4 uvw_ : SV_Target4;
    float4 color_ : SV_Target5;
};

PixelShaderOutput main(PixelShaderInput input_pixel) {
    
    PixelShaderOutput output;
    
    input_pixel.normal_ = normalize(input_pixel.normal_);
    input_pixel.tangent_ = normalize(input_pixel.tangent_);
    input_pixel.bitangent_ = normalize(input_pixel.bitangent_);
    
    output.world_position_ = float4(input_pixel.world_position_, 1.0);
    output.normal_ = float4(input_pixel.normal_, 1.0);
    output.tangent_ = float4(input_pixel.tangent_, 1.0);
    output.bitangent_ = float4(input_pixel.bitangent_, 1.0);
    output.uvw_ = float4(input_pixel.uvw_, 1.0);
    output.color_ = input_pixel.color_;
    
    return output;
}