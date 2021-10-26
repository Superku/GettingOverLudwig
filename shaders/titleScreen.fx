const float4x4 matWorldViewProj;
const float4x4 matWorld;
float fAlpha;
float4 vecSkill41;

texture entSkin1;

sampler ColorMapSampler = sampler_state 
{ 
	Texture = <entSkin1>; 
	AddressU  = Clamp; 
	AddressV  = Clamp; 
}; 

void DiffuseVS( 
in float4 InPos: POSITION, 
in float2 InTex: TEXCOORD0, 
out float4 OutPos: POSITION, 
out float2 OutTex: TEXCOORD0) 
{ 
	//InPos.xyz *= vecSkill41.w;
	OutPos = mul(InPos, matWorldViewProj); 
	OutTex = InTex; //+vecSkill45.zw; 
} 

float4 DiffusePS( 
in float2 InTex: TEXCOORD0): COLOR 
{ 
	float4 Color = tex2D(ColorMapSampler, InTex); 
	Color.a *= fAlpha;
	
	return Color; 
} 


technique DiffuseTechnique 
{ 
	pass P0 
	{ 
		//cullmode = none;
		VertexShader = compile vs_3_0 DiffuseVS(); 
		PixelShader  = compile ps_3_0 DiffusePS(); 
	} 
}
