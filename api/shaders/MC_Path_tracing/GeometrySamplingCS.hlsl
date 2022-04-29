#include "MC_Properties.hlsli"
// This shader samples loaded geometry for each pixel using sampling rays taken from corresponding position in ray buffer
// Dispatch(Frame width, Frame Height, Sampling rays per pixel)

// Input buffers
// Sampled geometry - limited to 16k tris (to upgrade)
Texture2D<float4> geometry_tris_v0 : register(t0);
Texture2D<float4> geometry_tris_v1 : register(t1);
Texture2D<float4> geometry_tris_v2 : register(t2);

// Pixel data of frame
Texture2D<float4> pixel_world_position : register(t3);
Texture2D<float4> pixel_normal : register(t4);
Texture2D<float4> pixel_tangent : register(t5);
Texture2D<float4> pixel_bitangent : register(t6);

// Sampling rays for each pixel od frame
Texture2D<float4> sampling_rays : register(t7);

// Output buffer - x(t), y(u), z(v), w(Triangle index)
// Each slice stores values of N-th iteration of random sampling
// The depth of this texture is indexed by groupID.z
// Texture depth (Z dimension) = Sampling iterations (Samples count) = groupID.z
// groupThreadID.x coordinate is used to iterate through triangles
RWTexture3D<float4> intersections_data : register(u0);

float3 intersection_test(float3 ray_origin, float3 ray_direction, float3 v_0, float3 v_1, float3 v_2) {
    
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


[numthreads(TRIANGLES_COUNT, 1, 1)]
void main( uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID) {
    
    int seed = 1 - 2 * fmod(groupID.z, 2);
    float scaler = groupID.z / (groupID.z + 0.1);
    
    float ray_x = (sampling_rays[groupID.xy].x - 0.5) * seed * scaler;
    float ray_y = sampling_rays[groupID.xy].y * (seed + 2);
    float ray_z = (sampling_rays[groupID.xy].z - 0.5) * -seed * scaler;
     
    float3x3 tangent_space_matrix = float3x3(
        pixel_tangent[groupID.xy].xyz,
        pixel_normal[groupID.xy].xyz,
        pixel_bitangent[groupID.xy].xyz);
    
    float3 sampling_ray = normalize(float3(ray_x, ray_y, ray_z));
    sampling_ray = mul(sampling_ray, tangent_space_matrix);
    
    uint2 geometry_index = uint2(groupThreadID.x, 0);
     
    float3 v0 = geometry_tris_v0[geometry_index].xyz;
    float3 v1 = geometry_tris_v1[geometry_index].xyz;
    float3 v2 = geometry_tris_v2[geometry_index].xyz;
    
    float4 tuv;
    tuv.xyz = intersection_test(pixel_world_position[groupID.xy].xyz, sampling_ray, v0, v1, v2);
    tuv.w = float(groupThreadID.x);
    
    if(groupThreadID.x == 0)
        intersections_data[groupID.xyz] = float4(0.0, 0.0, 0.0, 0.0);
    
    if (!((tuv.z == -1.0) || (tuv.x < 0.001))) 
        intersections_data[groupID.xyz] = tuv;
    
         
}