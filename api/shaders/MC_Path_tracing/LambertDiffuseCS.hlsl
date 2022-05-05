#include "MC_Properties.hlsli"

// This shader calculates Lambert diffuse from geometry and intersection data
// Dispatch(Frame width, Frame Height, 1)

// Input buffer of tuv values which are replaced with lighting values after calculations
// TUV values are used to calculate data of intersection points
RWTexture3D<float4> in_out_data : register(u0);

// Geometry data
Texture2D<float4> geometry_tris_v0 : register(t0);
Texture2D<float4> geometry_tris_v1 : register(t1);
Texture2D<float4> geometry_tris_v2 : register(t2);

// Frame pixel data
Texture2D<float4> pixel_world_position : register(t3);
Texture2D<float4> pixel_normal : register(t4);

struct LightSource {
    float4 position_;
    float4 color_;
};

// Light source data
LightSource light_source : register(b0);

float3 lambert_diffuse(float3 pixel_position, float3 pixel_normal, float3 pixel_color, LightSource point_light) {
    
    float3 light_ray = point_light.position_.xyz - pixel_position.xyz;
    float distance = length(light_ray);
    float attenuation = 1.0 / ((distance + 1.0) * (distance + 1.0));
    
    light_ray = normalize(light_ray);
    
    float intensity = point_light.color_.w * attenuation * max(0.0, dot(light_ray, pixel_normal));
    float3 color = pixel_color * point_light.color_.xyz;
    
    return color * intensity;
}

[numthreads(SAMPLES_COUNT, 1, 1)]
void main(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID) {
    
    float4 tuv = in_out_data[uint3(groupID.xy, groupThreadID.x)];
    
    if (tuv.x > 0.0) {

        uint2 triangle_index = uint2(tuv.w, 0);
        
        float3 v0 = geometry_tris_v0[triangle_index].xyz;
        float3 v1 = geometry_tris_v1[triangle_index].xyz;
        float3 v2 = geometry_tris_v2[triangle_index].xyz;
    
        triangle_index.y = 3;
        
        float3 v0_n = geometry_tris_v0[triangle_index].xyz;
        float3 v1_n = geometry_tris_v1[triangle_index].xyz;
        float3 v2_n = geometry_tris_v2[triangle_index].xyz;
        
        float3 intersection_pos = (1.0 - tuv.y - tuv.z) * v0 + tuv.y * v1 + tuv.z * v2;
        float3 intersection_n = normalize((1.0 - tuv.y - tuv.z) * v0_n + tuv.y * v1_n + tuv.z * v2_n);
        
        float3 illumination_value = lambert_diffuse(intersection_pos, intersection_n, float3(1.0, 1.0, 1.0), light_source);
        
        LightSource sampled;
        sampled.position_ = float4(intersection_pos, 0.0);
        sampled.color_ = float4(illumination_value, 1.0);

        illumination_value = lambert_diffuse(pixel_world_position[groupID.xy].xyz, pixel_normal[groupID.xy].xyz, float3(1.0, 1.0, 1.0), sampled);
    
        in_out_data[uint3(groupID.xy, groupThreadID.x)] = float4(illumination_value, 1.0);
    }
    else {
        in_out_data[uint3(groupID.xy, groupThreadID.x)] = float4(0.0, 0.0, 0.0, 0.0);
    }
}