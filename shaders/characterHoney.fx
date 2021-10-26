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

float4x3 matBones[72];
int iWeights;

float4 DoBones(float4 Pos,int4 BoneIndices,float4 BoneWeights)
{
	if(iWeights == 0) return Pos;
	float3 OutPos = 0;
	for(int i=0; i<iWeights; i++)
		OutPos += mul(Pos.xzyw,matBones[BoneIndices[i]])*BoneWeights[i];
	return float4(OutPos.xzy,1.0);
}

// only rotation and translation => inv(transpose(matWorld)) == matWorld
float3 DoBones(float3 Normal,int4 BoneIndices,float4 BoneWeights)
{
	if(iWeights == 0) return Normal;
	float3 OutNormal = 0;
	for(int i=0; i<iWeights; i++)
		OutNormal += mul(Normal.xzy,(float3x3)matBones[BoneIndices[i]])*BoneWeights[i];
	return normalize(OutNormal.xzy);
}


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
in float2 InTex2: TEXCOORD1,
in int4 inBoneIndices: BLENDINDICES,
in float4 inBoneWeights: BLENDWEIGHT,
out float4 OutPos: POSITION, 
out float2 OutTex: TEXCOORD0, 
out float3 OutNormal: TEXCOORD1,
out float2 OutLookup: TEXCOORD2) 
{ 
	float3 dressDir = float3(0,-27,-26) - InPos.xyz;

	float time = vecTime.w*0.2;
	float fallEffect = vecSkill41.x; // sin(time*0.925)*0.5+0.5;
	float dressEffect = saturate(InTex2.y);
	float wiggleEffect = pow(fallEffect,0.5)*0.8+0.2; // wind
	//float wiggleEffect = pow(fallEffect,0.5);
	float yShiftAmount = fallEffect*20 + sin(time*5 + InPos.x*0.2)*4*wiggleEffect;
	dressEffect = dressEffect*dressEffect;

	InPos.y += yShiftAmount*dressEffect;
	InPos.xz *= 1+dressEffect*fallEffect*0.5;

	float len = length(dressDir)*0.035;
	dressDir.y += 24;
	dressDir.z += 16;
	dressDir = normalize(dressDir);
	float pushFac = saturate(1-len*len);
	float fallEffectInv = 1-fallEffect;
	InPos.xyz += dressDir*dressEffect*2.25*pushFac*fallEffectInv; // push the dress close to the spear away to avoid clipping artifacts


	float4 bonePos = DoBones(InPos,inBoneIndices,inBoneWeights);
	float3 boneNormal = DoBones(InNormal,inBoneIndices,inBoneWeights);
	OutPos = mul(bonePos, matWorldViewProj); 
	OutNormal = normalize(mul(boneNormal, matWorld));
	OutTex.xy = InTex; 
	OutLookup.xy = OutNormal.xy*0.2+float2(0.475,0.4);
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
	
	return final;
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
