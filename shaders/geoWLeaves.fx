const float4x4 matWorldViewProj;
const float4x4 matWorld;
const float4x4 matView;
const float fAmbient;
const float4 vecSunDir;
const float4 vecViewDir;
const float4 vecViewPos;
const float4 vecSkill41;
const float4 vecSkill45;
float3 playerPosLocal_var;
//bool PASS_SOLID;
float fAlbedo;

texture highlights_tga_bmap;
sampler HighlightMapSampler = sampler_state 
{ 
	Texture = <highlights_tga_bmap>; 
	AddressU  = Clamp; 
	AddressV  = Clamp; 
}; 

texture entSkin1;
sampler LookupMapSampler = sampler_state 
{ 
	Texture = <entSkin1>; 
	AddressU  = Mirror; 
	AddressV  = Mirror; 
}; 

texture autumnLeavesMask_tga_bmap;
sampler LeavesMapSampler = sampler_state 
{ 
	Texture = <autumnLeavesMask_tga_bmap>; 
	AddressU  = Wrap; 
	AddressV  = Wrap; 
}; 

void DiffuseVS( 
in float4 InPos: POSITION, 
in float3 InNormal: NORMAL, 
in float2 InTex: TEXCOORD0, 
in float2 InTex2: TEXCOORD1, 
out float4 OutPos: POSITION, 
out float4 OutTex: TEXCOORD0, 
out float3 OutNormal: TEXCOORD1,
out float2 OutLookup: TEXCOORD2,
out float4 OutWorldPos: TEXCOORD3,
out float3 OutLightDiff: TEXCOORD4,
out float4 OutLeaves: TEXCOORD5) 
{ 
	OutPos = mul(InPos, matWorldViewProj); 
	OutNormal = normalize(mul(InNormal, matWorld));
	OutTex.xy = InTex; 
	OutTex.zw = InTex2-1; 
	OutLookup = OutNormal; //mul((OutNormal), matView).xy;
	OutLookup = OutLookup*0.25+0.5;
	OutWorldPos = mul(InPos,matWorld);
	OutLeaves.xy = InPos.xy*0.0175;
	OutLeaves.zw = OutWorldPos.xy*0.0035+OutWorldPos.z*0.0002+ InNormal.x*0.1; // + InNormal.z*0.001;
	OutLeaves.w *= 2;
	OutLightDiff.xy = playerPosLocal_var.xz-OutWorldPos.xy;
	OutLightDiff.xy *= 0.002125;
	OutLightDiff.z = -0.125;
} 

float4 DiffusePS( 
in float4 InTex: TEXCOORD0, 
in float3 InNormal: TEXCOORD1,
in float2 InLookup: TEXCOORD2,
in float4 InWorldPos: TEXCOORD3,
in float3 InLightDiff: TEXCOORD4,
in float4 InLeaves: TEXCOORD5): COLOR 
{ 
	//return InTex.z;
	//
	//float leavesFac = (1+InNormal.z);
	float leavesFac = InTex.z; //InNormal.y*vecSkill41.y;
	float4 leavesTexture = tex2D(LeavesMapSampler, InLeaves.zw);
	//leavesTexture.g = leavesTexture.r;
	//return leavesTexture;

	//leavesFac = InTex.z*10 -leavesTexture.r-0.5;
	leavesFac = saturate(InTex.z*2 + leavesTexture.r -1);
	leavesFac = saturate(leavesFac*10-0.5);
	//return leavesFac;
	
	InNormal = normalize(InNormal);
	
	float3 InSunDir = -normalize(float3(4.75,-8,3));
	float preDiffuse = saturate(dot(InSunDir, InNormal)); 
	//return preDiffuse;
	float Diffuse = 0.6+0.4*preDiffuse;
	float3 R = 2*dot(InNormal, InSunDir) * InNormal - InSunDir; 
	R = normalize(R);
	float Specular = saturate(-R.z); //saturate(dot(R, InViewDir));
	Specular = pow(Specular, 32);
	
	
	float Highlight = tex2D(HighlightMapSampler, InLookup);  //InTex // InLookup
	//return Highlight;
	float4 Color = tex2D(LookupMapSampler, InTex.xy);  //InTex // InLookup
	Color.rg += InNormal.xy*0.1;
	float4 final = Color*(Diffuse+Specular*0.25+Highlight*Highlight*0.5)-0.0985; //Diffuse
	float colorFac = fAlbedo;
	final = lerp(Highlight, final, colorFac);


	//return leavesFac;
	float leavesDiffuse = 0.45+0.65*saturate(dot(normalize(float3(4.75,-5,0)), -InNormal)); 
	float4 leaves = float4(0.9,0.2+leavesTexture.g*0.3,0.075,1);
	//leaves = lerp(0.3, leaves, leavesTexture.r);
	leaves *= 0.5+leavesTexture.r*0.5;
	//float leavesBrightness = leavesTexture.r*0.25+leavesTexture.a*0.1+0.65;
	leaves = leaves*leavesDiffuse; //*leavesBrightness;
	final = lerp(final, leaves, leavesFac);








	float len = saturate(1-length(InLightDiff.xy));
	float playerLightFac = len*len;
	//InLightDiff.xy = normalize(InLightDiff.xy)*0.5;
	//InLightDiff /= len;
	//InLightDiff.z += len*0.1;
	InLightDiff = normalize(InLightDiff);
	float playerDiffuse = saturate(dot(InLightDiff, InNormal)); 
	playerLightFac = playerDiffuse*playerLightFac;
	//float greyFac2Inv = 1-leavesFac*0.6;
	final += playerLightFac*0.5; //*greyFac2Inv;
	final *= fAmbient+1;
	return final;
} 


technique DiffuseTechnique 
{ 
	pass P0 
	{ 
		zWriteEnable = true;
		alphaBlendEnable = false;
		VertexShader = compile vs_3_0 DiffuseVS(); 
		PixelShader  = compile ps_3_0 DiffusePS(); 
	} 
}
