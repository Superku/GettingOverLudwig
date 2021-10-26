const float4x4 matWorldViewProj;
const float4x4 matViewProj;
const float4x4 matWorld;
const float4x4 matView;
const float fAmbient;
float4 vecTime;
const float4 vecSunDir;
const float4 vecViewDir;
const float4 vecViewPos;
const float4 vecSkill41;
const float4 vecSkill45;

static const int boneShift[65] = { 8,40,4,1,56,18,55,26,43,24,63,17,12,50,44,30,46,27,36,13,20,6,37,21,57,42,60,51,41,25,23,39,53,47,10,49,28,22,62,9,61,59,45,19,11,34,52,54,64,35,15,31,5,16,58,48,38,7,0,33,14,3,2,29,32 };

texture highlights_tga_bmap;

sampler HighlightMapSampler = sampler_state 
{ 
	Texture = <highlights_tga_bmap>; 
	AddressU  = Clamp; 
	AddressV  = Clamp; 
}; 


texture entSkin1;
sampler MaskSampler = sampler_state 
{ 
	Texture = <entSkin1>; 
	AddressU  = Clamp; 
	AddressV  = Clamp; 
	minfilter = none;
	magfilter = none;
	mipfilter = none;
}; 

const float myPI = 3.1415926535;

void DiffuseVS( 
in float4 InPos: POSITION, 
in float3 InNormal: NORMAL, 
in float2 InTex: TEXCOORD0, 
in int inBoneIndices: BLENDINDICES, 
out float4 OutPos: POSITION, 
out float3 OutTex: TEXCOORD0, 
out float3 OutNormal: TEXCOORD1,
out float2 OutLookup: TEXCOORD2) 
{ 
	//InPos.xyz *= 0.5;
	//InPos.z += 20*saturate(1-saturate(InPos.y*0.1));
	int inBoneIndices2 = boneShift[inBoneIndices];
	float time = vecTime.w+inBoneIndices*77;
	time *= 3;

	float size = 1800;
	float fallDist = time*2+inBoneIndices2*50;
	float fallProgress = frac(fallDist/size);
	fallDist = fallProgress*size*2-size;
	float sizeFac = 0.5;

	InPos.xyz *= sizeFac+inBoneIndices2*0.01*sizeFac;

	float angle = vecTime.w*0.1+inBoneIndices*0.1;
	float cosa = cos(angle);
	float sina = sin(angle);
	float oldValue = InPos.x;
	InPos.x = cosa*InPos.x - sina*InPos.y;
	InPos.y = sina*oldValue + cosa*InPos.y;

	angle = sin(vecTime.w*0.07+inBoneIndices*0.1)*2;
	cosa = cos(angle);
	sina = sin(angle);
	oldValue = InPos.z;
	InPos.z = cosa*InPos.z - sina*InPos.y;
	InPos.y = sina*oldValue + cosa*InPos.y;


	float4 worldPos = mul(InPos, matWorld); 
	worldPos.z += inBoneIndices2-256;
	float xShift = sin(time*0.1+inBoneIndices);
	worldPos.x -= fallDist*3;
	//worldPos.x += (inBoneIndices*16-512)*(0.75+fallProgress*0.75)+xShift*(64+inBoneIndices*2);
	//fallDist -= xShift*xShift*90.5;
	//worldPos.y -= fallDist;
	fallProgress *= myPI*2;
	//worldPos.x += sin(fallProgress)*1000+sin(fallProgress*10+inBoneIndices2*0.2)*100;
	//worldPos.y += cos(fallProgress + inBoneIndices2*0)*545+500+sin(fallProgress*2+77)*200;
	float range = 340+inBoneIndices2*7;
	//worldPos.x += sin(fallProgress+ vecTime.w*0.01)*range+sin(fallProgress*10+inBoneIndices2*0.2)*100+20;
	worldPos.y += cos(fallProgress*2 + inBoneIndices2*0)*range + cos(fallProgress + inBoneIndices2)*200 + inBoneIndices2*30;
	worldPos.y -= cos(worldPos.x*0.00045+1.1)*1000;

	//worldPos.y += floor(inBoneIndices/4.0)*64-1000*0;
	
	OutPos = mul(worldPos, matViewProj); 
	
	OutNormal = normalize(mul(InNormal, matWorld));
	OutTex.xy = InTex; 
	OutTex.z = sin(inBoneIndices2)*0.5+0.5;  // brightness
	OutLookup = OutNormal*0.25+0.5;
} 

float4 DiffusePS( 
in float3 InTex: TEXCOORD0, 
in float3 InNormal: TEXCOORD1,
in float2 InLookup: TEXCOORD2): COLOR 
{ 
	//return 1;
	InNormal = normalize(InNormal);
	float3 InSunDir = -normalize(float3(4.75,-8,3));
	float Diffuse = 0.8+0.2*saturate(dot(InSunDir, InNormal)); 
	float Highlight = tex2D(HighlightMapSampler, InLookup);
	float4 Color = float4(0.6,0.2,0.1,1);
	Color = lerp(Color, float4(0.65,0.45,0.1,1), InTex.z);
	float4 final = Color*(Diffuse+Highlight*Highlight*0.5)-0.0985; //Diffuse
	float mask = tex2D(MaskSampler, InTex).a;
	final.a = mask;
	
	return final;
} 


technique DiffuseTechnique 
{ 
	pass P0 
	{ 
		cullmode = none;
		zWriteEnable = true;
		alphaBlendEnable = false;
		alphaTestEnable = true;
		//zWriteEnable = false;
		//alphaBlendEnable = true;
		VertexShader = compile vs_3_0 DiffuseVS(); 
		PixelShader  = compile ps_3_0 DiffusePS(); 
	} 
}
