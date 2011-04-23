/*
    MIDI Virtual Piano Keyboard
    Copyright (C) 2008-2011, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; If not, see <http://www.gnu.org/licenses/>.
*/

/*
 *  This code is based on Qmidinet, by Rui Nuno Capela: http://qmidinet.sf.net
 */

#if defined(NETWORK_MIDI)

#include <RtMidi.h>
#include <pthread.h>
#include <cstring>
#include <sstream>

#if defined(__SYMBIAN32__)
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>
#else
#if defined(WIN32)
#include <winsock.h>
static WSADATA g_wsaData;
typedef int socklen_t;
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#endif
#endif

static int g_iUdpPort = 21928;

struct NetworkMidiData {
    int socket;
    struct sockaddr_in sockaddr;
    pthread_t thread;
};

/* RtMidiIn */

extern "C" void *networkMidiHandler( void *ptr )
{
    RtMidiIn::RtMidiInData *data = static_cast<RtMidiIn::RtMidiInData *> (ptr);
    NetworkMidiData *apiData = static_cast<NetworkMidiData *> (data->apiData);
    RtMidiIn::MidiMessage message;
    while (data->doInput) {
        int sockin = apiData->socket;
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(sockin, &fds);
        int fdmax = sockin;
        struct timeval tv;
        tv.tv_sec  = 1; // timeout period (1 second)...
        tv.tv_usec = 0;
        int s = ::select(fdmax + 1, &fds, NULL, NULL, &tv);
        if (s < 0) {
            std::cerr << "\nRtMidiIn: select=" << s;
            break;
        }
        if (s == 0) {
            //std::cerr << "\nRtMidiIn: timeout";
            continue;
        }
        // A Network event
        if (FD_ISSET(sockin, &fds)) {
            // Read from network...
            unsigned char buf[1024];
            struct sockaddr_in sender;
            socklen_t slen = sizeof(sender);
            int r = ::recvfrom(sockin, (char *) buf, sizeof(buf),
                        0, (struct sockaddr *) &sender, &slen);
            if (r > 0) {
                message.timeStamp = 0;
                message.bytes.clear();
                for ( unsigned int i = 0; i < r; ++i ) {
                      message.bytes.push_back( buf[i] );
                }
                if ( data->usingCallback ) {
                    RtMidiIn::RtMidiCallback callback = (RtMidiIn::RtMidiCallback) data->userCallback;
                    callback( message.timeStamp, &message.bytes, data->userData );
                } else {
                    if ( data->queueLimit > data->queue.size() ) {
                      data->queue.push( message );
                    } else {
                      std::cerr << "\nRtMidiIn: message queue limit reached!!\n\n";
                    }
                }
            } else if (r < 0) {
                std::cerr << "\nRtMidiIn: recvfrom=" << r;
            }
        }
    }
}

void RtMidiIn :: initialize( const std::string& clientName )
{
    NetworkMidiData *data = new NetworkMidiData;
#if defined(WIN32)
    WSAStartup(MAKEWORD(1, 1), &g_wsaData);
#endif
    apiData_ = (void *) data;
    inputData_.apiData = (void *) data;
}

RtMidiIn :: ~RtMidiIn()
{
    // Close a connection if it exists.
    closePort();
    // Shutdown the input thread.
    NetworkMidiData *data = (NetworkMidiData *) inputData_.apiData ;
    if ( inputData_.doInput ) {
        inputData_.doInput = false;
        pthread_join( data->thread, NULL );
    }
    // Cleanup.
#if defined(WIN32)
    WSACleanup();
#endif
    delete data;
}

void RtMidiIn :: openPort( unsigned int portNumber, const std::string portName )
{
    NetworkMidiData *data = static_cast<NetworkMidiData *> (apiData_);
    closePort();
    // Setup network protocol...
    int protonum = 0;
    data->socket = ::socket(PF_INET, SOCK_DGRAM, protonum);
    if (data->socket < 0) {
        std::cerr << "socket(in)";
        return;
    }
    struct sockaddr_in addrin;
    ::memset(&addrin, 0, sizeof(addrin));
    addrin.sin_family = AF_INET;
    addrin.sin_addr.s_addr = htonl(INADDR_ANY);
    addrin.sin_port = htons(g_iUdpPort);
    if (::bind(data->socket, (struct sockaddr *) (&addrin), sizeof(addrin)) < 0) {
        std::cerr << "bind";
        return;
    }
    // Will Hall, 2007
    // INADDR_ANY will bind to default interface,
    struct in_addr if_addr_in;
    if_addr_in.s_addr = htonl(INADDR_ANY);
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = ::inet_addr("225.0.0.37");
    mreq.imr_interface.s_addr = if_addr_in.s_addr;
    if (::setsockopt (data->socket, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                     (char *) &mreq, sizeof(mreq)) < 0) {
        std::cerr << "setsockopt(IP_ADD_MEMBERSHIP)";
        errorString_ = "RtMidiIn::openPort: OS is probably missing multicast support.";
        error( RtError::SYSTEM_ERROR );
    }
    // start the input thread
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
    inputData_.doInput = true;
    int err = pthread_create(&data->thread, &attr, networkMidiHandler, &inputData_);
    pthread_attr_destroy(&attr);
    if (err) {
        inputData_.doInput = false;
        errorString_ = "RtMidiIn::openPort: error starting MIDI input thread!";
        error( RtError::THREAD_ERROR );
    }
}

