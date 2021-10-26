const float4x4 matWorldViewProj;
const float4x4 matWorld;
float4 vecSkill41;
float4 vecTime;

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
   out float3 OutTex: TEXCOORD0) 
{ 
   float flareFac = step(InPos.z, -13);
   float fireFac = 1-flareFac;
   float fireTip = 1-InTex.y;
   //fireTip = fireTip*fireTip+fireTip;
   fireTip = fireTip*fireTip*2;
   InPos.xy *= 1+(1+sin(vecTime.w*0.2)*0.1)*flareFac;
   float time = vecTime.w*1.5;
   float xMoveAmount = (sin(time*0.3)*0.5 + sin(time*0.8)*0.1-1)*2;
   xMoveAmount *= fireTip*fireFac;
   InPos.x += xMoveAmount;
   InPos.y += xMoveAmount*(0.3 + sin(time*0.5)*0.2);
   /*OutPos = mul(float4(0,0,0,1), matWorldViewProj); 
   float fadeOut = 1.02-abs(OutPos.x);
   fadeOut = saturate(fadeOut*20);
   InPos.xy *= smoothstep(0,1,fadeOut*0.5+0.5+fireFac); works but doesn't look great*/
   OutPos = mul(InPos, matWorldViewProj); 
   //OutPos.z *= 0.9;
   OutTex.xy = InTex;
   float flareAlpha = 1 + sin(vecTime.w*0.237)*0.05; // + sin(vecTime.w)*0.025;
   OutTex.z = 1-0.5*flareFac*flareAlpha;
} 
    
float4 DiffusePS( 
   in float3 InTex: TEXCOORD0): COLOR 
{ 
   float4 Color = tex2D(ColorMapSampler, InTex); 
   Color.gb *= 0.85;
   Color.a = Color.a*Color.a*InTex.z;
  
   return Color; 
} 
 

technique DiffuseTechnique 
{ 
   pass P0 
   { 
      destBlend = one;
      VertexShader = compile vs_3_0 DiffuseVS(); 
      PixelShader  = compile ps_3_0 DiffusePS(); 
   } 
}
