/**********************************************************************/
/*! \class RtMidiAlsa
    \brief ALSA backend for realtime MIDI input/output.

    This class implements some common functionality for the realtime
    MIDI input/output subclasses RtMidiIn and RtMidiOut.

    RtMidi WWW site: http://music.mcgill.ca/~gary/rtmidi/

    RtMidi: realtime MIDI i/o C++ classes
    Copyright (c) 2003-2011 Gary P. Scavone

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation files
    (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    Any person wishing to distribute modifications to the Software is
    requested to send the modifications to the original developer so that
    they can be incorporated into the canonical version.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
    ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
    CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/**********************************************************************/

// RtMidi: Version 1.0.14

#include "RtMidiCoreMidi.h"
#include <sstream>

//*********************************************************************//
//  API: Macintosh OS-X
//*********************************************************************//

// API information found at:
//   - http://developer.apple.com/audio/pdf/coreaudio.pdf 

// The CoreMIDI API is based on the use of a callback function for
// MIDI input.  We convert the system specific time stamps to delta
// time values.

// OS-X CoreMIDI header files.
#include <CoreMIDI/CoreMIDI.h>
#include <CoreAudio/HostTime.h>
#include <CoreServices/CoreServices.h>

// A structure to hold variables related to the CoreMIDI API
// implementation.
struct CoreMidiData {
  MIDIClientRef client;
  MIDIPortRef port;
  MIDIEndpointRef endpoint;
  MIDIEndpointRef destinationId;
  unsigned long long lastTime;
  MIDISysexSendRequest sysexreq;
};

//*********************************************************************//
//  API: OS-X
//  Class Definitions: RtMidiIn
//*********************************************************************//

