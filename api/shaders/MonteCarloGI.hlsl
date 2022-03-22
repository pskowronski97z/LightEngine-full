#include "PixelShader.hlsli"

struct Vertex {
    float4 position_;
    float4 color_;
    float3 uvw_;
    float3 normal_;
};

float4 main(PS_INPUT ps_input) : SV_TARGET
{
    Vertex v1, v2, v3, v4;
    
    v1.position_ = float4(25.0, 0.0, -25.0, 0.0);
    v1.color_ = float4(1.0, 1.0, 1.0, 1.0);
    v1.normal_ = float3(0.0, 1.0, 0.0);
    
    v2.position_ = float4(-25.0, 0.0, 25.0, 0.0);
    v2.color_ = float4(1.0, 1.0, 1.0, 1.0);
    v2.normal_ = float3(0.0, 1.0, 0.0);
    
    v3.position_ = float4(25.0, 0.0, 25.0, 0.0);
    v3.color_ = float4(1.0, 1.0, 1.0, 1.0);
    v3.normal_ = float3(0.0, 1.0, 0.0);
    
    v4.position_ = float4(-25.0, 0.0, -25.0, 0.0);
    v4.color_ = float4(1.0, 1.0, 1.0, 1.0);
    v4.normal_ = float3(0.0, 1.0, 0.0);
     
    
    ps_input.world_position = mul(float4(ps_input.world_position, 1.0), camera_matrix_);
    
    float3 light_ray = mul(coordinates_pt, camera_matrix_) - ps_input.world_position;
    float d = length(light_ray);
    light_ray = normalize(light_ray);
    
    float3 diffuse_shading = diffuse * max(0.0f, dot(light_ray, normalize(ps_input.normal))) * get_attenuation(d) * additional_pt[0];
    
    diffuse_shading = diffuse_shading / (diffuse_shading + 1.0f); // Conversion to HDR
    diffuse_shading = pow(diffuse_shading, 0.8f); // Gamma correction
    
    return float4(diffuse_shading, 1.0);
}