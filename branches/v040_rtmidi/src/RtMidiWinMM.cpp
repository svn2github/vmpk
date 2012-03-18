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

#include "RtMidiWinMM.h"
#include <sstream>

//*********************************************************************//
//  API: Windows Multimedia Library (MM)
//*********************************************************************//

// API information deciphered from:
//  - http://msdn.microsoft.com/library/default.asp?url=/library/en-us/multimed/htm/_win32_midi_reference.asp

// Thanks to Jean-Baptiste Berruchon for the sysex code.

// The Windows MM API is based on the use of a callback function for
// MIDI input.  We convert the system specific time stamps to delta
// time values.

// Windows MM MIDI header files.
#include <windows.h>
#include <mmsystem.h>

#define  RT_SYSEX_BUFFER_SIZE 1024
#define  RT_SYSEX_BUFFER_COUNT 4

// A structure to hold variables related to the CoreMIDI API
// implementation.
struct WinMidiData {
  HMIDIIN inHandle;    // Handle to Midi Input Device
  HMIDIOUT outHandle;  // Handle to Midi Output Device
  DWORD lastTime;
  RtMidiIn::MidiMessage message;
  LPMIDIHDR sysexBuffer[RT_SYSEX_BUFFER_COUNT];
};

//*********************************************************************//
//  API: Windows MM
//  Class Definitions: RtMidiIn
//*********************************************************************//

static void CALLBACK midiInputCallback( HMIDIIN hmin,
                                        UINT inputStatus, 
                                        DWORD_PTR instancePtr,
                                        DWORD_PTR midiMessage,
                                        DWORD timestamp )
{
  if ( inputStatus != MIM_DATA && inputStatus != MIM_LONGDATA && inputStatus != MIM_LONGERROR ) return;

  //RtMidiIn::RtMidiInData *data = static_cast<RtMidiIn::RtMidiInData *> (instancePtr);
  RtMidiIn::RtMidiInData *data = (RtMidiIn::RtMidiInData *)instancePtr;
  WinMidiData *apiData = static_cast<WinMidiData *> (data->apiData);

  // Calculate time stamp.
  apiData->message.timeStamp = 0.0;
  if ( data->firstMessage == true ) data->firstMessage = false;
  else apiData->message.timeStamp = (double) ( timestamp - apiData->lastTime ) * 0.001;
  apiData->lastTime = timestamp;

  if ( inputStatus == MIM_DATA ) { // Channel or system message

    // Make sure the first byte is a status byte.
    unsigned char status = (unsigned char) (midiMessage & 0x000000FF);
    if ( !(status & 0x80) ) return;

    // Determine the number of bytes in the MIDI message.
    unsigned short nBytes = 1;
    if ( status < 0xC0 ) nBytes = 3;
    else if ( status < 0xE0 ) nBytes = 2;
    else if ( status < 0xF0 ) nBytes = 3;
    else if ( status == 0xF1 ) {
      if ( data->ignoreFlags & 0x02 ) return;
      else nBytes = 2;
    }
    else if ( status == 0xF2 ) nBytes = 3;
    else if ( status == 0xF3 ) nBytes = 2;
    else if ( status == 0xF8 && (data->ignoreFlags & 0x02) ) {
      // A MIDI timing tick message and we're ignoring it.
      return;
    }
    else if ( status == 0xFE && (data->ignoreFlags & 0x04) ) {
      // A MIDI active sensing message and we're ignoring it.
      return;
    }

    // Copy bytes to our MIDI message.
    unsigned char *ptr = (unsigned char *) &midiMessage;
    for ( int i=0; i<nBytes; ++i ) apiData->message.bytes.push_back( *ptr++ );
  }
  else { // Sysex message ( MIM_LONGDATA or MIM_LONGERROR )
    MIDIHDR *sysex = ( MIDIHDR *) midiMessage; 
    if ( !( data->ignoreFlags & 0x01 ) && inputStatus != MIM_LONGERROR ) {  
      // Sysex message and we're not ignoring it
      for ( int i=0; i<(int)sysex->dwBytesRecorded; ++i )
        apiData->message.bytes.push_back( sysex->lpData[i] );
    }

    // The WinMM API requires that the sysex buffer be requeued after
    // input of each sysex message.  Even if we are ignoring sysex
    // messages, we still need to requeue the buffer in case the user
    // decides to not ignore sysex messages in the future.  However,
    // it seems that WinMM calls this function with an empty sysex
    // buffer when an application closes and in this case, we should
    // avoid requeueing it, else the computer suddenly reboots after
    // one or two minutes.
	if ( apiData->sysexBuffer[sysex->dwUser]->dwBytesRecorded > 0 ) {
    //if ( sysex->dwBytesRecorded > 0 ) {
      MMRESULT result = midiInAddBuffer( apiData->inHandle, apiData->sysexBuffer[sysex->dwUser], sizeof(MIDIHDR) );
      if ( result != MMSYSERR_NOERROR )
        std::cerr << "\nRtMidiIn::midiInputCallback: error sending sysex to Midi device!!\n\n";

      if ( data->ignoreFlags & 0x01 ) return;
    }
    else return;
  }

  if ( data->usingCallback ) {
    RtMidiIn::RtMidiCallback callback = (RtMidiIn::RtMidiCallback) data->userCallback;
    callback( apiData->message.timeStamp, &apiData->message.bytes, data->userData );
  }
  else {
    // As long as we haven't reached our queue size limit, push the message.
    if ( data->queueLimit > data->queue.size() )
      data->queue.push( apiData->message );
    else
      std::cerr << "\nRtMidiIn: message queue limit reached!!\n\n";
  }

  // Clear the vector for the next input message.
  apiData->message.bytes.clear();
}

