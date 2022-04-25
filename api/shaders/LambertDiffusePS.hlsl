// This shader calculates diffuse lighting values using Lambert's law.

struct PixelShaderInput {
    float4 position_: SV_POSITION;
    float4 color_ : VT3_COLOR;
    float3 uvw_ : VT3_UVW;
    float3 normal_ : VT3_NORMAL;
    float3 tangent_ : VT3_TANGENT;
    float3 bitangent_ : VT3_BITANGENT;
    float3 world_position_ : VT3_POSITION;
    float3 gouraud_shading_ : VT3_COLOR1;
};

// Light source data is expected to be stored in b0 register and to be a structure which consists of two 4D vectors of floats
struct LightSource {
    float4 position_;
    float4 color_;
};

LightSource light_source : register(b0);
Texture2D<float4> shadow_map : register(t0);

float4 main(PixelShaderInput input_pixel) : SV_TARGET {
    
    input_pixel.normal_ = normalize(input_pixel.normal_);
    float3 light_ray = light_source.position_.xyz - input_pixel.world_position_.xyz;
    float distance = length(light_ray);
    float attenuation = 1.0 / (distance * distance);
    
    light_ray = normalize(light_ray);
    
    float3 pixel_lighting = light_source.color_ * light_source.color_.w * attenuation * max(0.0, dot(light_ray, input_pixel.normal_.xyz));
    //pixel_lighting *= shadow_map[input_pixel.position_.xy].x;
    pixel_lighting = pixel_lighting / (pixel_lighting + 1.0f); // Conversion to HDR
    pixel_lighting = pow(pixel_lighting, 0.8f); // Gamma correction
    
    return float4(pixel_lighting, 1.0);

}