void midiInputCallback( const MIDIPacketList *list, void *procRef, void *srcRef )
{
  RtMidiIn::RtMidiInData *data = static_cast<RtMidiIn::RtMidiInData *> (procRef);
  CoreMidiData *apiData = static_cast<CoreMidiData *> (data->apiData);

  unsigned char status;
  unsigned short nBytes, iByte, size;
  unsigned long long time;

  bool& continueSysex = data->continueSysex;
  RtMidiIn::MidiMessage& message = data->message;

  const MIDIPacket *packet = &list->packet[0];
  for ( unsigned int i=0; i<list->numPackets; ++i ) {

    // My interpretation of the CoreMIDI documentation: all message
    // types, except sysex, are complete within a packet and there may
    // be several of them in a single packet.  Sysex messages can be
    // broken across multiple packets and PacketLists but are bundled
    // alone within each packet (these packets do not contain other
    // message types).  If sysex messages are split across multiple
    // MIDIPacketLists, they must be handled by multiple calls to this
    // function.

    nBytes = packet->length;
    if ( nBytes == 0 ) continue;

    // Calculate time stamp.
    message.timeStamp = 0.0;
    if ( data->firstMessage )
      data->firstMessage = false;
    else {
      time = packet->timeStamp;
      if ( time == 0 ) { // this happens when receiving asynchronous sysex messages
        time = AudioGetCurrentHostTime();
      }
      time -= apiData->lastTime;
      time = AudioConvertHostTimeToNanos( time );
      message.timeStamp = time * 0.000000001;
    }
    apiData->lastTime = packet->timeStamp;
    if ( apiData->lastTime == 0 ) { // this happens when receiving asynchronous sysex messages
      apiData->lastTime = AudioGetCurrentHostTime();
    }
    //std::cout << "TimeStamp = " << packet->timeStamp << std::endl;

    iByte = 0;
    if ( continueSysex ) {
      // We have a continuing, segmented sysex message.
      if ( !( data->ignoreFlags & 0x01 ) ) {
        // If we're not ignoring sysex messages, copy the entire packet.
        for ( unsigned int j=0; j<nBytes; ++j )
          message.bytes.push_back( packet->data[j] );
      }
      continueSysex = packet->data[nBytes-1] != 0xF7;

      if ( !continueSysex ) {
        // If not a continuing sysex message, invoke the user callback function or queue the message.
        if ( data->usingCallback && message.bytes.size() > 0 ) {
          RtMidiIn::RtMidiCallback callback = (RtMidiIn::RtMidiCallback) data->userCallback;
          callback( message.timeStamp, &message.bytes, data->userData );
        }
        else {
          // As long as we haven't reached our queue size limit, push the message.
          if ( data->queueLimit > data->queue.size() )
            data->queue.push( message );
          else
            std::cerr << "\nRtMidiIn: message queue limit reached!!\n\n";
        }
        message.bytes.clear();
      }
    }
    else {
      while ( iByte < nBytes ) {
        size = 0;
        // We are expecting that the next byte in the packet is a status byte.
        status = packet->data[iByte];
        if ( !(status & 0x80) ) break;
        // Determine the number of bytes in the MIDI message.
        if ( status < 0xC0 ) size = 3;
        else if ( status < 0xE0 ) size = 2;
        else if ( status < 0xF0 ) size = 3;
        else if ( status == 0xF0 ) {
          // A MIDI sysex
          if ( data->ignoreFlags & 0x01 ) {
            size = 0;
            iByte = nBytes;
          }
          else size = nBytes - iByte;
          continueSysex = packet->data[nBytes-1] != 0xF7;
        }
        else if ( status == 0xF1 ) {
            // A MIDI time code message
           if ( data->ignoreFlags & 0x02 ) {
            size = 0;
            iByte += 2;
           }
           else size = 2;
        }
        else if ( status == 0xF2 ) size = 3;
        else if ( status == 0xF3 ) size = 2;
        else if ( status == 0xF8 && ( data->ignoreFlags & 0x02 ) ) {
          // A MIDI timing tick message and we're ignoring it.
          size = 0;
          iByte += 1;
        }
        else if ( status == 0xFE && ( data->ignoreFlags & 0x04 ) ) {
          // A MIDI active sensing message and we're ignoring it.
          size = 0;
          iByte += 1;
        }
        else size = 1;

        // Copy the MIDI data to our vector.
        if ( size ) {
          message.bytes.assign( &packet->data[iByte], &packet->data[iByte+size] );
          if ( !continueSysex ) {
            // If not a continuing sysex message, invoke the user callback function or queue the message.
            if ( data->usingCallback ) {
              RtMidiIn::RtMidiCallback callback = (RtMidiIn::RtMidiCallback) data->userCallback;
              callback( message.timeStamp, &message.bytes, data->userData );
            }
            else {
              // As long as we haven't reached our queue size limit, push the message.
              if ( data->queueLimit > data->queue.size() )
                data->queue.push( message );
              else
                std::cerr << "\nRtMidiIn: message queue limit reached!!\n\n";
            }
            message.bytes.clear();
          }
          iByte += size;
        }
      }
    }
    packet = MIDIPacketNext(packet);
  }
}

RtMidiInCoreMidi :: RtMidiInCoreMidi( const std::string clientName ) : RtMidiIn()
{
  this->initialize( clientName );
}

void RtMidiInCoreMidi :: initialize( const std::string& clientName )
{
  // Set up our client.
  MIDIClientRef client;
  OSStatus result = MIDIClientCreate( CFStringCreateWithCString( NULL, clientName.c_str(), kCFStringEncodingASCII ), NULL, NULL, &client );
  if ( result != noErr ) {
    errorString_ = "RtMidiIn::initialize: error creating OS-X MIDI client object.";
    error( RtError::DRIVER_ERROR );
  }

  // Save our api-specific connection information.
  CoreMidiData *data = (CoreMidiData *) new CoreMidiData;
  data->client = client;
  data->endpoint = 0;
  apiData_ = (void *) data;
  inputData_.apiData = (void *) data;
}

