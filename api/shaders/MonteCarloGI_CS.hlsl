// This shader samples world with N rays and calculates amount of reflected light.
// Results are stored in N textures, where one pixel contains intensity of light reflected
// towards corresponding pixel in screen space.  

Texture2D<float4> pixel_world_position : register(t0);
Texture2D<float4> pixel_normal : register(t1);
Texture2D<float4> pixel_tangent : register(t2);
Texture2D<float4> pixel_bitangent : register(t3);
RWTexture2D<float4> output : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 groupID : SV_GroupID) {
    
    float3x3 tangent_space_matrix = float3x3(
        pixel_normal[groupID.xy].xyz,
        pixel_tangent[groupID.xy].xyz,
        pixel_bitangent[groupID.xy].xyz);
   
    
    float3 sampling_ray = float3(0.5, 1.0, -1.0);
    
    sampling_ray = normalize(sampling_ray);
    
    sampling_ray = mul(sampling_ray, tangent_space_matrix);
    
    output[groupID.xy] = float4(sampling_ray, 0.0);
}