void RtMidiIn :: openVirtualPort( const std::string portName )
{
    errorString_ = "RtMidiIn::openVirtualPort: cannot be implemented in UDP!";
    error( RtError::WARNING );
}

unsigned int RtMidiIn :: getPortCount()
{
    return 1;
}

std::string RtMidiIn :: getPortName( unsigned int portNumber )
{
    std::ostringstream ost;
    ost << "UDP/" << g_iUdpPort;
    return ost.str();
}

void RtMidiIn :: closePort()
{
    NetworkMidiData *data = static_cast<NetworkMidiData *> (apiData_);
    if (data->socket >= 0) {
#if defined(WIN32)
        ::closesocket(data->socket);
#else
        ::close(data->socket);
#endif
        data->socket = 0;
    }
}

/* RtMidiOut */

void RtMidiOut :: initialize( const std::string& clientName )
{
    int result;
    NetworkMidiData *data = new NetworkMidiData;
    if ( result < 0 ) {
      delete data;
      errorString_ = "RtMidiOut::initialize: error initializing MIDI event parser!\n\n";
      error( RtError::DRIVER_ERROR );
    }
    apiData_ = (void *) data;
}

RtMidiOut :: ~RtMidiOut()
{
    // Close a connection if it exists.
    closePort();
    NetworkMidiData *data = (NetworkMidiData *) apiData_;
    // Cleanup.
    delete data;
}

void RtMidiOut :: openPort( unsigned int portNumber, const std::string portName )
{
    NetworkMidiData *data = static_cast<NetworkMidiData *> (apiData_);
    int protonum = 0;
    data->socket = ::socket(AF_INET, SOCK_DGRAM, protonum);
    if (data->socket < 0) {
        std::cerr << "socket(out)";
        return;
    }

    ::memset(&data->sockaddr, 0, sizeof(data->sockaddr));
    data->sockaddr.sin_family = AF_INET;
    data->sockaddr.sin_addr.s_addr = ::inet_addr("225.0.0.37");
    data->sockaddr.sin_port = htons(g_iUdpPort);

    // Turn off loopback...
    int loop = 0;
    if (::setsockopt(data->socket, IPPROTO_IP, IP_MULTICAST_LOOP,
                    (char *) &loop, sizeof (loop)) < 0) {
        std::cerr << "setsockopt(IP_MULTICAST_LOOP)";
        return;
    }
}

void RtMidiOut :: openVirtualPort( const std::string portName )
{
    errorString_ = "RtMidiOut::openVirtualPort: cannot be implemented in UDP!";
    error( RtError::WARNING );
}

unsigned int RtMidiOut :: getPortCount()
{
    return 1;
}

std::string RtMidiOut :: getPortName( unsigned int portNumber )
{
    std::ostringstream ost;
    ost << "UDP/" << g_iUdpPort;
    return ost.str();
}

void RtMidiOut :: closePort()
{
    NetworkMidiData *data = static_cast<NetworkMidiData *> (apiData_);
    if (data->socket >= 0) {
#if defined(WIN32)
        ::closesocket(data->socket);
#else
        ::close(data->socket);
#endif
        data->socket = 0;
    }
}

void RtMidiOut :: sendMessage( std::vector<unsigned char> *message )
{
    NetworkMidiData *data = static_cast<NetworkMidiData *> (apiData_);
    if (data->socket < 0) {
        std::cerr << "socket";
        return;
    }
    if (::sendto(data->socket, (char *) message->data(), message->size(), 0,
            (struct sockaddr*) &data->sockaddr, sizeof(data->sockaddr)) < 0) {
        std::cerr << "sendto";
        return;
    }
}

#endif
