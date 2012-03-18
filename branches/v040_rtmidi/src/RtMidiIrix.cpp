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

#include "RtMidiIrix.h"
#include <sstream>

//*********************************************************************//
//  API: IRIX MD
//*********************************************************************//

// API information gleamed from:
//   http://techpubs.sgi.com/library/tpl/cgi-bin/getdoc.cgi?cmd=getdoc&coll=0650&db=man&fname=3%20mdIntro

// If the Makefile doesn't work, try the following:
// CC -o midiinfo -LANG:std -D__IRIX_MD__ -I../ ../RtMidi.cpp midiinfo.cpp -lpthread -lmd
// CC -o midiout -LANG:std -D__IRIX_MD__ -I../ ../RtMidi.cpp midiout.cpp -lpthread -lmd
// CC -o qmidiin -LANG:std -D__IRIX_MD__ -I../ ../RtMidi.cpp qmidiin.cpp -lpthread -lmd
// CC -o cmidiin -LANG:std -D__IRIX_MD__ -I../ ../RtMidi.cpp cmidiin.cpp -lpthread -lmd

#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

// Irix MIDI header file.
#include <dmedia/midi.h>

// A structure to hold variables related to the IRIX API
// implementation.
struct IrixMidiData {
  MDport port;
  pthread_t thread;
};

//*********************************************************************//
//  API: IRIX
//  Class Definitions: RtMidiIn
//*********************************************************************//

