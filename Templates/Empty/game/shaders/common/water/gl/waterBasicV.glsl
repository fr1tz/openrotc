//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "../../gl/hlslCompat.glsl"

//-----------------------------------------------------------------------------
// Defines                                                                  
//-----------------------------------------------------------------------------

// TexCoord 0 and 1 (xy,zw) for ripple texture lookup
varying vec4 rippleTexCoord01;
#define OUT_rippleTexCoord01 rippleTexCoord01

// TexCoord 2 for ripple texture lookup
varying vec2 rippleTexCoord2;
#define OUT_rippleTexCoord2 rippleTexCoord2

// Screenspace vert position BEFORE wave transformation
varying vec4 posPreWave;
#define OUT_posPreWave posPreWave

// Screenspace vert position AFTER wave transformation
varying vec4 posPostWave;
#define OUT_posPostWave posPostWave 

// Worldspace unit distance/depth of this vertex/pixel
varying float  pixelDist;
#define OUT_pixelDist pixelDist

varying vec4 objPos;
#define OUT_objPos objPos

varying vec3 misc;
#define OUT_misc misc

//-----------------------------------------------------------------------------
// Uniforms                                                                  
//-----------------------------------------------------------------------------
uniform float4x4 modelMat;
uniform float4x4 modelview;
uniform float4   rippleMat[3];
uniform float3   eyePos;       
uniform float2   waveDir[3];
uniform float2   waveData[3];
uniform float2   rippleDir[3];
uniform float2   rippleTexScale[3];                    
uniform float3   rippleSpeed;
uniform float3   inLightVec;
uniform float3   reflectNormal;
uniform float    gridElementSize;
uniform float    elapsedTime;
uniform float    undulateMaxDist;

