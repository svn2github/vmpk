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

#include "RtMidiJack.h"
#include <sstream>

RtMidiInJack :: RtMidiInJack( const std::string clientName ) : RtMidiIn()
{
  this->initialize( clientName );
}

RtMidiOutJack :: RtMidiOutJack( const std::string clientName ) : RtMidiOut()
{
  this->initialize( clientName );
}

//*********************************************************************//
//  API: LINUX JACK
//
//  Written primarily by Alexander Svetalkin, with updates for delta
//  time by Gary Scavone, April 2011.
//
//  *********************************************************************//

// JACK header files
#include <jack/jack.h>
#include <jack/midiport.h>
#include <jack/ringbuffer.h>

#define JACK_RINGBUFFER_SIZE 16384 // Default size for ringbuffer

struct JackMidiData {
  jack_client_t *client;
  jack_port_t *port;
  jack_ringbuffer_t *buffSize;
  jack_ringbuffer_t *buffMessage;
  jack_time_t lastTime;
  };

struct Arguments {
  JackMidiData *jackData;
  RtMidiIn :: RtMidiInData *rtMidiIn;
  };

//*********************************************************************//
//  API: JACK
//  Class Definitions: RtMidiIn
//*********************************************************************//

int jackProcessIn( jack_nframes_t nframes, void *arg )
{
  JackMidiData *jData = ( (Arguments *) arg )->jackData;
  RtMidiIn :: RtMidiInData *rtData = ( (Arguments *) arg )->rtMidiIn;
  jack_midi_event_t event;
  jack_time_t long long time;

  // Is port created?
  if ( jData->port == NULL ) return 0;
  void *buff = jack_port_get_buffer( jData->port, nframes );

  // We have midi events in buffer
  int evCount = jack_midi_get_event_count( buff );
  if ( evCount > 0 ) {
    RtMidiIn::MidiMessage message;
    message.bytes.clear();

    jack_midi_event_get( &event, buff, 0 );

    for (unsigned int i = 0; i < event.size; i++ )
      message.bytes.push_back( event.buffer[i] );

    // Compute the delta time.
    time = jack_get_time();
    if ( rtData->firstMessage == true )
      rtData->firstMessage = false;
    else
      message.timeStamp = ( time - jData->lastTime ) * 0.000001;

    jData->lastTime = time;

    if ( rtData->usingCallback && !rtData->continueSysex ) {
      RtMidiIn::RtMidiCallback callback = (RtMidiIn::RtMidiCallback) rtData->userCallback;
      callback( message.timeStamp, &message.bytes, rtData->userData );
    }
    else {
      // As long as we haven't reached our queue size limit, push the message.
      if ( rtData->queueLimit > rtData->queue.size() )
        rtData->queue.push( message );
      else
        std::cerr << "\nRtMidiIn: message queue limit reached!!\n\n";
    }
  }

  return 0;
}

void RtMidiInJack :: initialize( const std::string& clientName )
{
  JackMidiData *data = new JackMidiData;

  // Initialize JACK client
  if (( data->client = jack_client_open( clientName.c_str(), JackNullOption, NULL )) == 0)
    {
    errorString_ = "RtMidiOut::initialize: JACK server not running?";
    error( RtError::DRIVER_ERROR );
    return;
    }

  Arguments *arg = new Arguments;
  arg->jackData = data;

  arg->rtMidiIn = &inputData_;
  jack_set_process_callback( data->client, jackProcessIn, arg );
  data->port = NULL;
  jack_activate( data->client );

  apiData_ = (void *) data;
}

RtMidiInJack :: ~RtMidiInJack()
{
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);
  jack_client_close( data->client );
}

void RtMidiInJack :: openPort( unsigned int portNumber, const std::string portName )
{
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);

  // Creating new port
  if ( data->port == NULL)
    data->port = jack_port_register( data->client, portName.c_str(),
                                     JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0 );

  if ( data->port == NULL) {
    errorString_ = "RtMidiOut::openVirtualPort: JACK error creating virtual port";
    error( RtError::DRIVER_ERROR );
  }

  // Connecting to the output
  std::string name = getPortName( portNumber );
  jack_connect( data->client, name.c_str(), jack_port_name( data->port ) );
}

void RtMidiInJack :: openVirtualPort( const std::string portName )
{
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);

  if ( data->port == NULL )
    data->port = jack_port_register( data->client, portName.c_str(),
                                     JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0 );

  if ( data->port == NULL ) {
    errorString_ = "RtMidiOut::openVirtualPort: JACK error creating virtual port";
    error( RtError::DRIVER_ERROR );
  }
}

unsigned int RtMidiInJack :: getPortCount()
{
  int count = 0;
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);

  // List of available ports
  const char **ports = jack_get_ports( data->client, NULL,
    JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput );

  if ( ports == NULL ) return 0;
  while ( ports[count] != NULL )
    count++;

  free( ports );

  return count;
}

