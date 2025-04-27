struct ModelViewProjection
{
    matrix MVP;
};
ConstantBuffer<ModelViewProjection> ModelViewProjectionCB : register(b0);

struct VertexPosColor
{
    float4 Position : POSITION;
    float3 Color : COLOR;
};

struct VertexShaderOutput
{
    float4 Color : COLOR;
    float4 Position : SV_Position;
};

VertexShaderOutput main(VertexPosColor IN)
{
    VertexShaderOutput OUT;
    OUT.Position = mul(ModelViewProjectionCB.MVP, float4(IN.Position.xyz, 1.0));
    OUT.Color = float4(IN.Color, 1.0f);
    return OUT;
}