
Texture2D TEXTURE_A : register(t2);
Texture2D TEXTURE_B : register(t3);
RWTexture2D<float4> OUTPUT : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 groupID : SV_GroupID) {
    OUTPUT[groupID.xy] = TEXTURE_A[groupID.xy] + TEXTURE_B[groupID.xy];

}