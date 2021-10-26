const float4x4 matWorldViewProj;
const float4x4 matWorld;
float4 vecSkill41;

texture background_blurred_bmap_bmap;
sampler BackgroundMapSampler = sampler_state 
{ 
	Texture = <background_blurred_bmap_bmap>; 
	mipfilter = none;
	minfilter = none;
	magfilter = none;
	AddressU  = Clamp; 
	AddressV  = Clamp; 
}; 

void DiffuseVS( 
   in float4 InPos: POSITION, 
   in float3 InNormal: NORMAL, 
   in float2 InTex: TEXCOORD0, 
   out float4 OutPos: POSITION, 
   out float2 OutTex: TEXCOORD0) 
{ 
   OutPos = mul(InPos, matWorldViewProj); 
   OutTex = InTex;
} 

float4 DiffusePS(in float2 InTex: TEXCOORD0) : COLOR0
{ 
	float4 Color = tex2D(BackgroundMapSampler, InTex); 
   return Color*0.875;
} 
 

technique DiffuseTechnique 
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 DiffuseVS(); 
      PixelShader  = compile ps_3_0 DiffusePS(); 
   } 
}
