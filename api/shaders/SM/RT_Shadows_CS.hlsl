// Light source data is expected to be stored in b0 register and to be a structure which consists of two 4D vectors of floats
struct LightSource {
    float4 position_;
    float4 color_;
};

LightSource light_source : register(b0);
Texture2D<float4> geometry_tris_v0 : register(t0);
Texture2D<float4> geometry_tris_v1 : register(t1);
Texture2D<float4> geometry_tris_v2 : register(t2);
Texture2D<float4> pixel_positions : register(t3);
RWTexture2D<float4> shadow_map : register(u0);



groupshared float is_lighted = 1.0;

float3 ray_tris_intersection(float3 ray_origin, float3 ray_direction, float3 v_0, float3 v_1, float3 v_2) {
    
    float epsilon = 0.000001;
    
    float3 zeros = float3(0.0, 0.0, -1.0);
    float3 edge_0 = v_1 - v_0;
    float3 edge_1 = v_2 - v_0;
    float3 p_vector = cross(ray_direction, edge_1);
    float det = dot(edge_0, p_vector);
    
    if ((det > -epsilon) && (det < epsilon))
        return zeros;
    
    float inv_det = 1.0 / det;
    float3 t_vector = ray_origin - v_0;  
    float u_coord = dot(t_vector, p_vector) * inv_det;
    
    if ((u_coord < 0.0)  || (u_coord > 1.0))
        return zeros;
    
    float3 q_vector = cross(t_vector, edge_0);  
    float v_coord = dot(ray_direction, q_vector) * inv_det;
    
    if ((v_coord < 0.0)  || ((u_coord + v_coord) > 1.0))
        return zeros;
    
    float t_value = dot(edge_1, q_vector) * inv_det;

    return float3(t_value, u_coord, v_coord);
}

[numthreads(14, 1, 1)]
void main(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID) {   
    
    float3 light_position = light_source.position_.xyz;

    float3 l = light_position - pixel_positions[groupID.xy].xyz;
    float distance = length(l);
    float attenuation = 1.0 / (distance * distance);
    
    l = normalize(l);
    
    float3 tuv = ray_tris_intersection(
        pixel_positions[groupID.xy].xyz,
        l, 
        geometry_tris_v0[uint2(groupThreadID.x, 0)].xyz,
        geometry_tris_v1[uint2(groupThreadID.x, 0)].xyz,
        geometry_tris_v2[uint2(groupThreadID.x, 0)].xyz);
    
	if (!((tuv.z == -1.0) || (tuv.x < 0.001)))
        is_lighted *= 0.0;
    
    GroupMemoryBarrierWithGroupSync();
    
    if(groupThreadID.x == 0)
        shadow_map[groupID.xy] = float4(is_lighted, 0.0, 0.0, 0.0);
    
}