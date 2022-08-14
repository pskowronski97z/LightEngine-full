struct PixelShaderInput {
    float4 position : SV_POSITION;
    float4 color : VT3_COLOR;
    float3 uvw : VT3_UVW;
    float3 normal : VT3_NORMAL;
    float3 tangent : VT3_TANGENT;
    float3 bitangent : VT3_BITANGENT;
    float3 world_position : VT3_POSITION;
    float3 depth : VT3_COLOR1;
};


float4 main(PixelShaderInput input) : SV_TARGET {
    
    return float4(input.depth, 1.0);
}