RtMidiInWinMM :: RtMidiInWinMM( const std::string clientName ) : RtMidiIn()
{
  this->initialize( clientName );
}

void RtMidiInWinMM :: initialize( const std::string& /*clientName*/ )
{
  // We'll issue a warning here if no devices are available but not
  // throw an error since the user can plugin something later.
  unsigned int nDevices = midiInGetNumDevs();
  if ( nDevices == 0 ) {
    errorString_ = "RtMidiIn::initialize: no MIDI input devices currently available.";
    error( RtError::WARNING );
  }

  // Save our api-specific connection information.
  WinMidiData *data = (WinMidiData *) new WinMidiData;
  apiData_ = (void *) data;
  inputData_.apiData = (void *) data;
  data->message.bytes.clear();  // needs to be empty for first input message
}

void RtMidiInWinMM :: openPort( unsigned int portNumber, const std::string /*portName*/ )
{
  if ( connected_ ) {
    errorString_ = "RtMidiIn::openPort: a valid connection already exists!";
    error( RtError::WARNING );
    return;
  }

  unsigned int nDevices = midiInGetNumDevs();
  if (nDevices == 0) {
    errorString_ = "RtMidiIn::openPort: no MIDI input sources found!";
    error( RtError::NO_DEVICES_FOUND );
  }

  std::ostringstream ost;
  if ( portNumber >= nDevices ) {
    ost << "RtMidiIn::openPort: the 'portNumber' argument (" << portNumber << ") is invalid.";
    errorString_ = ost.str();
    error( RtError::INVALID_PARAMETER );
  }

  WinMidiData *data = static_cast<WinMidiData *> (apiData_);
  MMRESULT result = midiInOpen( &data->inHandle,
                                portNumber,
                                (DWORD_PTR)&midiInputCallback,
                                (DWORD_PTR)&inputData_,
                                CALLBACK_FUNCTION );
  if ( result != MMSYSERR_NOERROR ) {
    errorString_ = "RtMidiIn::openPort: error creating Windows MM MIDI input port.";
    error( RtError::DRIVER_ERROR );
  }

  // Allocate and init the sysex buffers.
  for ( int i=0; i<RT_SYSEX_BUFFER_COUNT; ++i ) {
    data->sysexBuffer[i] = (MIDIHDR*) new char[ sizeof(MIDIHDR) ];
    data->sysexBuffer[i]->lpData = new char[ RT_SYSEX_BUFFER_SIZE ];
    data->sysexBuffer[i]->dwBufferLength = RT_SYSEX_BUFFER_SIZE;
    data->sysexBuffer[i]->dwUser = i; // We use the dwUser parameter as buffer indicator
    data->sysexBuffer[i]->dwFlags = 0;

    result = midiInPrepareHeader( data->inHandle, data->sysexBuffer[i], sizeof(MIDIHDR) );
    if ( result != MMSYSERR_NOERROR ) {
      midiInClose( data->inHandle );
      errorString_ = "RtMidiIn::openPort: error starting Windows MM MIDI input port (PrepareHeader).";
      error( RtError::DRIVER_ERROR );
    }

    // Register the buffer.
    result = midiInAddBuffer( data->inHandle, data->sysexBuffer[i], sizeof(MIDIHDR) );
    if ( result != MMSYSERR_NOERROR ) {
      midiInClose( data->inHandle );
      errorString_ = "RtMidiIn::openPort: error starting Windows MM MIDI input port (AddBuffer).";
      error( RtError::DRIVER_ERROR );
    }
  }

  result = midiInStart( data->inHandle );
  if ( result != MMSYSERR_NOERROR ) {
    midiInClose( data->inHandle );
    errorString_ = "RtMidiIn::openPort: error starting Windows MM MIDI input port.";
    error( RtError::DRIVER_ERROR );
  }

  connected_ = true;
}

