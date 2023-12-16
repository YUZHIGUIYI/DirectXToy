#ifndef _DEFERRED_REGISTERS_
#define _DEFERRED_REGISTERS_

Texture2D   gGeometryAlbedoMetalness : register(t4);
Texture2D   gGeometryNormalRoughness : register(t5);
Texture2D   gGeometryWorldPosition   : register(t6);
TextureCube gPrefilteredSpecularMap  : register(t7);
TextureCube gIrradianceMap           : register(t8);
Texture2D   gBRDFLUT                 : register(t9);

Texture2DArray gShadowMap            : register(t10);

#endif