std::string RtMidiInJack :: getPortName( unsigned int portNumber )
{
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);
  std::ostringstream ost;
  std::string retStr("");

  // List of available ports
  const char **ports = jack_get_ports( data->client, NULL,
    JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput );

  // Check port validity
  if ( ports == NULL ) {
    errorString_ = "RtMidiOut::getPortName: no ports available!";
    error( RtError::WARNING );
    return retStr;
  }

  if ( ports[portNumber] == NULL ) {
    ost << "RtMidiOut::getPortName: the 'portNumber' argument (" << portNumber << ") is invalid.";
    errorString_ = ost.str();
    error( RtError::WARNING );
  }
  else retStr.assign( ports[portNumber] );

  free( ports );

  return retStr;
}

void RtMidiInJack :: closePort()
{
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);

  if ( data->port == NULL ) return;
  jack_port_unregister( data->client, data->port );
}

//*********************************************************************//
//  API: JACK
//  Class Definitions: RtMidiOut
//*********************************************************************//

// Jack process callback
int jackProcessOut( jack_nframes_t nframes, void *arg )
{
  JackMidiData *data = (JackMidiData *) arg;
  jack_midi_data_t *midiData;
  int space;

  // Is port created?
  if ( data->port == NULL ) return 0;

  void *buff = jack_port_get_buffer( data->port, nframes );
  jack_midi_clear_buffer( buff );

  while ( jack_ringbuffer_read_space( data->buffSize ) > 0 ) {
    jack_ringbuffer_read( data->buffSize, (char *) &space, (size_t) sizeof(space) );
    midiData = jack_midi_event_reserve( buff, 0, space );

    jack_ringbuffer_read( data->buffMessage, (char *) midiData, (size_t) space );
  }

  return 0;
}

void RtMidiOutJack :: initialize( const std::string& clientName )
{
  JackMidiData *data = new JackMidiData;

  // Initialize JACK client
  if (( data->client = jack_client_open( clientName.c_str(), JackNullOption, NULL )) == 0)
    {
    errorString_ = "RtMidiOut::initialize: JACK server not running?";
    error( RtError::DRIVER_ERROR );
    return;
    }

  jack_set_process_callback( data->client, jackProcessOut, data );
  data->buffSize = jack_ringbuffer_create( JACK_RINGBUFFER_SIZE );
  data->buffMessage = jack_ringbuffer_create( JACK_RINGBUFFER_SIZE );
  jack_activate( data->client );

  data->port = NULL;

  apiData_ = (void *) data;
}

RtMidiOutJack :: ~RtMidiOutJack()
{
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);

  // Cleanup
  jack_client_close( data->client );
  jack_ringbuffer_free( data->buffSize );
  jack_ringbuffer_free( data->buffMessage );
}

void RtMidiOutJack :: openPort( unsigned int portNumber, const std::string portName )
{
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);

  // Creating new port
  if ( data->port == NULL )
    data->port = jack_port_register( data->client, portName.c_str(),
      JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0 );

  if ( data->port == NULL ) {
    errorString_ = "RtMidiOut::openVirtualPort: JACK error creating virtual port";
    error( RtError::DRIVER_ERROR );
  }

  // Connecting to the output
  std::string name = getPortName( portNumber );
  jack_connect( data->client, jack_port_name( data->port ), name.c_str() );
}

void RtMidiOutJack :: openVirtualPort( const std::string portName )
{
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);

  if ( data->port == NULL )
    data->port = jack_port_register( data->client, portName.c_str(),
      JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0 );

  if ( data->port == NULL ) {
    errorString_ = "RtMidiOut::openVirtualPort: JACK error creating virtual port";
    error( RtError::DRIVER_ERROR );
  }
}

unsigned int RtMidiOutJack :: getPortCount()
{
  int count = 0;
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);

  // List of available ports
  const char **ports = jack_get_ports( data->client, NULL,
    JACK_DEFAULT_MIDI_TYPE, JackPortIsInput );

  if ( ports == NULL ) return 0;
  while ( ports[count] != NULL )
    count++;

  free( ports );

  return count;
}

std::string RtMidiOutJack :: getPortName( unsigned int portNumber )
{
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);
  std::ostringstream ost;
  std::string retStr("");

  // List of available ports
  const char **ports = jack_get_ports( data->client, NULL,
    JACK_DEFAULT_MIDI_TYPE, JackPortIsInput );

  // Check port validity
  if ( ports == NULL) {
    errorString_ = "RtMidiOut::getPortName: no ports available!";
    error( RtError::WARNING );
    return retStr;
  }

  if ( ports[portNumber] == NULL) {
    ost << "RtMidiOut::getPortName: the 'portNumber' argument (" << portNumber << ") is invalid.";
    errorString_ = ost.str();
    error( RtError::WARNING );
  }
  else retStr.assign( ports[portNumber] );

  free( ports );

  return retStr;
}

void RtMidiOutJack :: closePort()
{
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);

  if ( data->port == NULL ) return;
  jack_port_unregister( data->client, data->port );
  data->port = NULL;
}

void RtMidiOutJack :: sendMessage( std::vector<unsigned char> *message )
{
  int nBytes = message->size();
  JackMidiData *data = static_cast<JackMidiData *> (apiData_);

  // Write full message to buffer
  jack_ringbuffer_write( data->buffMessage, ( const char * ) &( *message )[0],
                         message->size() );
  jack_ringbuffer_write( data->buffSize, ( char * ) &nBytes, sizeof( nBytes ) );
}