void RtMidiInCoreMidi :: openPort( unsigned int portNumber, const std::string portName )
{
  if ( connected_ ) {
    errorString_ = "RtMidiIn::openPort: a valid connection already exists!";
    error( RtError::WARNING );
    return;
  }

  unsigned int nSrc = MIDIGetNumberOfSources();
  if (nSrc < 1) {
    errorString_ = "RtMidiIn::openPort: no MIDI input sources found!";
    error( RtError::NO_DEVICES_FOUND );
  }

  std::ostringstream ost;
  if ( portNumber >= nSrc ) {
    ost << "RtMidiIn::openPort: the 'portNumber' argument (" << portNumber << ") is invalid.";
    errorString_ = ost.str();
    error( RtError::INVALID_PARAMETER );
  }

  MIDIPortRef port;
  CoreMidiData *data = static_cast<CoreMidiData *> (apiData_);
  OSStatus result = MIDIInputPortCreate( data->client, 
                                         CFStringCreateWithCString( NULL, portName.c_str(), kCFStringEncodingASCII ),
                                         midiInputCallback, (void *)&inputData_, &port );
  if ( result != noErr ) {
    MIDIClientDispose( data->client );
    errorString_ = "RtMidiIn::openPort: error creating OS-X MIDI input port.";
    error( RtError::DRIVER_ERROR );
  }

  // Get the desired input source identifier.
  MIDIEndpointRef endpoint = MIDIGetSource( portNumber );
  if ( endpoint == 0 ) {
    MIDIPortDispose( port );
    MIDIClientDispose( data->client );
    errorString_ = "RtMidiIn::openPort: error getting MIDI input source reference.";
    error( RtError::DRIVER_ERROR );
  }

  // Make the connection.
  result = MIDIPortConnectSource( port, endpoint, NULL );
  if ( result != noErr ) {
    MIDIPortDispose( port );
    MIDIClientDispose( data->client );
    errorString_ = "RtMidiIn::openPort: error connecting OS-X MIDI input port.";
    error( RtError::DRIVER_ERROR );
  }

  // Save our api-specific port information.
  data->port = port;

  connected_ = true;
}

void RtMidiInCoreMidi :: openVirtualPort( const std::string portName )
{
  CoreMidiData *data = static_cast<CoreMidiData *> (apiData_);

  // Create a virtual MIDI input destination.
  MIDIEndpointRef endpoint;
  OSStatus result = MIDIDestinationCreate( data->client,
                                           CFStringCreateWithCString( NULL, portName.c_str(), kCFStringEncodingASCII ),
                                           midiInputCallback, (void *)&inputData_, &endpoint );
  if ( result != noErr ) {
    errorString_ = "RtMidiIn::openVirtualPort: error creating virtual OS-X MIDI destination.";
    error( RtError::DRIVER_ERROR );
  }

  // Save our api-specific connection information.
  data->endpoint = endpoint;
}

void RtMidiInCoreMidi :: closePort( void )
{
  if ( connected_ ) {
    CoreMidiData *data = static_cast<CoreMidiData *> (apiData_);
    MIDIPortDispose( data->port );
    connected_ = false;
  }
}

RtMidiInCoreMidi :: ~RtMidiInCoreMidi()
{
  // Close a connection if it exists.
  closePort();

  // Cleanup.
  CoreMidiData *data = static_cast<CoreMidiData *> (apiData_);
  MIDIClientDispose( data->client );
  if ( data->endpoint ) MIDIEndpointDispose( data->endpoint );
  delete data;
}

unsigned int RtMidiInCoreMidi :: getPortCount()
{
  return MIDIGetNumberOfSources();
}

// This function was submitted by Douglas Casey Tucker and apparently
// derived largely from PortMidi.
CFStringRef EndpointName( MIDIEndpointRef endpoint, bool isExternal )
{
  CFMutableStringRef result = CFStringCreateMutable( NULL, 0 );
  CFStringRef str;

  // Begin with the endpoint's name.
  str = NULL;
  MIDIObjectGetStringProperty( endpoint, kMIDIPropertyName, &str );
  if ( str != NULL ) {
    CFStringAppend( result, str );
    CFRelease( str );
  }

  MIDIEntityRef entity = NULL;
  MIDIEndpointGetEntity( endpoint, &entity );
  if ( entity == 0 )
    // probably virtual
    return result;

  if ( CFStringGetLength( result ) == 0 ) {
    // endpoint name has zero length -- try the entity
    str = NULL;
    MIDIObjectGetStringProperty( entity, kMIDIPropertyName, &str );
    if ( str != NULL ) {
      CFStringAppend( result, str );
      CFRelease( str );
    }
  }
  // now consider the device's name
  MIDIDeviceRef device = 0;
  MIDIEntityGetDevice( entity, &device );
  if ( device == 0 )
    return result;

  str = NULL;
  MIDIObjectGetStringProperty( device, kMIDIPropertyName, &str );
  if ( CFStringGetLength( result ) == 0 ) {
      CFRelease( result );
      return str;
  }
  if ( str != NULL ) {
    // if an external device has only one entity, throw away
    // the endpoint name and just use the device name
    if ( isExternal && MIDIDeviceGetNumberOfEntities( device ) < 2 ) {
      CFRelease( result );
      return str;
    } else {
      if ( CFStringGetLength( str ) == 0 ) {
        CFRelease( str );
        return result;
      }
      // does the entity name already start with the device name?
      // (some drivers do this though they shouldn't)
      // if so, do not prepend
        if ( CFStringCompareWithOptions( result, /* endpoint name */
             str /* device name */,
             CFRangeMake(0, CFStringGetLength( str ) ), 0 ) != kCFCompareEqualTo ) {
        // prepend the device name to the entity name
        if ( CFStringGetLength( result ) > 0 )
          CFStringInsert( result, 0, CFSTR(" ") );
        CFStringInsert( result, 0, str );
      }
      CFRelease( str );
    }
  }
  return result;
}

