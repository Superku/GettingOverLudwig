const float4x4 matWorldViewProj;
const float4x4 matWorld;
float fAlpha;
float4 vecSkill41;
float menuOpenPerc_var;
float menuPageItemSelected_var;

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
in float2 InTex2: TEXCOORD1, 
in int inBoneIndices: BLENDINDICES, 
out float4 OutPos: POSITION, 
out float4 OutTex: TEXCOORD0,
out float4 OutData: TEXCOORD1) 
{ 
	OutData.x = step(0, InTex.x);
	OutData.y = inBoneIndices + vecSkill41.y + vecSkill41.x*2; //floor(-InPos.y/30);
	OutData.y = saturate(OutData.y)*0.5+0.5;
	OutData.z = (menuPageItemSelected_var == inBoneIndices);
	OutData.w = 0;
	//InPos.xyz *= vecSkill41.w;
	InPos.x *= 1.75;
	InPos.y *= 1.15;
	OutPos = mul(InPos, matWorldViewProj); 
	OutTex.xy = InTex; //+vecSkill45.zw; 
	OutTex.y += vecSkill41.x;
	OutTex.zw = InTex2;
} 

float4 DiffusePS( 
in float4 InTex: TEXCOORD0,
in float4 InData: TEXCOORD1): COLOR 
{
	float4 data = tex2D(EncodedMapSampler, InTex);
	//return data.b*1110;
	float2 symbolTex = InTex.zw;
	//symbolTex.x /= 45;
	//symbolTex.x += data.b*255/45.0;
	symbolTex.x = InTex.x*32;
	symbolTex.x = fmod(symbolTex.x, 1);
	symbolTex.x += data.b*255;
	symbolTex.x /= 45;

	//return symbolTex.x;
	float4 symbol = tex2D(FontMapSampler, symbolTex); 
	//return symbol;

	symbol.rgb *= 1-InData.z;
	float thisIsBackground = 1-InData.x;
	float4 background = InData.z*0.9;
	symbol *= InData.y;
	float4 final = lerp(background, symbol, InData.x);
	final.a = (InData.z+0.6)*(1-InData.x)+InData.x*symbol.a; // * fAlpha

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
