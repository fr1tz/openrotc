// Copyright information can be found in the file named COPYING
// located in the root directory of this distribution.

$GameVersionString = "Pre-Alpha 0.1.1+dev";

// Load up core script base
loadDir("core"); // Should be loaded at a higher level, but for now leave -- SRZ 11/29/07

//-----------------------------------------------------------------------------
// Package overrides to initialize the mod.
package fps {

function displayHelp() {
   Parent::displayHelp();
   error(
      "Fps Mod options:\n"@
      "  -dedicated             Start as dedicated server\n"@
      "  -connect <address>     For non-dedicated: Connect to a game at <address>\n" @
      "  -mission <filename>    For dedicated: Load the mission\n"
   );
}

function parseArgs()
{
   // Call the parent
   Parent::parseArgs();

   // Arguments, which override everything else.
   for (%i = 1; %i < $Game::argc ; %i++)
   {
      %arg = $Game::argv[%i];
      %nextArg = $Game::argv[%i+1];
      %hasNextArg = $Game::argc - %i > 1;
   
      switch$ (%arg)
      {
         //--------------------
         case "-dedicated":
            $Server::Dedicated = true;
            enableWinConsole(true);
            $argUsed[%i]++;

         //--------------------
         case "-mission":
            $argUsed[%i]++;
            if (%hasNextArg) {
               $missionArg = %nextArg;
               $argUsed[%i+1]++;
               %i++;
            }
            else
               error("Error: Missing Command Line argument. Usage: -mission <filename>");

         //--------------------
         case "-connect":
            $argUsed[%i]++;
            if (%hasNextArg) {
               $JoinGameAddress = %nextArg;
               $argUsed[%i+1]++;
               %i++;
            }
            else
               error("Error: Missing Command Line argument. Usage: -connect <ip_address>");
      }
   }
}

function onStart()
{
   // The core does initialization which requires some of
   // the preferences to loaded... so do that first.  
   exec( "./client/defaults.cs" );
   exec( "./server/defaults.cs" );
             
   Parent::onStart();
   echo("\n--------- Initializing Directory: scripts ---------");

   // Load the scripts that start it all...
   exec("./client/init.cs");
   exec("./server/init.cs");
   
   // Init the physics plugin.
   physicsInit();
      
   // Start up the audio system.
   sfxStartup();

   // Server gets loaded for all sessions, since clients
   // can host in-game servers.
   initServer();

   // Start up in either client, or dedicated server mode
   if($Server::Dedicated)
      initDedicated();
   else
      initClient();
}

function onExit()
{
   // Ensure that we are disconnected and/or the server is destroyed.
   // This prevents crashes due to the SceneGraph being deleted before
   // the objects it contains.
   if ($Server::Dedicated)
      destroyServer();
   else
      disconnect();
   
   // Destroy the physics plugin.
   physicsDestroy();
      
   echo("Exporting client prefs");
   export("$pref::*", "./client/prefs.cs", False);

   echo("Exporting server prefs");
   export("$Pref::Server::*", "./server/prefs.cs", False);
   BanList::Export("./server/banlist.cs");

   Parent::onExit();
}

}; // package fps

// Activate the game package.
activatePackage(fps);