void RtMidiInWinMM :: openVirtualPort( std::string portName )
{
  // This function cannot be implemented for the Windows MM MIDI API.
  errorString_ = "RtMidiIn::openVirtualPort: cannot be implemented in Windows MM MIDI API!";
  error( RtError::WARNING );
}

void RtMidiInWinMM :: closePort( void )
{
  if ( connected_ ) {
    WinMidiData *data = static_cast<WinMidiData *> (apiData_);
    midiInReset( data->inHandle );
    midiInStop( data->inHandle );

    for ( int i=0; i<RT_SYSEX_BUFFER_COUNT; ++i ) {
      int result = midiInUnprepareHeader(data->inHandle, data->sysexBuffer[i], sizeof(MIDIHDR));
      delete [] data->sysexBuffer[i]->lpData;
      delete [] data->sysexBuffer[i];
      if ( result != MMSYSERR_NOERROR ) {
        midiInClose( data->inHandle );
        errorString_ = "RtMidiIn::openPort: error closing Windows MM MIDI input port (midiInUnprepareHeader).";
        error( RtError::DRIVER_ERROR );
      }
    }

    midiInClose( data->inHandle );
    connected_ = false;
  }
}

RtMidiInWinMM :: ~RtMidiInWinMM()
{
  // Close a connection if it exists.
  closePort();

  // Cleanup.
  WinMidiData *data = static_cast<WinMidiData *> (apiData_);
  delete data;
}

unsigned int RtMidiInWinMM :: getPortCount()
{
  return midiInGetNumDevs();
}

std::string RtMidiInWinMM :: getPortName( unsigned int portNumber )
{
  std::string stringName;
  unsigned int nDevices = midiInGetNumDevs();
  if ( portNumber >= nDevices ) {
    std::ostringstream ost;
    ost << "RtMidiIn::getPortName: the 'portNumber' argument (" << portNumber << ") is invalid.";
    errorString_ = ost.str();
    //error( RtError::INVALID_PARAMETER );
    error( RtError::WARNING );
    return stringName;
  }

  MIDIINCAPS deviceCaps;
  midiInGetDevCaps( portNumber, &deviceCaps, sizeof(MIDIINCAPS));

  // For some reason, we need to copy character by character with
  // UNICODE (thanks to Eduardo Coutinho!).
  //std::string stringName = std::string( deviceCaps.szPname );
  char nameString[MAXPNAMELEN];
  for( int i=0; i<MAXPNAMELEN; ++i )
    nameString[i] = (char)( deviceCaps.szPname[i] );

  stringName = nameString;
  return stringName;
}

//*********************************************************************//
//  API: Windows MM
//  Class Definitions: RtMidiOut
//*********************************************************************//

unsigned int RtMidiOutWinMM :: getPortCount()
{
  return midiOutGetNumDevs();
}

std::string RtMidiOutWinMM :: getPortName( unsigned int portNumber )
{
  std::string stringName;
  unsigned int nDevices = midiOutGetNumDevs();
  if ( portNumber >= nDevices ) {
    std::ostringstream ost;
    ost << "RtMidiOut::getPortName: the 'portNumber' argument (" << portNumber << ") is invalid.";
    errorString_ = ost.str();
    //error( RtError::INVALID_PARAMETER );
    error( RtError::WARNING );
    return stringName;
  }

  MIDIOUTCAPS deviceCaps;
  midiOutGetDevCaps( portNumber, &deviceCaps, sizeof(MIDIOUTCAPS));

  // For some reason, we need to copy character by character with
  // UNICODE (thanks to Eduardo Coutinho!).
  //std::string stringName = std::string( deviceCaps.szPname );
  char nameString[MAXPNAMELEN];
  for( int i=0; i<MAXPNAMELEN; ++i )
    nameString[i] = (char)( deviceCaps.szPname[i] );

  stringName = nameString;
  return stringName;
}

RtMidiOutWinMM :: RtMidiOutWinMM( const std::string clientName ) : RtMidiOut()
{
  this->initialize( clientName );
}

void RtMidiOutWinMM :: initialize( const std::string& /*clientName*/ )
{
  // We'll issue a warning here if no devices are available but not
  // throw an error since the user can plug something in later.
  unsigned int nDevices = midiOutGetNumDevs();
  if ( nDevices == 0 ) {
    errorString_ = "RtMidiOut::initialize: no MIDI output devices currently available.";
    error( RtError::WARNING );
  }

  // Save our api-specific connection information.
  WinMidiData *data = (WinMidiData *) new WinMidiData;
  apiData_ = (void *) data;
}

