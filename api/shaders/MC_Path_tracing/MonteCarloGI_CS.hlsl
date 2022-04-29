// This shader samples world with N rays and calculates amount of reflected light.
// Results are stored in N textures, where one pixel contains intensity of light reflected
// towards corresponding pixel in screen space.  

Texture2D<float4> pixel_world_position : register(t0);
Texture2D<float4> pixel_normal : register(t1);
Texture2D<float4> pixel_tangent : register(t2);
Texture2D<float4> pixel_bitangent : register(t3);

Texture2D<float4> geometry_tris_v0 : register(t4);
Texture2D<float4> geometry_tris_v1 : register(t5);
Texture2D<float4> geometry_tris_v2 : register(t6);
Texture2D<float4> noise_map : register(t7);

RWTexture3D<float4> output : register(u0);


struct LightSource {
    float4 position_;
    float4 color_;
};

LightSource light_source : register(b0);

groupshared float min_distance;
groupshared float3 intersection_pos;
groupshared float3 intersection_n;


float3 ray_tris_intersection(float3 ray_origin, float3 ray_direction, float3 v_0, float3 v_1, float3 v_2) {
    
    float epsilon = 0.000001;
    
    float3 zeros = float3(0.0, 0.0, -1.0);
    float3 edge_0 = v_1 - v_0;
    float3 edge_1 = v_2 - v_0;
    float3 p_vector = cross(ray_direction, edge_1);
    float det = dot(edge_0, p_vector);
    
    if (det < -epsilon)
        return zeros;
    
    
    
    float3 t_vector = ray_origin - v_0;
    float u_coord = dot(t_vector, p_vector);
    
    if ((u_coord < 0.0) || (u_coord > det))
        return zeros;
    
    float3 q_vector = cross(t_vector, edge_0);
    float v_coord = dot(ray_direction, q_vector);
    
    if ((v_coord < 0.0) || ((u_coord + v_coord) > det))
        return zeros;
    
    float t_value = dot(edge_1, q_vector);
    
    float inv_det = 1.0 / det;
    
    t_value *= inv_det;
    u_coord *= inv_det;
    v_coord *= inv_det;

    return float3(t_value, u_coord, v_coord);
}


float3 lambert_diffuse(float3 pixel_position, float3 pixel_normal, float3 pixel_color, LightSource point_light) {
    
    float3 light_ray = point_light.position_.xyz - pixel_position.xyz;
    float distance = length(light_ray);
    float attenuation = 1.0 / ((distance + 1.0) * (distance + 1.0));
    
    light_ray = normalize(light_ray);
    
    return pixel_color * point_light.color_.xyz * point_light.color_.w * attenuation * max(0.0, dot(light_ray, pixel_normal));
}

[numthreads(14, 1, 1)]
void main(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID) {
     
    float random_0 = noise_map[groupID.xy].x - 0.5;
    float random_1 = noise_map[groupID.xy].y - 0.5;
    float random_2 = noise_map[groupID.xy].z;
    
    int seed = 1 - 2 * fmod(groupID.z, 2);
    
    random_0 *= seed * (groupID.z / (groupID.z + 0.1));
    random_1 *= -seed * (groupID.z / (groupID.z + 0.1));
    random_2 *= (seed + 2);

    float3x3 tangent_space_matrix = float3x3(
        pixel_normal[groupID.xy].xyz,
        pixel_tangent[groupID.xy].xyz,
        pixel_bitangent[groupID.xy].xyz);
   
    
    float3 sampling_ray = float3(random_2, random_0, random_1);
    
    sampling_ray = mul(sampling_ray, tangent_space_matrix);
    
    min_distance = 99999.0;
    
    GroupMemoryBarrierWithGroupSync();
    
    float3 v0 = geometry_tris_v0[uint2(groupThreadID.x, 0)].xyz;
    float3 v1 = geometry_tris_v1[uint2(groupThreadID.x, 0)].xyz;
    float3 v2 = geometry_tris_v2[uint2(groupThreadID.x, 0)].xyz;
    
    float3 v0_n = geometry_tris_v0[uint2(groupThreadID.x, 3)].xyz;
    float3 v1_n = geometry_tris_v1[uint2(groupThreadID.x, 3)].xyz;
    float3 v2_n = geometry_tris_v2[uint2(groupThreadID.x, 3)].xyz;
    
    float3 tuv;
    
    tuv = ray_tris_intersection(pixel_world_position[groupID.xy].xyz, sampling_ray, v0, v1, v2);
    
    if ((!((tuv.z == -1.0) || (tuv.x < 0.001))) && (tuv.x < min_distance)) {
        
        min_distance = tuv.x;
             
        intersection_pos = (1.0 - tuv.y - tuv.z) * v0 + tuv.y * v1 + tuv.z * v2;
        intersection_n = (1.0 - tuv.y - tuv.z) * v0_n + tuv.y * v1_n + tuv.z * v2_n;
            
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    if (groupThreadID.x == 0) {
        if (min_distance < 99999.0) {
            
            // Calculating illumination intensity value of intersection point
            
            intersection_n = normalize(intersection_n);
            
            LightSource world_sample;
            world_sample.position_ = float4(intersection_pos.xyz, 0.0);
            world_sample.color_.xyz = lambert_diffuse(intersection_pos.xyz, intersection_n.xyz, float3(1.0, 1.0, 1.0), light_source);
            world_sample.color_.w = 1.0;
            
            // Calculating contribution to illumination of current pixel
                       
            float3 contribution = lambert_diffuse(pixel_world_position[groupID.xy].xyz, pixel_normal[groupID.xy].xyz, float3(1.0, 1.0, 1.0), world_sample);
            
            output[groupID.xyz] = float4(contribution, 0.0);
        }
        else 
            output[groupID.xyz] = float4(0.0, 0.0, 0.0, 0.0);
        
    }

}