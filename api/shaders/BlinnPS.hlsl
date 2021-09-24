#include "PixelShader.hlsli"

float3 calculate_blinn_phong_pt(float3 specular_color, float3 diffuse_color, float3 ambient_color, float3 light_position, float3 light_color, int glossiness, PS_INPUT pixel) {
    
    float3 v = pixel.world_position;
    float3 l = light_position - v;
    float d = length(l);
    
    v = -normalize(v);
    l = normalize(l);
    
    float3 h = normalize(l + v);
    float3 specular = specular_color * pow(max(0.0f, dot(pixel.normal, h)), glossiness);
    float3 diffuse = diffuse_color * get_attenuation(d) * max(0.0f, dot(l, pixel.normal));
    
    return (specular + diffuse + ambient_color) * light_color;
}

float3 calculate_blinn_phong_dir(float3 specular_color, float3 diffuse_color, float3 ambient_color, float3 light_direction, float3 light_color, int glossiness, PS_INPUT pixel) {
    float3 v = pixel.world_position;
    float3 l = -light_direction;
    
    v = -normalize(v);
    l = normalize(l);
    
    float3 h = normalize(l + v);
    float3 specular = specular_color * pow(max(0.0f, dot(pixel.normal, h)), glossiness);
    float3 diffuse = diffuse_color * max(0.0f, dot(l, pixel.normal));
    
    return (specular + diffuse + ambient_color) * light_color;
}

float4 main(PS_INPUT input) : SV_TARGET {
    
    float3 lighting = float3(0.0, 0.0, 0.0); 
    float4 used_diffuse;
    
    if(use_diffuse_map_)
        used_diffuse = DIFFUSE_MAP.Sample(DEFAULT_SAMPLER, float2(input.uvw[0], input.uvw[1]));
    else
        used_diffuse = diffuse;
    
    if(use_normal_map_)
        input.normal = get_mapped_normal(input);
    else
        input.normal = normalize(input.normal);
    
    lighting += calculate_blinn_phong_dir(specular, used_diffuse, used_diffuse * ambient, mul(coordinates_dir, camera_matrix_), color_dir, glossiness, input);
    lighting += calculate_blinn_phong_pt(specular, used_diffuse, used_diffuse * ambient, mul(coordinates_pt, camera_matrix_), color_pt, glossiness, input);
      
    lighting = saturate(lighting);
    
    return float4(lighting, 1.0);

}