void RtMidiOutWinMM :: openPort( unsigned int portNumber, const std::string /*portName*/ )
{
  if ( connected_ ) {
    errorString_ = "RtMidiOut::openPort: a valid connection already exists!";
    error( RtError::WARNING );
    return;
  }

  unsigned int nDevices = midiOutGetNumDevs();
  if (nDevices < 1) {
    errorString_ = "RtMidiOut::openPort: no MIDI output destinations found!";
    error( RtError::NO_DEVICES_FOUND );
  }

  std::ostringstream ost;
  if ( portNumber >= nDevices ) {
    ost << "RtMidiOut::openPort: the 'portNumber' argument (" << portNumber << ") is invalid.";
    errorString_ = ost.str();
    error( RtError::INVALID_PARAMETER );
  }

  WinMidiData *data = static_cast<WinMidiData *> (apiData_);
  MMRESULT result = midiOutOpen( &data->outHandle,
                                 portNumber,
                                 (DWORD)NULL,
                                 (DWORD)NULL,
                                 CALLBACK_NULL );
  if ( result != MMSYSERR_NOERROR ) {
    errorString_ = "RtMidiOut::openPort: error creating Windows MM MIDI output port.";
    error( RtError::DRIVER_ERROR );
  }

  connected_ = true;
}

void RtMidiOutWinMM :: closePort( void )
{
  if ( connected_ ) {
    WinMidiData *data = static_cast<WinMidiData *> (apiData_);
    midiOutReset( data->outHandle );
    midiOutClose( data->outHandle );
    connected_ = false;
  }
}

void RtMidiOutWinMM :: openVirtualPort( std::string portName )
{
  // This function cannot be implemented for the Windows MM MIDI API.
  errorString_ = "RtMidiOut::openVirtualPort: cannot be implemented in Windows MM MIDI API!";
  error( RtError::WARNING );
}

RtMidiOutWinMM :: ~RtMidiOutWinMM()
{
  // Close a connection if it exists.
  closePort();

  // Cleanup.
  WinMidiData *data = static_cast<WinMidiData *> (apiData_);
  delete data;
}

void RtMidiOutWinMM :: sendMessage( std::vector<unsigned char> *message )
{
  unsigned int nBytes = static_cast<unsigned int>(message->size());
  if ( nBytes == 0 ) {
    errorString_ = "RtMidiOut::sendMessage: message argument is empty!";
    error( RtError::WARNING );
    return;
  }

  MMRESULT result;
  WinMidiData *data = static_cast<WinMidiData *> (apiData_);
  if ( message->at(0) == 0xF0 ) { // Sysex message

    // Allocate buffer for sysex data.
    char *buffer = (char *) malloc( nBytes );
    if ( buffer == NULL ) {
      errorString_ = "RtMidiOut::sendMessage: error allocating sysex message memory!";
      error( RtError::MEMORY_ERROR );
    }

    // Copy data to buffer.
    for ( unsigned int i=0; i<nBytes; ++i ) buffer[i] = message->at(i);

    // Create and prepare MIDIHDR structure.
    MIDIHDR sysex;
    sysex.lpData = (LPSTR) buffer;
    sysex.dwBufferLength = nBytes;
    sysex.dwFlags = 0;
    result = midiOutPrepareHeader( data->outHandle,  &sysex, sizeof(MIDIHDR) ); 
    if ( result != MMSYSERR_NOERROR ) {
      free( buffer );
      errorString_ = "RtMidiOut::sendMessage: error preparing sysex header.";
      error( RtError::DRIVER_ERROR );
    }

    // Send the message.
    result = midiOutLongMsg( data->outHandle, &sysex, sizeof(MIDIHDR) );
    if ( result != MMSYSERR_NOERROR ) {
      free( buffer );
      errorString_ = "RtMidiOut::sendMessage: error sending sysex message.";
      error( RtError::DRIVER_ERROR );
    }

    // Unprepare the buffer and MIDIHDR.
    while ( MIDIERR_STILLPLAYING == midiOutUnprepareHeader( data->outHandle, &sysex, sizeof (MIDIHDR) ) ) Sleep( 1 );
    free( buffer );

  }
  else { // Channel or system message.

    // Make sure the message size isn't too big.
    if ( nBytes > 3 ) {
      errorString_ = "RtMidiOut::sendMessage: message size is greater than 3 bytes (and not sysex)!";
      error( RtError::WARNING );
      return;
    }

    // Pack MIDI bytes into double word.
    DWORD packet;
    unsigned char *ptr = (unsigned char *) &packet;
    for ( unsigned int i=0; i<nBytes; ++i ) {
      *ptr = message->at(i);
      ++ptr;
    }

    // Send the message immediately.
    result = midiOutShortMsg( data->outHandle, packet );
    if ( result != MMSYSERR_NOERROR ) {
      errorString_ = "RtMidiOut::sendMessage: error sending MIDI message.";
      error( RtError::DRIVER_ERROR );
    }
  }
}
