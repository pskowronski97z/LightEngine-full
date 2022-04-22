struct Light
{
    float4 position_;
    float4 color_;
};

Light light_0 : register(b0);
Texture2D<float4> GEOMETRY_TRIS_V0_S : register(t0);
Texture2D<float4> GEOMETRY_TRIS_V1_S : register(t1);
Texture2D<float4> GEOMETRY_TRIS_V2_S : register(t2);
Texture2D<float4> WORLD_POSITIONS : register(t3);
Texture2D<float4> NORMALS : register(t4);
RWTexture2D<unorm float4> FRAME_BUFFER : register(u0);



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
    
    float3 light_position = light_0.position_.xyz;

    float3 l = light_position - WORLD_POSITIONS[groupID.xy].xyz;
    float distance = length(l);
    float attenuation = 1.0 / (distance * distance);
    
    l = normalize(l);
    
    float3 tuv = ray_tris_intersection(
        WORLD_POSITIONS[groupID.xy].xyz,
        l, 
        GEOMETRY_TRIS_V0_S[uint2(groupThreadID.x, 0)].xyz,
        GEOMETRY_TRIS_V1_S[uint2(groupThreadID.x, 0)].xyz,
        GEOMETRY_TRIS_V2_S[uint2(groupThreadID.x, 0)].xyz);
    
	if (!((tuv.z == -1.0) || (tuv.x < 0.001)))
        is_lighted *= 0.5;
    
    GroupMemoryBarrierWithGroupSync();
    
    if ((WORLD_POSITIONS[groupID.xy].w == 1.0)  && (groupThreadID.x == 0))
        FRAME_BUFFER[groupID.xy] = light_0.color_ * light_0.color_.w * is_lighted * attenuation * max(0.0, dot(l, NORMALS[groupID.xy].xyz));
   
}