//-----------------------------------------------------------------------------
// Main                                                                        
//-----------------------------------------------------------------------------
void main()
{	
   vec4 IN_position = gl_Vertex;
   vec3 IN_normal = gl_Normal;
   vec2 IN_undulateData = gl_MultiTexCoord0.st;
   vec4 IN_horizonFactor = gl_MultiTexCoord1;
   vec4 OUT_hpos = vec4(0);
   
   // use projection matrix for reflection / refraction texture coords
   mat4 texGen = mat4FromRow( 0.5,  0.0,  0.0,  0.5,
                              0.0, -0.5,  0.0,  0.5,
                              0.0,  0.0,  1.0,  0.0,
                              0.0,  0.0,  0.0,  1.0 );

   // Move the vertex based on the horizonFactor if specified to do so for this vert.
   // if ( IN_horizonFactor.z > 0 )
   // {
      // float2 offsetXY = eyePos.xy - eyePos.xy % gridElementSize;         
      // IN_position.xy += offsetXY;
      // IN_undulateData += offsetXY; 
   // }         
   
   float4 worldPos = mul( modelMat, IN_position );
      
   IN_position.z = lerp( IN_position.z, eyePos.z, IN_horizonFactor.x );
   
   //OUT_objPos = worldPos;
   OUT_objPos.xyz = IN_position.xyz;
   OUT_objPos.w = worldPos.z;
   
   // Send pre-undulation screenspace position
   OUT_posPreWave = mul( modelview, IN_position );
   OUT_posPreWave = mul( texGen, OUT_posPreWave );
      
   // Calculate the undulation amount for this vertex.   
   float2 undulatePos = mul( modelMat, float4( IN_undulateData.xy, 0, 1 ) ).xy;
   //if ( undulatePos.x < 0 )
   //   undulatePos = IN_position.xy;
   
   float undulateAmt = 0.0;  

   undulateAmt += waveData[0].y * sin( elapsedTime * waveData[0].x + 
                                       undulatePos.x * waveDir[0].x +
                                       undulatePos.y * waveDir[0].y );
   undulateAmt += waveData[1].y * sin( elapsedTime * waveData[1].x + 
                                       undulatePos.x * waveDir[1].x +
                                       undulatePos.y * waveDir[1].y );
   undulateAmt += waveData[2].y * sin( elapsedTime * waveData[2].x + 
                                       undulatePos.x * waveDir[2].x +
                                       undulatePos.y * waveDir[2].y ); 
   
   float undulateFade = 1;
   
   // Scale down wave magnitude amount based on distance from the camera.      
   float dist = distance( IN_position.xyz, eyePos );
   dist = clamp( dist, 1.0, undulateMaxDist );          
   undulateFade *= ( 1 - dist / undulateMaxDist ); 
   
   // Also scale down wave magnitude if the camera is very very close.
   undulateFade *= saturate( ( distance( IN_position.xyz, eyePos ) - 0.5 ) / 10.0 );
   
   undulateAmt *= undulateFade;
   
   //#endif
   //undulateAmt = 0;
   
   // Apply wave undulation to the vertex.
   OUT_posPostWave = IN_position; 
   OUT_posPostWave.xyz += IN_normal.xyz * undulateAmt;   
   
   // Save worldSpace position of this pixel/vert
   //OUT_worldPos = OUT_posPostWave.xyz;   
   //OUT_worldPos = mul( modelMat, OUT_posPostWave.xyz );   
   //OUT_worldPos.z += objTrans[2][2]; //91.16;
   
   // OUT_misc.w = mul( modelMat, OUT_fogPos ).z;
   // if ( IN_horizonFactor.x > 0 )
   // {
      // float3 awayVec = normalize( OUT_fogPos.xyz - eyePos );
      // OUT_fogPos.xy += awayVec.xy * 1000.0;
   // }
   
   // Convert to screen 
   OUT_posPostWave = mul( modelview, OUT_posPostWave ); // mul( modelview, float4( OUT_posPostWave.xyz, 1 ) );     
   
   // Setup the OUT position symantic variable
   OUT_hpos = OUT_posPostWave; // mul( modelview, float4( IN_position.xyz, 1 ) ); //float4( OUT_posPostWave.xyz, 1 );   
   //OUT_hpos.z = lerp( OUT_hpos.z, OUT_hpos.w, IN_horizonFactor.x );
   
   // Save world space camera dist/depth of the outgoing pixel
   OUT_pixelDist = OUT_hpos.z;              

   // Convert to reflection texture space   
   OUT_posPostWave = mul( texGen, OUT_posPostWave );
        
   float2 txPos = undulatePos;
   if ( bool(IN_horizonFactor.x) )
      txPos = normalize( txPos ) * 50000.0;
      
   // set up tex coordinates for the 3 interacting normal maps   
   OUT_rippleTexCoord01.xy = txPos * rippleTexScale[0];
   OUT_rippleTexCoord01.xy += rippleDir[0] * elapsedTime * rippleSpeed.x;
            
   float2x2 texMat;   
   texMat[0][0] = rippleMat[0].x;
   texMat[1][0] = rippleMat[0].y;
   texMat[0][1] = rippleMat[0].z;
   texMat[1][1] = rippleMat[0].w;
   OUT_rippleTexCoord01.xy = mul( texMat, OUT_rippleTexCoord01.xy );      

   OUT_rippleTexCoord01.zw = txPos * rippleTexScale[1];
   OUT_rippleTexCoord01.zw += rippleDir[1] * elapsedTime * rippleSpeed.y;
   
   texMat[0][0] = rippleMat[1].x;
   texMat[1][0] = rippleMat[1].y;
   texMat[0][1] = rippleMat[1].z;
   texMat[1][1] = rippleMat[1].w;
   OUT_rippleTexCoord01.zw = mul( texMat, OUT_rippleTexCoord01.zw );         

   OUT_rippleTexCoord2.xy = txPos * rippleTexScale[2];
   OUT_rippleTexCoord2.xy += rippleDir[2] * elapsedTime * rippleSpeed.z; 
   
   texMat[0][0] = rippleMat[2].x;
   texMat[1][0] = rippleMat[2].y;
   texMat[0][1] = rippleMat[2].z;
   texMat[1][1] = rippleMat[2].w;
   OUT_rippleTexCoord2.xy = mul( texMat, OUT_rippleTexCoord2.xy );   

#ifdef WATER_SPEC
   
   float3 binormal = float3( 1, 0, 0 );
   float3 tangent = float3( 0, 1, 0 );
   float3 normal;
   for ( int i = 0; i < 3; i++ )
   {
      binormal.z += undulateFade * waveDir[i].x * waveData[i].y * cos( waveDir[i].x * IN_undulateData.x + waveDir[i].y * IN_undulateData.y + elapsedTime * waveData[i].x );
	  tangent.z += undulateFade * waveDir[i].y * waveData[i].y * cos( waveDir[i].x * IN_undulateData.x + waveDir[i].y * IN_undulateData.y + elapsedTime * waveData[i].x );
   } 
      
   binormal = normalize( binormal );
   tangent = normalize( tangent );
   normal = cross( binormal, tangent );
   
   float3x3 worldToTangent;
   worldToTangent[0] = binormal;
   worldToTangent[1] = tangent;
   worldToTangent[2] = normal;
   
   worldToTangent = transpose(worldToTangent);
      
   OUT_misc.xyz = mul( inLightVec, modelMat );
   OUT_misc.xyz = mul( worldToTangent, OUT_misc.xyz );   
   
#else

   OUT_misc.xyz = inLightVec;
   
#endif
   
   gl_Position = OUT_hpos;
   correctSSP(gl_Position);
}

