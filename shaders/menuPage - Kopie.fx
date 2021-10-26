const float4x4 matWorldViewProj;
const float4x4 matWorld;
float fAlpha;
float4 vecSkill41;

texture bmpMenuStringsEncoded_bmap;
sampler EncodedMapSampler = sampler_state 
{ 
	Texture = <bmpMenuStringsEncoded_bmap>; 
	mipfilter = none;
	minfilter = none;
	magfilter = none;
	AddressU  = Clamp; 
	AddressV  = Clamp; 
}; 

texture bmpMenuFont_bmap;
sampler FontMapSampler = sampler_state 
{ 
	Texture = <bmpMenuFont_bmap>; 
	AddressU  = Clamp; 
	AddressV  = Wrap; 
}; 

void DiffuseVS( 
in float4 InPos: POSITION, 
in float2 InTex: TEXCOORD0, 
out float4 OutPos: POSITION, 
out float2 OutTex: TEXCOORD0,
out float2 OutData: TEXCOORD1) 
{ 
	//InPos.xyz *= vecSkill41.w;
	InPos.x *= 1.75;
	InPos.y *= 1.15;
	OutPos = mul(InPos, matWorldViewProj); 
	OutTex = InTex; //+vecSkill45.zw; 
	OutData.x = step(0, InTex.x);
	OutData.y = 0;
} 

float4 DiffusePS( 
in float2 InTex: TEXCOORD0,
in float2 InData: TEXCOORD1): COLOR 
{
	float4 data = tex2D(EncodedMapSampler, InTex);
	//return data.b*1110;
	float2 symbolTex = float2(InTex.x*32, InTex.y*36);
	symbolTex.y -= 0.1;
	symbolTex.x = fmod(symbolTex.x, 1);
	symbolTex.x += data.b*255;
	symbolTex.x /= 45;
	//return symbolTex.x;
	float4 symbol = tex2D(FontMapSampler, symbolTex)+0.7; 

	float4 final = symbol;
	final.a = 0.5*(1-InData.x)+InData.x*symbol.a; // * fAlpha

	return final; 
} 


technique DiffuseTechnique 
{ 
	pass P0 
	{ 
		//cullmode = none;
		zWriteEnable = true;
		alphaBlendEnable = false;
		zWriteEnable = false;
		alphaBlendEnable = true;
		VertexShader = compile vs_3_0 DiffuseVS(); 
		PixelShader  = compile ps_3_0 DiffusePS(); 
	} 
}
