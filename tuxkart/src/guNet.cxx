
#include <stdio.h>
#include <stdlib.h>
#if defined(WIN32)
 #if defined(__CYGWIN__)
  #include <unistd.h>
 #endif
 #include <windows.h>
 #if defined(_MSC_VER)
  #include <io.h>
  #include <direct.h>
 #endif
#else
 #include <unistd.h>
#endif


#include "guNet.h"

#include <string.h>
#include <sys/types.h>
#if defined(_MSC_VER)
 #include <winsock.h>
#else
 #include <sys/socket.h>
 #include <sys/param.h>
 #include <netinet/in.h>
 #ifndef __CYGWIN__
  #include <netinet/tcp.h>
 #endif
 #include <netdb.h>
 #include <sys/uio.h>
 #include <arpa/inet.h>
 #include <sys/errno.h>
#endif
#include <fcntl.h>

#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif


guUDPConnection::guUDPConnection ()
{
  sockfd   = 0    ;
  in_addr  = NULL ;
  out_addr = NULL ;
}


guUDPConnection::~guUDPConnection ()
{
  disconnect () ;
}


void guUDPConnection::disconnect ()
{
#ifdef WIN32
	return;
#else
  if ( sockfd > 0 )
    shutdown ( sockfd, SHUT_RDWR ) ;

  delete in_addr  ;
  delete out_addr ;
  sockfd   = 0    ;
  in_addr  = NULL ;
  out_addr = NULL ;
#endif
}


int guUDPConnection::connect ( char *hostname, int _port )
{
#ifdef WIN32
	return 1;
#else
  in_addr  = new sockaddr_in ;
  out_addr = new sockaddr_in ;
  port     = _port ;
  sockfd   = socket ( AF_INET, SOCK_DGRAM, IPPROTO_UDP ) ;

  if ( sockfd < 0 )
  {
    perror ( "socket" ) ;
    fprintf ( stderr, "net: Failed to open a net connection.\n" ) ;
    exit ( 1 ) ;
  }

  /*
    We have to pick a transmit port and a receive port.

    We'll pick the lower number to send on if our hostname
    is alphabetically lower than the other machine.
  */

  char myname [ 256 ] ;

  if ( gethostname ( myname, 256 ) != 0 )
  {
    perror ( "gethostname" ) ;
    fprintf ( stderr, "net: Failed to find this machine's hostname.\n" ) ;
    exit ( 1 ) ;
  }

  int delta = strcmp ( myname, hostname ) ;

  if ( delta == 0 )
  {
    fprintf ( stderr, "net: '%s' is this machine!\n", hostname ) ;
    exit ( 1 ) ;
  }

  if ( delta < 0 )
  {
    iport = port   ;
    oport = port+1 ;
  }
  else
  {
    iport = port+1 ;
    oport = port   ;
  }

  memset ( (char *) in_addr, 0, sizeof( sockaddr_in ) ) ;
  in_addr->sin_family      = AF_INET ;
  in_addr->sin_port        = htons ( iport ) ;
  in_addr->sin_addr.s_addr = htonl(INADDR_ANY) ;

  memset ( (char *) out_addr, 0, sizeof ( sockaddr_in ) ) ;
  out_addr->sin_family     = AF_INET ;
  out_addr->sin_port       = htons ( oport ) ;

  hostent *host = gethostbyname ( hostname ) ;

  if ( ! host )
  {
    fprintf ( stderr, "No match for host: '%s'\n", hostname ) ;
    exit ( 1 ) ;
  }
  
  fprintf ( stderr, "Found host %s at %u.%u.%u.%u\n", hostname,
      (unsigned int) host->h_addr_list[0][0] & 0xFF,
      (unsigned int) host->h_addr_list[0][1] & 0xFF,
      (unsigned int) host->h_addr_list[0][2] & 0xFF,
      (unsigned int) host->h_addr_list[0][3] & 0xFF ) ;

  memcpy( & (out_addr->sin_addr), host->h_addr_list[0],
                                  sizeof ( out_addr->sin_addr ) ) ;
  
  fprintf ( stderr,"Looking for other player on %s on port %d/%d\n",
                                              hostname, iport, oport ) ;

  if ( bind ( sockfd, (struct sockaddr *) in_addr, sizeof(sockaddr_in)) <0)
  {
    perror ( "bind" ) ;
    fprintf ( stderr, "net: Failed to bind to port %d.\n", iport ) ;
    exit ( 1 ) ;
  }

  fcntl ( sockfd, F_SETFL, FNDELAY ) ;

  return 1 ;
#endif
}


int guUDPConnection::sendMessage ( char *mesg, int length )
{
#ifdef WIN32
  return 1;
#else
  while ( 1 )
  {
    int r = sendto ( sockfd, mesg, length, 0, (sockaddr *) out_addr,
                                               sizeof(sockaddr_in) ) ;

    if ( r == length )
      return 1 ;

    if ( r < 0 && errno != EAGAIN && errno != EWOULDBLOCK &&
                                     errno != ECONNREFUSED )
    {
      perror ( "Sendto" ) ; 
      fprintf ( stderr, "net: Error in sending data.\n" ) ;
      exit ( 1 ) ;
    }

    if ( r >= 0 )
    {
      perror ( "Sendto" ) ; 
      fprintf ( stderr, "net: Unable to send enough data.\n" ) ;
      exit ( 1 ) ;
    }
  }
#endif
}



int guUDPConnection::recvMessage ( char *mesg, int length )
{
#ifdef WIN32
  return 0;
#else
  unsigned int len = sizeof ( in_addr ) ;

  int r = recvfrom ( sockfd, mesg, length, 0, (sockaddr *) in_addr, (unsigned int *)(&len) );

  if ( r < 0 && errno != EAGAIN      &&
                errno != EWOULDBLOCK &&
                errno != ECONNREFUSED )
    perror ( "RecvFrom" ) ;

  return r ;
#endif
}



