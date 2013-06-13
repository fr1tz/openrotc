// Copyright information can be found in the file named COPYING
// located in the root directory of this distribution.

#include "platform/platform.h"
#include "util/messaging/eventManager.h"

#include "console/consoleTypes.h"
#include "console/consoleInternal.h"

IMPLEMENT_CONOBJECT( EventManager );

ConsoleDocClass( EventManager,
   "@brief The EventManager class is a wrapper for the standard messaging system.\n\n"

   "It provides functionality for management of event queues, events, and "
   "subscriptions. Creating an EventManager is as simple as calling new EventManager "
   "and specifying a queue name.\n\n"

   "@tsexample\n"
  "// Create the EventManager.\n"
  "$MyEventManager = new EventManager() { queue = \"MyEventManager\"; };\n\n" 
  "// Create an event.\n" 
  "$MyEventManager.registerEvent( \"SomeCoolEvent\" );\n\n"  
  "// Create a listener and subscribe.\n" 
  "$MyListener = new ScriptMsgListener() { class = MyListener; };\n" 
  "$MyEventManager.subscribe( $MyListener, \"SomeCoolEvent\" );\n\n"  
  "function MyListener::onSomeCoolEvent( %this, %data )\n" 
  "{\n" 
  "	  echo( \"onSomeCoolEvent Triggered\" );\n" 
  "}\n\n"  
  "// Trigger the event.\n" 
  "$MyEventManager.postEvent( \"SomeCoolEvent\", \"Data\" );\n" 
   "@endtsexample\n\n"

   "@see ScriptMsgListener\n\n"

   "@ingroup Messaging\n"
);

Vector<EventManager*> EventManager::smEventManagers;

//-----------------------------------------------------------------------------
/// Gets a list of all listeners of a specific event type and executes a
/// callback on each one.
/// 
/// @param event The name of the event that was triggered.
/// @param data The data associated with the event.
/// @return true to allow other listeners to receive the event, false otherwise
//-----------------------------------------------------------------------------

