
#ifndef _GUNET_H_
#define _GUNET_H_  1

#define GU_UDP_DEFAULT_PORT_NUMBER  5100

class guUDPConnection
{
  struct sockaddr_in  *in_addr ;
  struct sockaddr_in *out_addr ;

  int sockfd ;
  int  port  ;
  int iport  ;
  int oport  ;

 public:

   guUDPConnection () ;
  ~guUDPConnection () ;
  
  void disconnect  () ;

  int connect      ( char *hostname = "localhost",
                     int   _port    = GU_UDP_DEFAULT_PORT_NUMBER ) ;

  int sendMessage  ( char *mesg, int length ) ;
  int recvMessage  ( char *mesg, int length ) ;
} ;


#endif

