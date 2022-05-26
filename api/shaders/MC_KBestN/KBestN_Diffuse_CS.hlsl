// This Compute Shader calcultes the contribution of the best selected triangles neighbouring 
// each pixel to it's illumination. 
// Dispatch X dimension - frame width
// Dispatch Y dimension - frame height
// Dispatch Z dimension - max neighbours (best neighbour lookup table depth)

// ----------------------- Input --------------------------
// Frame G-Buffer
Texture2D<float4> pixel_world_position : register(t0);
Texture2D<float4> pixel_normal : register(t1);
Texture2D<float4> pixel_tangent : register(t2);
Texture2D<float4> pixel_bitangent : register(t3);
Texture2D<float4> pixel_color : register(t4);

// Contributing (merged) triangles buffer
Texture2D<float4> triangle_v0 : register(t5);
Texture2D<float4> triangle_v1 : register(t6);
Texture2D<float4> triangle_v2 : register(t7);

// Per pixel best neighbouring triangles indices
Texture3D<int> best_neighbour_index : register(t8);

// Point light source
struct PointLight {
    float4 position_;
    float4 color_;
};

cbuffer LightSources : register(b0) {
    PointLight point_light;
}

// ----------------------- Output --------------------------
// The output is a 3D texture which width and height is 
// the same as a frame. The depth is the same as the depth of 
// neighbours indices lookup table. 
RWTexture3D<float4> neighbour_contribution : register(u0);

float3 lambert_diffuse(float3 pixel_position, float3 pixel_normal, float3 pixel_color, PointLight point_light) {
    
    float3 light_ray = point_light.position_.xyz - pixel_position.xyz;
    float distance = length(light_ray);
    float attenuation = 1.0 / ((distance + 1.0) * (distance + 1.0));
    
    light_ray = normalize(light_ray);
    
    float intensity = point_light.color_.w * attenuation * max(0.0, dot(light_ray, pixel_normal));
    float3 color = pixel_color * point_light.color_.xyz;
    
    return color * intensity;
}

float3 lambert_diffuse_dir(float3 pixel_normal, float3 pixel_color, float3 light_direction, float3 light_color) {
       
    float intensity = max(0.0, dot(-light_direction, pixel_normal));
    float3 color = pixel_color * light_color;
    
    return color * intensity;
}

[numthreads(4, 1, 1)]
void main( uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID) {
    
    uint parent_object_index = pixel_tangent[groupID.xy].w;
    uint parent_triangle_index = pixel_bitangent[groupID.xy].w;
    uint neighbour_index = best_neighbour_index[uint3(parent_triangle_index, parent_object_index, groupThreadID.x)];
    
    if (neighbour_index == -1) {
        neighbour_contribution[uint3(groupID.xy, groupThreadID.x)] = float4(0.0, 0.0, 0.0, 0.0);
    }
    else {
    
        uint2 neighbour_data_index;
        float3 v0_data;
        float3 v1_data;
        float3 v2_data;
        float3 intersection_pos;
        float3 intersection_c;
        float3 intersection_n;
        float3 illumination_value;
        PointLight neighbour_emission;
    
        neighbour_data_index = uint2(neighbour_index, 0);
        v0_data = triangle_v0[neighbour_data_index].xyz;
        v1_data = triangle_v1[neighbour_data_index].xyz;
        v2_data = triangle_v2[neighbour_data_index].xyz;
        intersection_pos = v0_data + v1_data + v2_data;
        intersection_pos *= 0.33;
        
        neighbour_data_index.y = 1;
        v0_data = triangle_v0[neighbour_data_index].xyz;
        v1_data = triangle_v1[neighbour_data_index].xyz;
        v2_data = triangle_v2[neighbour_data_index].xyz;
        intersection_c = v0_data + v1_data + v2_data;
        intersection_c *= 0.33;
               
        neighbour_data_index.y = 3;
        v0_data = triangle_v0[neighbour_data_index].xyz;
        v1_data = triangle_v1[neighbour_data_index].xyz;
        v2_data = triangle_v2[neighbour_data_index].xyz;
        intersection_n = v0_data + v1_data + v2_data;
        intersection_n *= 0.33;
        intersection_n = normalize(intersection_n);
    
        illumination_value = lambert_diffuse(intersection_pos, intersection_n, intersection_c, point_light);
   
        neighbour_emission.position_ = float4(intersection_pos, 0.0);
        neighbour_emission.color_ = float4(illumination_value, 1.0);
 
        illumination_value = lambert_diffuse(pixel_world_position[groupID.xy].xyz, pixel_normal[groupID.xy].xyz, pixel_color[groupID.xy].xyz, neighbour_emission);
        
        
        neighbour_contribution[uint3(groupID.xy, groupThreadID.x)] = float4(max(illumination_value, 0.0), 1.0);
    }
}