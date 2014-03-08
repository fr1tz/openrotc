// Copyright information can be found in the file named COPYING
// located in the root directory of this distribution.

datablock LightDescription(WpnSMG1ProjectileLightDesc : BulletProjectileLightDesc)
{
   color = "0 1 0.952941 1";
   range = "10";
   brightness = "1";
};

datablock DecalData(WpnSMG1ProjectileDecal)
{
   Material = "xa_notc_core_shapes_smg1_projectile_p1_decalmat";
   size = "1";
   lifeSpan = "5000";
   randomize = "1";
   texRows = "4";
   texCols = "2";
   screenStartRadius = "20";
   screenEndRadius = "5";
   clippingAngle = "180";
   textureCoordCount = "7";
   textureCoords[0] = "0 0 0.25 0.5";
   textureCoords[1] = "0.25 0 0.25 0.5";
   textureCoords[2] = "0.5 0 0.25 0.5";
   textureCoords[3] = "0.75 0 0.25 0.5";
   textureCoords[4] = "0 0.5 0.25 0.5";
   textureCoords[5] = "0.25 0.5 0.25 0.5";
   textureCoords[6] = "0.5 0.5 0.25 0.5";
   textureCoords[7] = "0.75 0.5 0.25 0.5";
};

datablock ShotgunProjectileData(WpnSMG1Projectile)
{
   projectileShapeName = "content/xa/notc/core/shapes/smg1/projectile/p1/shape.dae";

   // ShotgunProjectileData fields
	numBullets = 1;
	range = 100;
	muzzleSpreadRadius = 0.0;
	referenceSpreadRadius = 0.0;
	referenceSpreadDistance = 50;

   //lightDesc = BulletProjectileLightDesc;

   directDamage        = 15;
   radiusDamage        = 0;
   damageRadius        = 0.5;
   areaImpulse         = 0.5;
   impactForce         = 600;

   explosion           = "WpnSMG1ProjectileExplosion";
   decal               = "WpnSMG1ProjectileDecal";

   muzzleVelocity      = 5;
   velInheritFactor    = 0;

   armingDelay         = 0;
   lifetime            = 992;
   fadeDelay           = 1472;
   bounceElasticity    = 0;
   bounceFriction      = 0;
   isBallistic         = false;
   gravityMod          = 1;
   //lightDesc = "WpnSMG1ProjectileLightDesc";
};

function WpnSMG1Projectile::onCollision(%this,%obj,%col,%fade,%pos,%normal)
{
   if(!(%col.getType() & $TypeMasks::GameBaseObjectType))
      return;

   %effectiveRange = 25;
   %dist = VectorLen(VectorSub(%pos, %obj.initialPosition));
   %distFactor = 1;
   if(%dist > %effectiveRange)
      %distFactor = 1 - (%dist-%effectiveRange) / (%this.range-%effectiveRange);
      
   error(%dist SPC %distFactor);
   %damage = %this.directDamage * %distFactor;

   %col.damage(%obj,%pos,%this.directDamage,"BulletProjectile");
}