// CodeReview [tom, 2/20/2007] There seemed to be a little confusion on the return value here.
// It is not a "successfully dispatched" value, it is used to prevent other
// listeners from receiving the message. Using the event manager this probably
// didn't matter since there was only one listener, however it would cause
// problems if more then one listener is registered with the queue.
bool EventManagerListener::onMessageReceived( StringTableEntry queue, const char* event, const char* data )
{
   Vector<Subscriber>* subscribers = mSubscribers.retreive( event );
   if( subscribers == NULL )
      return true;

   for( Vector<Subscriber>::iterator iter = subscribers->begin(); iter != subscribers->end(); iter++ )
   {
      if( iter->listener == NULL )
      {
         subscribers->erase_fast( iter -- );
         continue;
      }

      if (!iter->removeFlag)
	   {
		   iter->callDepth++;
		   Con::executef( iter->listener, iter->callback, data );
		   iter->callDepth--;
		   if (iter->removeFlag && iter->callDepth==0)
		   {
			   subscribers->erase_fast(iter--);
		   }
	   }
   }

   return true;
}

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
EventManager::EventManager() : mQueue( NULL )
{
   addEventManager( this );
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
EventManager::~EventManager()
{
   setMessageQueue( "" );
   unregisterAllEvents();
   removeEventManager( this );
}

//-----------------------------------------------------------------------------
// initPersistFields
//-----------------------------------------------------------------------------
void EventManager::initPersistFields()
{
   addProtectedField( "queue", TypeString, Offset( mQueue, EventManager ), &_setMessageQueue, &defaultProtectedGetFn, "List of events currently waiting" );
}

//-----------------------------------------------------------------------------
/// Registers the message queue and listener with the messaging system.
/// 
/// @param queue The name of the queue. Set to "" to destroy the queue.
//-----------------------------------------------------------------------------
void EventManager::setMessageQueue( const char* queue )
{
   // If a queue is already registered, unregister it.
   if( mQueue && Dispatcher::isQueueRegistered( mQueue ) )
   {
      unregisterAllEvents();
      Dispatcher::unregisterMessageListener( mQueue, &mListener );
      Dispatcher::unregisterMessageQueue( mQueue );
   }

   // Register the new queue.
   if( queue && *queue )
   {
      Dispatcher::registerMessageQueue( queue );
      Dispatcher::registerMessageListener( queue, &mListener );
      mQueue = StringTable->insert( queue );
   }
}

//-----------------------------------------------------------------------------
/// Determines whether or not an event is registered with the EventManager.
/// 
/// @param event the event to check.
//-----------------------------------------------------------------------------
bool EventManager::isRegisteredEvent( const char* event )
{
   // Iterate over the event list.
   StringTableEntry eventName = StringTable->insert( event );
   for( Vector<StringTableEntry>::const_iterator iter = mEvents.begin(); iter != mEvents.end(); iter++ )
   {
      // Found.
      if( *iter == eventName )
         return true;
   }

   // Not found.
   return false;
}

//-----------------------------------------------------------------------------
/// Register an event with the EventManager.
/// 
/// @param event The event to register.
/// @return Whether or not the event was successfully registered.
//-----------------------------------------------------------------------------
bool EventManager::registerEvent( const char* event )
{
   // Make sure the event has not been registered yet.
   if( isRegisteredEvent( event ) )
   {
      Con::warnf( "EventManager::registerEvent - event %s already registered", event );
      return false;
   }

   // Add to the event list.
   mEvents.push_back( StringTable->insert( event ) );

   // Create a list of subscribers for this event.
   mListener.mSubscribers.insert( new Vector<EventManagerListener::Subscriber>, event );

   return true;
}

//-----------------------------------------------------------------------------
/// Removes all events from the EventManager.
//-----------------------------------------------------------------------------
void EventManager::unregisterAllEvents()
{
   // Iterate over all events.
   for( Vector<StringTableEntry>::const_iterator iter = mEvents.begin(); iter != mEvents.end(); iter++ )
   {
      // Delete the subscriber list.
      Vector<EventManagerListener::Subscriber>* subscribers = mListener.mSubscribers.remove( *iter );
      if( subscribers )
         delete subscribers;
   }

   // Clear the event list.
   mEvents.clear();
}

//-----------------------------------------------------------------------------
/// Removes an event from the EventManager.
/// 
/// @param event The event to remove.
//-----------------------------------------------------------------------------
void EventManager::unregisterEvent( const char* event )
{
   // If the event doesn't exist, we have succeeded in removing it!
   if( !isRegisteredEvent( event ) )
      return;

   // Iterate over all events.
   StringTableEntry eventName = StringTable->insert( event );
   for( Vector<StringTableEntry>::iterator iter = mEvents.begin(); iter != mEvents.end(); iter++ )
   {
      // Erase the event.
      if( *iter == eventName )
      {
         mEvents.erase_fast( iter );
         break;
      }
   }

   // Delete the subscriber list.
   Vector<EventManagerListener::Subscriber>* subscribers = mListener.mSubscribers.remove( event );
   if( subscribers )
      delete subscribers;
}

//-----------------------------------------------------------------------------
/// Post an event to the EventManager's queue.
/// 
/// @param event The event to post.
/// @param data Various data associated with the event.
/// @return Whether or not the message was dispatched successfully.
//-----------------------------------------------------------------------------
bool EventManager::postEvent( const char* event, const char* data )
{
   AssertFatal( mQueue != NULL, "EventManager::postEvent - Queue not initialized" );
   return Dispatcher::dispatchMessage( mQueue, event, data );
}

//-----------------------------------------------------------------------------
/// Subscribe a listener to an event.
/// 
/// @param listener The listener to subscribe.
/// @param event The event to subscribe to.
/// @param callback Optional callback name to be called when the event is
/// triggered.
/// @return Whether or not the subscription was successful.
//-----------------------------------------------------------------------------

// CodeReview [tom, 2/20/2007] The "listener" argument was an IMessageListener,
// but it was actually used as a SimObject and never a listener. Thus, it is now a SimObject.
bool EventManager::subscribe(SimObject *callbackObj, const char* event, const char* callback /*= NULL */)
{
   // Make sure the event is valid.
   if( !isRegisteredEvent( event ) )
   {
      Con::warnf( "EventManager::subscribe - %s is not a registered event.", event );
      return false;
   }

   // Grab the callback name.
   char* cb = NULL;
   if( !callback || !*callback )
   {
      // Not specified, use default ( "onEvent" ).
      S32 length = dStrlen( event ) + 5;
      cb = new char[length];
      dSprintf( cb, length, "on%s", event );
   }
   else
   {
      cb = new char[dStrlen(callback) + 1];
      dStrcpy(cb, callback);
   }

   // Create the subscriber object.
   EventManagerListener::Subscriber subscriber;
   subscriber.listener = callbackObj;
   subscriber.event = StringTable->insert( event );
   subscriber.callback = StringTable->insert( cb );
   subscriber.callDepth = 0;
   subscriber.removeFlag = false;

   delete [] cb;

   // Grab the subscriber list.
   Vector<EventManagerListener::Subscriber>* subscribers = mListener.mSubscribers.retreive( event );

   // If the event exists, there should always be a valid subscriber list.
   AssertFatal( subscribers, "Invalid event subscriber list." );

   // Add the subscriber.
   subscribers->push_back( subscriber );

   return true;
}


//-----------------------------------------------------------------------------
/// remove a listener from an event.
/// 
/// @param listener The listener to remove from an event callback list.
/// @param event The event to remove the listener from.
//-----------------------------------------------------------------------------

// CodeReview [tom, 2/20/2007] The "listener" argument was an IMessageListener,
// but it was actually used as a SimObject and never a listener. Thus, it is now a SimObject.
void EventManager::remove(SimObject *cbObj, const char* event)
{
   // If the event doesn't exist, we have succeeded in removing it!
   if( !isRegisteredEvent( event ) )
      return;

   Vector<EventManagerListener::Subscriber>* subscribers = mListener.mSubscribers.retreive( event );
   if( !subscribers )
      return;

   for( Vector<EventManagerListener::Subscriber>::iterator iter = subscribers->begin(); iter != subscribers->end(); iter++ )
   {
      // Erase the event.
      if( iter->listener == cbObj )
      {
		  if (iter->callDepth > 0)
			  iter->removeFlag = true;
		  else
			  subscribers->erase_fast( iter );
         break;
      }
   }
}

void EventManager::removeAll(SimObject *cbObj)
{
   // Iterate over all events.
   for( Vector<StringTableEntry>::const_iterator iter1 = mEvents.begin(); iter1 != mEvents.end(); iter1++ )
   {
		Vector<EventManagerListener::Subscriber>* subscribers = mListener.mSubscribers.retreive( *iter1 );
	   if( !subscribers )
		  continue;
	   for( Vector<EventManagerListener::Subscriber>::iterator iter2 = subscribers->begin(); iter2 != subscribers->end(); iter2++ )
	   {
		  // Erase the event.
		  if( iter2->listener == cbObj )
		  {
			  if (iter2->callDepth > 0)
				  iter2->removeFlag = true;
			  else
				  subscribers->erase_fast( iter2 );
			 break;
		  }
	   }
   }

}
//-----------------------------------------------------------------------------
/// Print all registered events to the console.
//-----------------------------------------------------------------------------
void EventManager::dumpEvents()
{
   Con::printf( "%s Events", mQueue );
   for( Vector<StringTableEntry>::const_iterator iter = mEvents.begin(); iter != mEvents.end(); iter++ )
      Con::printf( "   %s", *iter );
}

//-----------------------------------------------------------------------------
/// Print the subscribers to an event.
/// 
/// @param event The event whose subscribers are to be printed.
//-----------------------------------------------------------------------------
void EventManager::dumpSubscribers( const char* event )
{
   Vector<EventManagerListener::Subscriber>* subscribers = mListener.mSubscribers.retreive( event );
   if( !subscribers )
   {
      Con::warnf( "EventManager::dumpSubscriber - %s is not a valid event.", event );
      return;
   }

   Con::printf( "%s Subscribers", event );
   for( Vector<EventManagerListener::Subscriber>::const_iterator iter = subscribers->begin(); iter != subscribers->end(); iter++ )
      if( iter->listener )
      {
         // Grab the best fit name. This should be the first found of name, class, superclass, or class type.
         Namespace* ns = iter->listener->getNamespace();
         const char* name = ns ? ns->mName : getClassName() ;
         Con::printf( "   %s -> %s", name, iter->callback );
      }
}

//-----------------------------------------------------------------------------
/// Print all registered events and their subscribers to the console.
//-----------------------------------------------------------------------------
void EventManager::dumpSubscribers()
{
   Con::printf( "%s Events", mQueue );
   for( Vector<StringTableEntry>::const_iterator iter = mEvents.begin(); iter != mEvents.end(); iter++ )
      dumpSubscribers( *iter );
}

//-----------------------------------------------------------------------------
// Console Methods
//-----------------------------------------------------------------------------
ConsoleMethod( EventManager, registerEvent, bool, 3, 3, "( String event )\n"
              "Register an event with the event manager.\n"
              "@param event The event to register.\n"
              "@return Whether or not the event was registered successfully." )
{
   return object->registerEvent( argv[2] );
}

ConsoleMethod( EventManager, unregisterEvent, void, 3, 3, "( String event )\n"
              "Remove an event from the EventManager.\n"
              "@param event The event to remove.\n" )
{
   object->unregisterEvent( argv[2] );
}

ConsoleMethod( EventManager, isRegisteredEvent, bool, 3, 3, "( String event )\n"
              "Check if an event is registered or not.\n"
              "@param event The event to check.\n"
              "@return Whether or not the event exists." )
{
   return object->isRegisteredEvent( argv[2] );
}

ConsoleMethod( EventManager, postEvent, bool, 3, 4, "( String event, String data )\n"
              "~Trigger an event.\n"
              "@param event The event to trigger.\n"
              "@param data The data associated with the event.\n"
              "@return Whether or not the event was dispatched successfully." )
{
   if( !object->getMessageQueue() || !object->getMessageQueue()[ 0 ] )
   {
      Con::errorf( "EventManager::postEvent - No queue name set on EventManager" );
      return false;
   }

   return object->postEvent( argv[2], argc > 3 ? argv[3] : "" );
}

ConsoleMethod( EventManager, subscribe, bool, 4, 5, "( SimObject listener, String event, String callback )\n\n"
              "Subscribe a listener to an event.\n"
              "@param listener The listener to subscribe.\n"
              "@param event The event to subscribe to.\n"
              "@param callback Optional method name to receive the event notification. If this is not specified, \"on[event]\" will be used.\n"
              "@return Whether or not the subscription was successful." )
{
   // Find the listener object.
   SimObject *cbObj = dynamic_cast<SimObject *>(Sim::findObject(argv[2]));
   if( cbObj == NULL )
   {
      Con::warnf( "EventManager::subscribe - Invalid listener." );
      return false;
   }

   return object->subscribe( cbObj, argv[3], argc > 4 ? argv[4] : NULL );
}

ConsoleMethod( EventManager, remove, void, 4, 4, "( SimObject listener, String event )\n\n"
              "Remove a listener from an event.\n"
              "@param listener The listener to remove.\n"
              "@param event The event to be removed from.\n")
{
   // Find the listener object.
   SimObject * listener = dynamic_cast< SimObject * >( Sim::findObject( argv[2] ) );
   if( listener )
      object->remove( listener, argv[3] );
}

ConsoleMethod( EventManager, removeAll, void, 3, 3, "( SimObject listener )\n\n"
              "Remove a listener from all events.\n"
              "@param listener The listener to remove.\n")
{
   // Find the listener object.
   SimObject * listener = dynamic_cast< SimObject * >( Sim::findObject( argv[2] ) );
   if( listener )
      object->removeAll( listener );
}

ConsoleMethod( EventManager, dumpEvents, void, 2, 2, "()\n\n"
              "Print all registered events to the console." )
{
   object->dumpEvents();
}

ConsoleMethod( EventManager, dumpSubscribers, void, 2, 3, "( String event )\n\n"
              "Print all subscribers to an event to the console.\n"
              "@param event The event whose subscribers are to be printed. If this parameter isn't specified, all events will be dumped." )
{
   if( argc > 2 )
      object->dumpSubscribers( argv[2] );
   else
      object->dumpSubscribers();
}