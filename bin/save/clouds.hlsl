// migrated from iq's GLSL code
// Created by inigo quilez - iq/2013

cbuffer Parameters : register(b0)
{
  float4 time;
  float4 view;
  float4 freq;
  float4 mpos;
}

float fbm(float3 p)
{
    float f;
    f  = 0.5000 * qnoise(p); p = p * 2.02;
    f += 0.2500 * qnoise(p); p = p * 2.03;
    f += 0.1250 * qnoise(p); p = p * 2.01;
    f += 0.0625 * qnoise(p);
    return f;
}

float4 map(in float3 p)
{
  float d = 0.5 - p.y;
  d += 3.0 * fbm( p*1.0 - float3(1.0,0.1,0.0) * time.x);  
  float4 res = saturate(d);
  res.xyz = lerp(1.15*float3(1.0,0.95,0.8), float3(0.7,0.7,0.7), res.x);
  return res;
}

static float3 sundir = float3(-1,0,0);

float4 raymarch(in float3 ro, in float3 rd)
{
  float4 sum = 0;
  float t = 0.0;
  for(int i = 0; i < 44; i++)
  {
    float3 pos = ro + t*rd;
    float4 col = map( pos );
    float dif = saturate((col.w - map(pos+0.3*sundir).w)/0.6);
    float3 brdf = float3(0.65,0.68,0.7)*1.35 + 0.45*float3(0.7, 0.5, 0.3)*dif;
    
    col.xyz *= brdf;
    col.a *= 0.35;
    col.rgb *= col.a;

    sum = sum + col*(1.0 - sum.a);	
    t += max(0.1,0.05*t);
  }
  sum.xyz /= (0.001+sum.w);
  return saturate(sum);
}

float4 ps_main(in float2 tc : TEXCOORD) : SV_TARGET
{
  float2 p = tc * 2 - 1;
  p *= float2(view.x * view.w, -1);
  float2 mo = -1 + 2 * mpos.xy / view.xy;

  // camera
  float3 ro = 4.0*normalize(float3(cos(2.75-3.0*mo.x), 0.7+(mo.y+1.0), sin(2.75-3.0*mo.x)));
  float3 ta = float3(0.0, 1.0, 0.0);
  float3 ww = normalize( ta - ro);
  float3 uu = normalize(cross( float3(0.0,1.0,0.0), ww ));
  float3 vv = normalize(cross(ww,uu));
  float3 rd = normalize( p.x*uu + p.y*vv + 1.5*ww );

  float4 res = raymarch( ro, rd );
  float sun = saturate(dot(sundir,rd));
  float3 col = float3(0.6,0.71,0.75) - rd.y*0.2*float3(1.0,0.5,1.0) + 0.15*0.5;
  col += 0.2*float3(1.0,.6,0.1) * pow(sun, 8.0);
  col *= 0.95;
  col = lerp(col, res.xyz, res.w);
  col += 0.1*float3(1.0,0.4,0.2) * pow(sun, 2.0);

  return float4( col, 1.0 );
}
