
Texture3D TEXTURE_A : register(t2);
Texture3D TEXTURE_B : register(t3);
RWTexture2D<float4> OUTPUT : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 groupID : SV_GroupID) {
    OUTPUT[groupID.xy] = TEXTURE_B.Load(int4(groupID.x, groupID.y, 0.0, 0.0)) + TEXTURE_A.Load(int4(groupID.x, groupID.y, 2.0, 0.0));
}