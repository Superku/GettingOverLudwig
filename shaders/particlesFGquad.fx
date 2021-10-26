const float4x4 matWorldViewProj;
const float4x4 matWorld;
const float fAmbient;
float4 vecTime;
float4 vecSkill41;

texture entSkin1;
sampler ColorMapSampler = sampler_state 
{ 
   Texture = <entSkin1>; 
   AddressU  = Wrap; 
   AddressV  = Wrap; 
}; 
    
void DiffuseVS( 
   in float4 InPos: POSITION, 
   in float2 InTex: TEXCOORD0, 
   out float4 OutPos: POSITION, 
   out float4 OutTex: TEXCOORD0) 
{ 
   OutPos = mul(InPos, matWorldViewProj); 
   float2 worldPos = mul(InPos, matWorld).xy/512.0;
   OutTex.xy = worldPos*float2(1,-1);
   //OutTex.x += OutTex.y*0.5;
   OutTex.x += vecTime.w*0.075;
   OutTex.y -= vecTime.w*0.0025;
   OutTex.zw = worldPos.xy*0.25 + vecTime.w*0.005;
} 
    
float4 DiffusePS( 
   in float4 InTex: TEXCOORD0): COLOR 
{ 
   float2 worldPos = InTex.zw;
   InTex.y += sin(worldPos.x + worldPos.y*0.5)*2; // this needs to happen in the pixel shader sadly
   float4 Color = tex2D(ColorMapSampler, InTex); 
   Color.rgb *= float3(1,0.1,0.5);
   InTex.x += vecTime.w*0.03;
   InTex.y += -vecTime.w*0.01;
   Color += tex2D(ColorMapSampler, InTex*1.13);
   float4 final = Color;
   final.rgba *= 0.5;

   return final;
} 
 

technique DiffuseTechnique 
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 DiffuseVS(); 
      PixelShader  = compile ps_3_0 DiffusePS(); 
   } 
}