// This function was submitted by Douglas Casey Tucker and apparently
// derived largely from PortMidi.
static CFStringRef ConnectedEndpointName( MIDIEndpointRef endpoint )
{
  CFMutableStringRef result = CFStringCreateMutable( NULL, 0 );
  CFStringRef str;
  OSStatus err;
  int i;

  // Does the endpoint have connections?
  CFDataRef connections = NULL;
  int nConnected = 0;
  bool anyStrings = false;
  err = MIDIObjectGetDataProperty( endpoint, kMIDIPropertyConnectionUniqueID, &connections );
  if ( connections != NULL ) {
    // It has connections, follow them
    // Concatenate the names of all connected devices
    nConnected = CFDataGetLength( connections ) / sizeof(MIDIUniqueID);
    if ( nConnected ) {
      const SInt32 *pid = (const SInt32 *)(CFDataGetBytePtr(connections));
      for ( i=0; i<nConnected; ++i, ++pid ) {
        MIDIUniqueID id = EndianS32_BtoN( *pid );
        MIDIObjectRef connObject;
        MIDIObjectType connObjectType;
        err = MIDIObjectFindByUniqueID( id, &connObject, &connObjectType );
        if ( err == noErr ) {
          if ( connObjectType == kMIDIObjectType_ExternalSource  ||
              connObjectType == kMIDIObjectType_ExternalDestination ) {
            // Connected to an external device's endpoint (10.3 and later).
            str = EndpointName( (MIDIEndpointRef)(connObject), true );
          } else {
            // Connected to an external device (10.2) (or something else, catch-
            str = NULL;
            MIDIObjectGetStringProperty( connObject, kMIDIPropertyName, &str );
          }
          if ( str != NULL ) {
            if ( anyStrings )
              CFStringAppend( result, CFSTR(", ") );
            else anyStrings = true;
            CFStringAppend( result, str );
            CFRelease( str );
          }
        }
      }
    }
    CFRelease( connections );
  }
  if ( anyStrings )
    return result;

  // Here, either the endpoint had no connections, or we failed to obtain names 
  return EndpointName( endpoint, false );
}

std::string RtMidiInCoreMidi :: getPortName( unsigned int portNumber )
{
  CFStringRef nameRef;
  MIDIEndpointRef portRef;
  std::ostringstream ost;
  char name[128];

  std::string stringName;
  if ( portNumber >= MIDIGetNumberOfSources() ) {
    ost << "RtMidiIn::getPortName: the 'portNumber' argument (" << portNumber << ") is invalid.";
    errorString_ = ost.str();
    error( RtError::WARNING );
    //error( RtError::INVALID_PARAMETER );
    return stringName;
  }

  portRef = MIDIGetSource( portNumber );
  nameRef = ConnectedEndpointName(portRef);
  CFStringGetCString( nameRef, name, sizeof(name), 0);
  CFRelease( nameRef );

  return stringName = name;
}

//*********************************************************************//
//  API: OS-X
//  Class Definitions: RtMidiOut
//*********************************************************************//