extern "C" void *irixMidiHandler( void *ptr )
{
  RtMidiIn::RtMidiInData *data = static_cast<RtMidiIn::RtMidiInData *> (ptr);
  IrixMidiData *apiData = static_cast<IrixMidiData *> (data->apiData);

  bool continueSysex = false;
  unsigned char status;
  unsigned short size;
  MDevent event;
  int fd = mdGetFd( apiData->port );
  if ( fd < 0 ) {
    data->doInput = false;
    std::cerr << "\nRtMidiIn::irixMidiHandler: error getting port descriptor!\n\n";
    return 0;
  }

  fd_set mask, rmask;
  FD_ZERO( &mask );
  FD_SET( fd, &mask );
  struct timeval timeout = {0, 0};
  RtMidiIn::MidiMessage message;
  int result;

  while ( data->doInput ) {

    rmask = mask;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    if ( select( fd+1, &rmask, NULL, NULL, &timeout ) <= 0 ) {
      // No data pending ... sleep a bit.
      usleep( 1000 );
      continue;
    }

    // If here, there should be data.
    result = mdReceive( apiData->port, &event, 1);
    if ( result <= 0 ) {
      std::cerr << "\nRtMidiIn::irixMidiHandler: MIDI input read error!\n\n";
      continue;
    }

    message.timeStamp = event.stamp * 0.000000001;

    size = 0;
    status = event.msg[0];
    if ( !(status & 0x80) ) continue;
    if ( status == 0xF0 ) {
      // Sysex message ... can be segmented across multiple messages.
      if ( !(data->ignoreFlags & 0x01) ) {
        if ( continueSysex ) {
          // We have a continuing, segmented sysex message.  Append
          // the new bytes to our existing message.
          for ( int i=0; i<event.msglen; ++i )
            message.bytes.push_back( event.sysexmsg[i] );
          if ( event.sysexmsg[event.msglen-1] == 0xF7 ) continueSysex = false;
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
      }
      mdFree( NULL );
      continue;
    }
    else if ( status < 0xC0 ) size = 3;
    else if ( status < 0xE0 ) size = 2;
    else if ( status < 0xF0 ) size = 3;
    else if ( status == 0xF1 && !(data->ignoreFlags & 0x02) ) {
        // A MIDI time code message and we're not ignoring it.
        size = 2;
    }
    else if ( status == 0xF2 ) size = 3;
    else if ( status == 0xF3 ) size = 2;
    else if ( status == 0xF8 ) {
      if ( !(data->ignoreFlags & 0x02) ) {
        // A MIDI timing tick message and we're not ignoring it.
        size = 1;
      }
    }
    else if ( status == 0xFE ) { // MIDI active sensing
      if ( !(data->ignoreFlags & 0x04) )
        size = 1;
    }
    else size = 1;

    // Copy the MIDI data to our vector.
    if ( size ) {
      message.bytes.assign( &event.msg[0], &event.msg[size] );
      // Invoke the user callback function or queue the message.
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
  }

  return 0;
}

RtMidiInIrix :: RtMidiInIrix( const std::string clientName ) : RtMidiIn()
{
  this->initialize( clientName );
}

void RtMidiInIrix :: initialize( const std::string& /*clientName*/ )
{
  // Initialize the Irix MIDI system.  At the moment, we will not
  // worry about a return value of zero (ports) because there is a
  // chance the user could plug something in after instantiation.
  int nPorts = mdInit();

  // Create our api-specific connection information.
  IrixMidiData *data = (IrixMidiData *) new IrixMidiData;
  apiData_ = (void *) data;
  inputData_.apiData = (void *) data;
}

void RtMidiInIrix :: openPort( unsigned int portNumber, const std::string /*portName*/ )
{
  if ( connected_ ) {
    errorString_ = "RtMidiIn::openPort: a valid connection already exists!";
    error( RtError::WARNING );
    return;
  }

  int nPorts = mdInit();
  if (nPorts < 1) {
    errorString_ = "RtMidiIn::openPort: no Irix MIDI input sources found!";
    error( RtError::NO_DEVICES_FOUND );
  }

  std::ostringstream ost;
  if ( portNumber >= nPorts ) {
    ost << "RtMidiIn::openPort: the 'portNumber' argument (" << portNumber << ") is invalid.";
    errorString_ = ost.str();
    error( RtError::INVALID_PARAMETER );
  }

  IrixMidiData *data = static_cast<IrixMidiData *> (apiData_);
  data->port = mdOpenInPort( mdGetName(portNumber) );
  if ( data->port == NULL ) {
    ost << "RtMidiIn::openPort: Irix error opening the port (" << portNumber << ").";
    errorString_ = ost.str();
    error( RtError::DRIVER_ERROR );
  }
  mdSetStampMode(data->port, MD_DELTASTAMP);

  // Start our MIDI input thread.
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_attr_setschedpolicy(&attr, SCHED_RR);

  inputData_.doInput = true;
  int err = pthread_create(&data->thread, &attr, irixMidiHandler, &inputData_);
  pthread_attr_destroy(&attr);
  if (err) {
    mdClosePort( data->port );
    inputData_.doInput = false;
    errorString_ = "RtMidiIn::openPort: error starting MIDI input thread!";
    error( RtError::THREAD_ERROR );
  }

  connected_ = true;
}

void RtMidiInIrix :: openVirtualPort( std::string portName )
{
  // This function cannot be implemented for the Irix MIDI API.
  errorString_ = "RtMidiIn::openVirtualPort: cannot be implemented in Irix MIDI API!";
  error( RtError::WARNING );
}

void RtMidiInIrix :: closePort( void )
{
  if ( connected_ ) {
    IrixMidiData *data = static_cast<IrixMidiData *> (apiData_);
    mdClosePort( data->port );
    connected_ = false;

    // Shutdown the input thread.
    inputData_.doInput = false;
    pthread_join( data->thread, NULL );
  }
}

RtMidiInIrix :: ~RtMidiInIrix()
{
  // Close a connection if it exists.
  closePort();

  // Cleanup.
  IrixMidiData *data = static_cast<IrixMidiData *> (apiData_);
  delete data;
}

unsigned int RtMidiInIrix :: getPortCount()
{
  int nPorts = mdInit();
  if ( nPorts >= 0 ) return nPorts;
  else return 0;
}

std::string RtMidiInIrix :: getPortName( unsigned int portNumber )
{
  int nPorts = mdInit();

  std::string stringName;
  std::ostringstream ost;
  if ( portNumber >= nPorts ) {
    ost << "RtMidiIn::getPortName: the 'portNumber' argument (" << portNumber << ") is invalid.";
    errorString_ = ost.str();
    //error( RtError::INVALID_PARAMETER );
    error( RtError::WARNING );
  }
  else
    std::string stringName = std::string( mdGetName( portNumber ) );

  return stringName;
}

//*********************************************************************//
//  API: IRIX MD
//  Class Definitions: RtMidiOut
//*********************************************************************//

RtMidiOutIrix :: RtMidiOutIrix( const std::string clientName ) : RtMidiOut()
{
  this->initialize( clientName );
}

unsigned int RtMidiOutIrix :: getPortCount()
{
  int nPorts = mdInit();
  if ( nPorts >= 0 ) return nPorts;
  else return 0;
}

std::string RtMidiOutIrix :: getPortName( unsigned int portNumber )
{
  int nPorts = mdInit();

  std::string stringName;
  std::ostringstream ost;
  if ( portNumber >= nPorts ) {
    ost << "RtMidiIn::getPortName: the 'portNumber' argument (" << portNumber << ") is invalid.";
    errorString_ = ost.str();
    //error( RtError::INVALID_PARAMETER );
    error( RtError::WARNING );
  }
  else
    std::string stringName = std::string( mdGetName( portNumber ) );

  return stringName;
}

void RtMidiOutIrix :: initialize( const std::string& /*clientName*/ )
{
  // Initialize the Irix MIDI system.  At the moment, we will not
  // worry about a return value of zero (ports) because there is a
  // chance the user could plug something in after instantiation.
  int nPorts = mdInit();

  // Create our api-specific connection information.
  IrixMidiData *data = (IrixMidiData *) new IrixMidiData;
  apiData_ = (void *) data;
}

void RtMidiOutIrix :: openPort( unsigned int portNumber, const std::string /*portName*/ )
{
  if ( connected_ ) {
    errorString_ = "RtMidiOut::openPort: a valid connection already exists!";
    error( RtError::WARNING );
    return;
  }

  int nPorts = mdInit();
  if (nPorts < 1) {
    errorString_ = "RtMidiOut::openPort: no Irix MIDI output sources found!";
    error( RtError::NO_DEVICES_FOUND );
  }

  std::ostringstream ost;
  if ( portNumber >= nPorts ) {
    ost << "RtMidiOut::openPort: the 'portNumber' argument (" << portNumber << ") is invalid.";
    errorString_ = ost.str();
    error( RtError::INVALID_PARAMETER );
  }

  IrixMidiData *data = static_cast<IrixMidiData *> (apiData_);
  data->port = mdOpenOutPort( mdGetName(portNumber) );
  if ( data->port == NULL ) {
    ost << "RtMidiOut::openPort: Irix error opening the port (" << portNumber << ").";
    errorString_ = ost.str();
    error( RtError::DRIVER_ERROR );
  }
  mdSetStampMode(data->port, MD_NOSTAMP);

  connected_ = true;
}

void RtMidiOutIrix :: closePort( void )
{
  if ( connected_ ) {
    IrixMidiData *data = static_cast<IrixMidiData *> (apiData_);
    mdClosePort( data->port );
    connected_ = false;
  }
}

void RtMidiOutIrix :: openVirtualPort( std::string portName )
{
  // This function cannot be implemented for the Irix MIDI API.
  errorString_ = "RtMidiOut::openVirtualPort: cannot be implemented in Irix MIDI API!";
  error( RtError::WARNING );
}

RtMidiOutIrix :: ~RtMidiOutIrix()
{
  // Close a connection if it exists.
  closePort();

  // Cleanup.
  IrixMidiData *data = static_cast<IrixMidiData *> (apiData_);
  delete data;
}

void RtMidiOutIrix :: sendMessage( std::vector<unsigned char> *message )
{
  int result;
  MDevent event;
  IrixMidiData *data = static_cast<IrixMidiData *> (apiData_);
  char *buffer = 0;

  unsigned int nBytes = message->size();
  if ( nBytes == 0 ) return;
  event.stamp = 0;
  if ( message->at(0) == 0xF0 ) {
    if ( nBytes < 3 ) return; // check for bogus sysex
    event.msg[0] = 0xF0;
    event.msglen = nBytes;
    buffer = (char *) malloc( nBytes );
    for ( int i=0; i<nBytes; ++i ) buffer[i] = message->at(i);
    event.sysexmsg = buffer;
  }
  else {
    for ( int i=0; i<nBytes; ++i )
      event.msg[i] = message->at(i);
  }

  // Send the event.
  result = mdSend( data->port, &event, 1 );
  if ( buffer ) free( buffer );
  if ( result < 1 ) {
    errorString_ = "RtMidiOut::sendMessage: IRIX error sending MIDI message!";
    error( RtError::WARNING );
    return;
  }
}
