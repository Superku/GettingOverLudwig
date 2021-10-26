const float4x4 matWorldViewProj; // World*view*projection matrix. 
//const float4x4 matViewProj; // World*view*projection matrix. 
const float4x4 matView;
const float4x4 matWorld; // World matrix. 
const float4 vecViewDir;  // The sun direction vector.
float4 vecSkill41;
float4 vecSkill45;
float4 vecColor;
//float4 vecSkill9;
float4 vecSkill13;
float4 vecSkill17;
float4 vecTime;
float fAmbient;
//bool PASS_SOLID;

texture entSkin1; // Model texture
texture highlights_tga_bmap;

sampler HighlightMapSampler = sampler_state 
{ 
   Texture = <highlights_tga_bmap>; 
   AddressU  = Clamp; 
   AddressV  = Clamp; 
}; 
sampler ColorMapSampler = sampler_state 
{ 
	Texture = <entSkin1>; 
	AddressU  = Wrap; 
	AddressV  = Wrap; 
}; 

void DiffuseVS( 
in float4 InPos: POSITION, 
in float3 InNormal: NORMAL, 
in float2 InTex: TEXCOORD0, 
in int inBoneIndices: BLENDINDICES, 
out float4 OutPos: POSITION, 
out float2 OutTex: TEXCOORD0, 
out float3 OutNormal: TEXCOORD1,
out float2 OutLookup: TEXCOORD2) 
{ 
	OutPos = mul(InPos, matWorldViewProj); 
	OutNormal = normalize(mul(InNormal, matWorld));
	OutTex.xy = InTex; 
	
	  OutLookup.xy = mul(OutNormal, matView).xy*0.2+float2(0.475,0.4);
} 

float4 DiffusePS( 
in float2 InTex: TEXCOORD0, 
in float3 InNormal: TEXCOORD1,
in float2 InLookup: TEXCOORD2) : COLOR0
{ 
	InNormal = normalize(InNormal);
	float3 sunDir = normalize(float3(1.25,-1,1));
	float Diffuse = saturate(dot(sunDir,-InNormal)*40)*0.3+0.75; //-InNormal.z+0.2; // length(InNormal.xy); //
	float4 Color = tex2D(ColorMapSampler, InTex);
	Color = Color*Color;
	float Highlight = tex2D(HighlightMapSampler, InLookup);
	Highlight = Highlight*0.4-0.04; //Highlight*Highlight*0.5+
	float4 final = Color*Diffuse+Highlight; //
	
	final = saturate(final);
	float greyscale = final.rgb*0.5;
	final = lerp(final,greyscale,vecSkill45.x);
	
	return final*0;
} 

// Technique: 
technique DiffuseTechnique 
{ 
	pass P0 
	{ 
		ZWriteEnable = true;
		AlphaBlendEnable=False;
		VertexShader = compile vs_3_0 DiffuseVS(); 
		PixelShader  = compile ps_3_0 DiffusePS(); 
	} 
} 