unsigned int RtMidiOutCoreMidi :: getPortCount()
{
  return MIDIGetNumberOfDestinations();
}

std::string RtMidiOutCoreMidi :: getPortName( unsigned int portNumber )
{
  CFStringRef nameRef;
  MIDIEndpointRef portRef;
  std::ostringstream ost;
  char name[128];

  std::string stringName;
  if ( portNumber >= MIDIGetNumberOfDestinations() ) {
    ost << "RtMidiOut::getPortName: the 'portNumber' argument (" << portNumber << ") is invalid.";
    errorString_ = ost.str();
    error( RtError::WARNING );
    return stringName;
    //error( RtError::INVALID_PARAMETER );
  }

  portRef = MIDIGetDestination( portNumber );
  nameRef = ConnectedEndpointName(portRef);
  CFStringGetCString( nameRef, name, sizeof(name), 0);
  CFRelease( nameRef );
  
  return stringName = name;
}

RtMidiOutCoreMidi :: RtMidiOutCoreMidi( const std::string clientName ) : RtMidiOut()
{
  this->initialize( clientName );
}

void RtMidiOutCoreMidi :: initialize( const std::string& clientName )
{
  // Set up our client.
  MIDIClientRef client;
  OSStatus result = MIDIClientCreate( CFStringCreateWithCString( NULL, clientName.c_str(), kCFStringEncodingASCII ), NULL, NULL, &client );
  if ( result != noErr ) {
    errorString_ = "RtMidiOut::initialize: error creating OS-X MIDI client object.";
    error( RtError::DRIVER_ERROR );
  }

  // Save our api-specific connection information.
  CoreMidiData *data = (CoreMidiData *) new CoreMidiData;
  data->client = client;
  data->endpoint = 0;
  apiData_ = (void *) data;
}

void RtMidiOutCoreMidi :: openPort( unsigned int portNumber, const std::string portName )
{
  if ( connected_ ) {
    errorString_ = "RtMidiOut::openPort: a valid connection already exists!";
    error( RtError::WARNING );
    return;
  }

  unsigned int nDest = MIDIGetNumberOfDestinations();
  if (nDest < 1) {
    errorString_ = "RtMidiOut::openPort: no MIDI output destinations found!";
    error( RtError::NO_DEVICES_FOUND );
  }

  std::ostringstream ost;
  if ( portNumber >= nDest ) {
    ost << "RtMidiOut::openPort: the 'portNumber' argument (" << portNumber << ") is invalid.";
    errorString_ = ost.str();
    error( RtError::INVALID_PARAMETER );
  }

  MIDIPortRef port;
  CoreMidiData *data = static_cast<CoreMidiData *> (apiData_);
  OSStatus result = MIDIOutputPortCreate( data->client, 
                                          CFStringCreateWithCString( NULL, portName.c_str(), kCFStringEncodingASCII ),
                                          &port );
  if ( result != noErr ) {
    MIDIClientDispose( data->client );
    errorString_ = "RtMidiOut::openPort: error creating OS-X MIDI output port.";
    error( RtError::DRIVER_ERROR );
  }

  // Get the desired output port identifier.
  MIDIEndpointRef destination = MIDIGetDestination( portNumber );
  if ( destination == 0 ) {
    MIDIPortDispose( port );
    MIDIClientDispose( data->client );
    errorString_ = "RtMidiOut::openPort: error getting MIDI output destination reference.";
    error( RtError::DRIVER_ERROR );
  }

  // Save our api-specific connection information.
  data->port = port;
  data->destinationId = destination;
  connected_ = true;
}

void RtMidiOutCoreMidi :: closePort( void )
{
  if ( connected_ ) {
    CoreMidiData *data = static_cast<CoreMidiData *> (apiData_);
    MIDIPortDispose( data->port );
    connected_ = false;
  }
}

