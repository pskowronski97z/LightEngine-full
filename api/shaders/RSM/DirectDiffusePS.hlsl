struct PS_INPUT {
    float4 position : SV_POSITION;
    float4 color : VT3_COLOR;
    float3 uvw : VT3_UVW;
    float3 normal : VT3_NORMAL;
    float3 tangent : VT3_TANGENT;
    float3 bitangent : VT3_BITANGENT;
    float3 world_position : VT3_POSITION;
    float3 gouraud_shading : VT3_COLOR1;
};

cbuffer CAMERA : register(b0) {
    matrix camera_matrix_;
    matrix projection_matrix_;
    float4 camera_world_position_;
}

float4 main(PS_INPUT input) : SV_TARGET {
    
    float3 v = normalize(camera_world_position_.xyz);
    float3 n = normalize(input.normal.xyz);
    float dot_product = dot(n, v);
    float4 coefficients = lit(dot_product, dot_product, 100);

    float shading = saturate(coefficients[1] + coefficients[2]);

    return float4(shading, shading, shading , 1.0f);
}