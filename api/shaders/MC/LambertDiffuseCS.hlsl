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
Texture2D<float4> pixel_color: register(t8);
Texture2D<float4> depth_map : register(t9);

struct LightSource {
    float4 coordinates;
    float4 color;
    float4 intensity;
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

float3 lambert_diffuse_pt(float3 pixel_normal, float3 pixel_color, LightSource point_light) {
    
    //float3 light_ray = point_light.position_.xyz - pixel_position.xyz;
    float3 light_ray = point_light.coordinates.xyz;
    float distance = length(light_ray);
    float attenuation = 1.0 / (0.1 * distance * distance + 1.0);
    
    light_ray = normalize(light_ray);
    
    float intensity = point_light.intensity.x * attenuation * max(0.0, dot(light_ray, pixel_normal));
    float3 color = pixel_color * point_light.color.xyz;
    
    return color * intensity;
}

float3 lambert_diffuse_dir(float3 pixel_normal, float3 pixel_color, LightSource direct_light) {
    
    float3 light_ray = normalize(direct_light.coordinates.xyz);
    
    float intensity = direct_light.intensity.x * max(0.0, dot(light_ray, pixel_normal));
    float3 color = pixel_color * direct_light.color.xyz;
    
    return color * intensity;
}

[numthreads(SAMPLES_COUNT, 1, 1)]
void main(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID) {
    
    float4 tuv = in_out_data[uint3(groupID.xy, groupThreadID.x)];
    
    if (tuv.x > 0.0) {

        uint2 triangle_index = uint2(tuv.w, 0);     
        float3 v0_data = geometry_tris_v0[triangle_index].xyz;
        float3 v1_data = geometry_tris_v1[triangle_index].xyz;
        float3 v2_data = geometry_tris_v2[triangle_index].xyz;
        float3 intersection_pos = (1.0 - tuv.y - tuv.z) * v0_data + tuv.y * v1_data + tuv.z * v2_data;
        
        
        triangle_index.y = 1;     
        v0_data = geometry_tris_v0[triangle_index].xyz;
        v1_data = geometry_tris_v1[triangle_index].xyz;
        v2_data = geometry_tris_v2[triangle_index].xyz;    
        float3 intersection_c = normalize((1.0 - tuv.y - tuv.z) * v0_data + tuv.y * v1_data + tuv.z * v2_data);
        
        
        triangle_index.y = 3;   
        v0_data = geometry_tris_v0[triangle_index].xyz;
        v1_data = geometry_tris_v1[triangle_index].xyz;
        v2_data = geometry_tris_v2[triangle_index].xyz;     
        float3 intersection_n = normalize((1.0 - tuv.y - tuv.z) * v0_data + tuv.y * v1_data + tuv.z * v2_data);
        
        
        // Calculate lighting in intersection
        
        LightSource direct_light;
        direct_light.coordinates = l_pov_world_position;
        direct_light.color = light_color;
        direct_light.intensity = light_intensity;
        
        float3 illumination_value = lambert_diffuse_dir(intersection_n, intersection_c, direct_light);
        
        // Shadow mapping
                    
        float4 l_pov_position = mul(float4(intersection_pos, 1.0), l_pov_transform_matrix);
    
        l_pov_position = mul(l_pov_position, l_pov_projection_matrix);
        l_pov_position /= l_pov_position.w;
    
        int3 coords;
        float2 depth_map_dims;
        depth_map.GetDimensions(depth_map_dims.x, depth_map_dims.y);
        depth_map_dims *= 0.5;
        
        coords.x = mad(l_pov_position.x, depth_map_dims.x, depth_map_dims.x);
        coords.y = mad(-l_pov_position.y, depth_map_dims.y, depth_map_dims.y);
        coords.z = 0;
        
        float4 sampled_depth = depth_map.Load(coords);
        
        if (l_pov_position.z > (sampled_depth.x + 0.001))
            illumination_value *= 0.0;
        
        // Calculate sample contribution 
        
        float3 pixel_world_pos = pixel_world_position[groupID.xy].xyz;
        float3 pixel_n = pixel_normal[groupID.xy].xyz;
        
        LightSource point_light;
        point_light.coordinates = float4(intersection_pos - pixel_world_pos, 1.0);
        point_light.color = float4(illumination_value, 1.0);
        point_light.intensity = float4(1.0, 1.0, 1.0, 1.0);

        illumination_value = lambert_diffuse_pt(pixel_n, pixel_color[groupID.xy].xyz, point_light);
        
        
        float3 pixel_to_intersection = pixel_world_pos - intersection_pos;
        
        float is_shaded = max(0.0, dot(pixel_to_intersection, intersection_n)) * max(0.0, dot(-pixel_to_intersection, pixel_n));
        
        if (is_shaded > 0.0)
            is_shaded = 1.0;
    
        illumination_value *= is_shaded;
         
        in_out_data[uint3(groupID.xy, groupThreadID.x)] = float4(illumination_value, 1.0);
    }
    else {
        in_out_data[uint3(groupID.xy, groupThreadID.x)] = float4(0.0, 0.0, 0.0, 0.0);
    }
}