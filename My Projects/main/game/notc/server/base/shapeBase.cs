// Copyright information can be found in the file named COPYING
// located in the root directory of this distribution.

// This file contains ShapeBase methods used by all the derived classes

//-----------------------------------------------------------------------------
// ShapeBase object
//-----------------------------------------------------------------------------

// A raycast helper function to keep from having to duplicate code everytime
// that a raycast is needed.
//  %this = the object doing the cast, usually a player
//  %range = range to search
//  %mask = what to look for

function ShapeBase::doRaycast(%this, %range, %mask)
{
   // get the eye vector and eye transform of the player
   %eyeVec = %this.getEyeVector();
   %eyeTrans = %this.getEyeTransform();

   // extract the position of the player's camera from the eye transform (first 3 words)
   %eyePos = getWord(%eyeTrans, 0) SPC getWord(%eyeTrans, 1) SPC getWord(%eyeTrans, 2);

   // normalize the eye vector
   %nEyeVec = VectorNormalize(%eyeVec);

   // scale (lengthen) the normalized eye vector according to the search range
   %scEyeVec = VectorScale(%nEyeVec, %range);

   // add the scaled & normalized eye vector to the position of the camera
   %eyeEnd = VectorAdd(%eyePos, %scEyeVec);

   // see if anything gets hit
   %searchResult = containerRayCast(%eyePos, %eyeEnd, %mask, %this);

   return %searchResult;
}

//-----------------------------------------------------------------------------

function ShapeBase::damage(%this, %sourceObject, %position, %damage, %damageType)
{
   // All damage applied by one object to another should go through this method.
   // This function is provided to allow objects some chance of overriding or
   // processing damage values and types.  As opposed to having weapons call
   // ShapeBase::applyDamage directly. Damage is redirected to the datablock,
   // this is standard procedure for many built in callbacks.

   if (isObject(%this))
      %this.getDataBlock().damage(%this, %sourceObject, %position, %damage, %damageType);
}

//-----------------------------------------------------------------------------

function ShapeBase::setDamageDt(%this, %damageAmount, %damageType)
{
   // This function is used to apply damage over time.  The damage is applied
   // at a fixed rate (50 ms).  Damage could be applied over time using the
   // built in ShapBase C++ repair functions (using a neg. repair), but this
   // has the advantage of going through the normal script channels.

   if (%this.getState() !$= "Dead")
   {
      %this.damage(0, "0 0 0", %damageAmount, %damageType);
      %this.damageSchedule = %this.schedule(50, "setDamageDt", %damageAmount, %damageType);
   }
   else
      %this.damageSchedule = "";
}

function ShapeBase::clearDamageDt(%this)
{
   if (%this.damageSchedule !$= "")
   {
      cancel(%this.damageSchedule);
      %this.damageSchedule = "";
   }
}

//-----------------------------------------------------------------------------

function ShapeBase::reloadWeapon(%this)
{
   %player = %this;
   %image = %player.getMountedImage($WeaponSlot);

   if(%image == 0)
      return;

   // Bail out if we're already reloading.
   if(%image.fireImage !$= "")
      return;

   if(%image.isField("ammo"))
   {
      if(%this.getInventory(%image.ammo) > 0)
         %this.mountImage(%image.reloadImage, $WeaponSlot);
   }
   else if(%image.isField("clip"))
   {
      if(%this.getInventory(%image.clip) > 0)
         %this.mountImage(%image.reloadImage, $WeaponSlot);
   }
}

//-----------------------------------------------------------------------------
// ShapeBase datablock
//-----------------------------------------------------------------------------

function ShapeBaseData::damage(%this, %obj, %source, %position, %amount, %damageType)
{
   %obj.applyDamage(%amount);
   
   %z = getWord(%obj.getVelocity(), 2);
   %obj.setVelocity("0" SPC "0" SPC %z);
   

   %bleed = %this.getBleed(%obj, %delta);
   if(isObject(%bleed))
   {
      %dpos = %position;
      %spos = %source.getPosition();
      %norm = VectorNormalize(VectorSub(%dpos, %obj.getWorldBoxCenter()));
      if(getWord(%norm, 2) < 0)
         %norm = VectorNormalize(VectorSub(%spos, %dpos));
      createExplosion(%bleed, %dpos, %norm);
   }
}

// Called by engine whenever the object's damage level changes.
function ShapeBaseData::onDamage(%this, %obj, %delta)
{
   // Avoid console error spam.
}
