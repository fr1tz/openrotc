// Copyright information can be found in the file named COPYING
// located in the root directory of this distribution.

datablock DecalData(WpnSR2ProjectileDecal)
{
   Material = "xa_notc_core_shapes_sr2_decal_p1_mat0";
   size = "3";
   lifeSpan = "0";
   randomize = "0";
   texRows = "1";
   texCols = "1";
   screenStartRadius = "20";
   screenEndRadius = "5";
   clippingAngle = "180";
   textureCoordCount = "0";
   textureCoords[0] = "0 0 1 1";
   textureCoords[1] = "0 0 1 1";
   textureCoords[2] = "0 0 1 1";
   textureCoords[3] = "0 0 1 1";
   textureCoords[4] = "0 0 1 1";
   textureCoords[5] = "0 0 1 1";
   textureCoords[6] = "0 0 1 1";
   textureCoords[7] = "0 0 1 1";
   textureCoords[8] = "0 0 1 1";
   textureCoords[9] = "0 0 1 1";
   textureCoords[10] = "0 0 1 1";
   fadeTime = "6000";
   paletteSlot = "0";
};

datablock ExplosionData(WpnSR2ProjectileExplosion)
{
	soundProfile = WpnSR2ProjectileImpactSound;

	// Shape
	explosionShape = "content/xa/notc/core/shapes/sr2/explosion/p1/shape.dts";
	faceViewer = false;
	playSpeed = 1.0;
	sizes[0] = "0.1 0.1 0.1";
	sizes[1] = "0.5 0.5 0.5";
	times[0] = 0.0;
	times[1] = 1.0;

	// Dynamic light
	lightStartRadius = 0;
	lightEndRadius = 0;
	lightStartColor = "0.984252 0.992126 0.992126 1";
	lightEndColor = "0.984252 0.984252 0.984252 1";
   lightStartBrightness = "16.0784";
   lightEndBrightness = "16.1569";
};

datablock ExplosionData(WpnSR2ProjectileMissedEnemyEffect)
{
	soundProfile = WpnSR2ProjectileMissedEnemySound;
};

datablock MultiNodeLaserBeamData(WpnSR2ProjectileLaserTrail0)
{
	material = "xa_notc_core_shapes_sr2_trail_p1_mat1";
	renderMode = $MultiNodeLaserBeamRenderMode::FaceViewer;
   width = 0.3;
	fadeTime = 100;
	windCoefficient = 0.0;
	//nodeDistance = 3;

   // node x movement...
   nodeMoveMode[0]     = $NodeMoveMode::None;
   nodeMoveSpeed[0]    = -0.002;
   nodeMoveSpeedAdd[0] =  0.004;
   // node y movement...
   nodeMoveMode[1]     = $NodeMoveMode::None;
   nodeMoveSpeed[1]    = -0.002;
   nodeMoveSpeedAdd[1] =  0.004;
   // node z movement...
   nodeMoveMode[2]     = $NodeMoveMode::None;
   nodeMoveSpeed[2]    = 0.5;
   nodeMoveSpeedAdd[2] = 1.0;
};

datablock MultiNodeLaserBeamData(WpnSR2ProjectileLaserTrail1)
{
	material[0] = "xa_notc_core_shapes_sr2_trail_p1_mat0";
   width[0] = 0.6;
	material[1] = "xa_notc_core_shapes_sr2_trail_p1_mat2";
   width[1] = 0.6;
 
	renderMode = $MultiNodeLaserBeamRenderMode::FaceViewer;

	fadeTime = 200;
	windCoefficient = 0.0;
	//nodeDistance = 3;

   // node x movement...
   nodeMoveMode[0]     = $NodeMoveMode::None;
   nodeMoveSpeed[0]    = -0.002;
   nodeMoveSpeedAdd[0] =  0.004;
   // node y movement...
   nodeMoveMode[1]     = $NodeMoveMode::None;
   nodeMoveSpeed[1]    = -0.002;
   nodeMoveSpeedAdd[1] =  0.004;
   // node z movement...
   nodeMoveMode[2]     = $NodeMoveMode::None;
   nodeMoveSpeed[2]    = 0.5;
   nodeMoveSpeedAdd[2] = 1.0;
};

datablock MultiNodeLaserBeamData(WpnSR2ProjectileLaserTrail2)
{
	material = "xa_notc_core_shapes_sr2_trail_p1_mat3";
	renderMode = $MultiNodeLaserBeamRenderMode::FaceViewer;
   width = 1.5;
	fadeTime = 3000;
	windCoefficient = 0.0;
	nodeDistance = 4;

   // Node x movement
   nodeMoveMode[0]     = $NodeMoveMode::ConstantSpeed;
   nodeMoveSpeed[0]    = -2.0;
   nodeMoveSpeedAdd[0] =  4.0;
   // Node y movement
   nodeMoveMode[1]     = $NodeMoveMode::ConstantSpeed;
   nodeMoveSpeed[1]    = -2.0;
   nodeMoveSpeedAdd[1] =  4.0;
   // Node z movement
   nodeMoveMode[2]     = $NodeMoveMode::ConstantSpeed;
   nodeMoveSpeed[2]    = -2.0;
   nodeMoveSpeedAdd[2] =  4.0;
};