void RtMidiOutCoreMidi :: openVirtualPort( std::string portName )
{
  CoreMidiData *data = static_cast<CoreMidiData *> (apiData_);

  if ( data->endpoint ) {
    errorString_ = "RtMidiOut::openVirtualPort: a virtual output port already exists!";
    error( RtError::WARNING );
    return;
  }

  // Create a virtual MIDI output source.
  MIDIEndpointRef endpoint;
  OSStatus result = MIDISourceCreate( data->client,
                                      CFStringCreateWithCString( NULL, portName.c_str(), kCFStringEncodingASCII ),
                                      &endpoint );
  if ( result != noErr ) {
    errorString_ = "RtMidiOut::initialize: error creating OS-X virtual MIDI source.";
    error( RtError::DRIVER_ERROR );
  }

  // Save our api-specific connection information.
  data->endpoint = endpoint;
}

RtMidiOutCoreMidi :: ~RtMidiOutCoreMidi()
{
  // Close a connection if it exists.
  closePort();

  // Cleanup.
  CoreMidiData *data = static_cast<CoreMidiData *> (apiData_);
  MIDIClientDispose( data->client );
  if ( data->endpoint ) MIDIEndpointDispose( data->endpoint );
  delete data;
}

char *sysexBuffer = 0;

void sysexCompletionProc( MIDISysexSendRequest * sreq )
{
  //std::cout << "Completed SysEx send\n";
 delete sysexBuffer;
 sysexBuffer = 0;
}

void RtMidiOutCoreMidi :: sendMessage( std::vector<unsigned char> *message )
{
  // We use the MIDISendSysex() function to asynchronously send sysex
  // messages.  Otherwise, we use a single CoreMidi MIDIPacket.
  unsigned int nBytes = message->size();
  if ( nBytes == 0 ) {
    errorString_ = "RtMidiOut::sendMessage: no data in message argument!";      
    error( RtError::WARNING );
    return;
  }

  //  unsigned int packetBytes, bytesLeft = nBytes;
  //  unsigned int messageIndex = 0;
  MIDITimeStamp timeStamp = AudioGetCurrentHostTime();
  CoreMidiData *data = static_cast<CoreMidiData *> (apiData_);
  OSStatus result;

  if ( message->at(0) == 0xF0 ) {

    while ( sysexBuffer != 0 ) usleep( 1000 ); // sleep 1 ms

   sysexBuffer = new char[nBytes];
   if ( sysexBuffer == NULL ) {
     errorString_ = "RtMidiOut::sendMessage: error allocating sysex message memory!";
     error( RtError::MEMORY_ERROR );
   }

   // Copy data to buffer.
   for ( unsigned int i=0; i<nBytes; ++i ) sysexBuffer[i] = message->at(i);

   data->sysexreq.destination = data->destinationId;
   data->sysexreq.data = (Byte *)sysexBuffer;
   data->sysexreq.bytesToSend = nBytes;
   data->sysexreq.complete = 0;
   data->sysexreq.completionProc = sysexCompletionProc;
   data->sysexreq.completionRefCon = &(data->sysexreq);

   result = MIDISendSysex( &(data->sysexreq) );
   if ( result != noErr ) {
     errorString_ = "RtMidiOut::sendMessage: error sending MIDI to virtual destinations.";
     error( RtError::WARNING );
   }
   return;
  }
  else if ( nBytes > 3 ) {
   errorString_ = "RtMidiOut::sendMessage: message format problem ... not sysex but > 3 bytes?";
   error( RtError::WARNING );
   return;
  }

  MIDIPacketList packetList;
  MIDIPacket *packet = MIDIPacketListInit( &packetList );
  packet = MIDIPacketListAdd( &packetList, sizeof(packetList), packet, timeStamp, nBytes, (const Byte *) &message->at( 0 ) );
  if ( !packet ) {
    errorString_ = "RtMidiOut::sendMessage: could not allocate packet list";      
    error( RtError::DRIVER_ERROR );
  }

  // Send to any destinations that may have connected to us.
  if ( data->endpoint ) {
    result = MIDIReceived( data->endpoint, &packetList );
    if ( result != noErr ) {
      errorString_ = "RtMidiOut::sendMessage: error sending MIDI to virtual destinations.";
      error( RtError::WARNING );
    }
  }

  // And send to an explicit destination port if we're connected.
  if ( connected_ ) {
    result = MIDISend( data->port, data->destinationId, &packetList );
    if ( result != noErr ) {
      errorString_ = "RtMidiOut::sendMessage: error sending MIDI message to port.";
      error( RtError::WARNING );
    }
  }
}
