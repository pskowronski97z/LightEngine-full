#define SAMPLES_PER_PIXEL 20

RWTexture3D<float4> illumination_map : register(u0);

// Geometry data
Texture2D<float4> geometry_tris_v0 : register(t0);
Texture2D<float4> geometry_tris_v1 : register(t1);
Texture2D<float4> geometry_tris_v2 : register(t2);

// Frame pixel data
Texture2D<float4> pixel_world_position : register(t3);
Texture2D<float4> pixel_normal : register(t4);
Texture2D<float4> pixel_color: register(t5);
Texture2D<float4> pixel_tangent : register(t6);

Texture2D<float4> random_floats : register(t7);
Texture2D<uint> indices_lookup : register(t8);

struct LightSource {
    float4 position_;
    float4 color_;
};

// Light source data
cbuffer LIGHT_SOURCE : register(b0) {
    LightSource light_source;
};

float3 lambert_diffuse(float3 pixel_position, float3 pixel_normal, float3 pixel_color, LightSource point_light) {
    
    float3 light_ray = point_light.position_.xyz - pixel_position.xyz;
    float distance = length(light_ray);
    float attenuation = 1.5 / ((distance + 1.0) * (distance + 1.0));
    
    light_ray = normalize(light_ray);
    
    float intensity = point_light.color_.w * attenuation * max(0.0, dot(light_ray, pixel_normal));
    float3 color = pixel_color * point_light.color_.xyz;
    
    return color * intensity;
}

[numthreads(SAMPLES_PER_PIXEL, 1, 1)]
void main(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID) {
    
    //uint objects_sizes[5] = { 0, 12, 22, 76, 156};
    //uint objects_sizes[5] = { 0, 51, 1019, 1325, 1837 };
    //uint objects_sizes[5] = { 0, 512, 732, 952, 1172};
    
    uint2 triangle_index;
    float3 v0_data;
    float3 v1_data;
    float3 v2_data;
    float3 intersection_pos;
    float3 intersection_c;
    float3 intersection_n;
    float3 illumination_value;
    LightSource sampled;
    
    uint object_index = uint(pixel_tangent[groupID.xy].w);
    float row_width = float(indices_lookup[uint2(0, object_index)]);
    
    //float random_float = frac(random_floats[groupID.xy].x*(groupThreadID.x + 1)*(groupID.z + 1));
    //float random_float = (sin(groupThreadID.x + groupThreadID.y + groupID.x) + 2.0) / 4.0;
    float random_float = frac(random_floats[groupID.xy].x + groupThreadID.x);
    //float random_float = frac(random_floats[groupID.xy] + groupThreadID.x/groupID.z);
    //float random_float = frac(sin(dot(float2(groupID.z, groupThreadID.z), float2(12.9898, 78.233))) * 43758.5453);
   
    uint random_lookup_index = uint(lerp(1.0, row_width, random_float));
    uint neighbour_index = indices_lookup[uint2(random_lookup_index, object_index)];
        
    float4 tuv = float4(0.5, 0.33, 0.33, 0.0);
    
    triangle_index = uint2(neighbour_index, 0);
    v0_data = geometry_tris_v0[triangle_index].xyz;
    v1_data = geometry_tris_v1[triangle_index].xyz;
    v2_data = geometry_tris_v2[triangle_index].xyz;
    intersection_pos = (1.0 - tuv.y - tuv.z) * v0_data + tuv.y * v1_data + tuv.z * v2_data;
        
        
    triangle_index.y = 1;
    v0_data = geometry_tris_v0[triangle_index].xyz;
    v1_data = geometry_tris_v1[triangle_index].xyz;
    v2_data = geometry_tris_v2[triangle_index].xyz;
    intersection_c = (1.0 - tuv.y - tuv.z) * v0_data + tuv.y * v1_data + tuv.z * v2_data;
        
        
    triangle_index.y = 3;
    v0_data = geometry_tris_v0[triangle_index].xyz;
    v1_data = geometry_tris_v1[triangle_index].xyz;
    v2_data = geometry_tris_v2[triangle_index].xyz;
    intersection_n = normalize((1.0 - tuv.y - tuv.z) * v0_data + tuv.y * v1_data + tuv.z * v2_data);
                      
    illumination_value = lambert_diffuse(intersection_pos, intersection_n, intersection_c, light_source);

    sampled.position_ = float4(intersection_pos, 0.0);
    sampled.color_ = float4(illumination_value, 1.0);

    illumination_value = lambert_diffuse(pixel_world_position[groupID.xy].xyz, pixel_normal[groupID.xy].xyz, pixel_color[groupID.xy].xyz, sampled);

    illumination_map[uint3(groupID.xy, groupThreadID.x)] = float4(illumination_value, 0.0);
}