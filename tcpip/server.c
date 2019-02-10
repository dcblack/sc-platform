/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <strings.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

void error( char* msg )
{
  perror( msg );
  exit( 1 );
}

int main( int argc, char* argv[] )
{
  int sockfd = -1;
  int newsockfd = -1;
  unsigned int portno = 28000;
  unsigned int clilen;
  char recv_buffer[256];
  struct sockaddr_in serv_addr, cli_addr;
  int n;
  if ( argc < 2 ) {
    fprintf( stderr, "Using default port number %d\n", portno );
  } else {
    portno = atoi( argv[1] );
  }
  //--------------------------------------------------------------------------
  // Create TCP/IP socket
  //--------------------------------------------------------------------------
  sockfd = socket( AF_INET, SOCK_STREAM, 0 );
  if ( sockfd < 0 ) {
    error( "ERROR opening socket" );
  }
  //--------------------------------------------------------------------------
  // Prepare the server address sockaddr_in structure
  //--------------------------------------------------------------------------
  bzero( ( char* ) &serv_addr, sizeof( serv_addr ) );
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons( portno ); // Host to Network -- big-endian conversion
  //--------------------------------------------------------------------------
  // Bind to incoming address in preparation to listening
  //--------------------------------------------------------------------------
  if ( bind( sockfd, ( struct sockaddr* ) &serv_addr,
             sizeof( serv_addr ) ) < 0 ) {
    error( "ERROR on binding" );
  }
  //--------------------------------------------------------------------------
  // Listen for a connection
  //--------------------------------------------------------------------------
  listen( sockfd, 5 );
  clilen = sizeof( cli_addr );
  newsockfd = accept( sockfd, ( struct sockaddr* ) &cli_addr, &clilen );
  if ( newsockfd < 0 ) {
    error( "ERROR on accept" );
  }
  do {
    bzero( recv_buffer, 256 );
    n = read( newsockfd, recv_buffer, 255 );
    if ( n < 0 ) {
      error( "ERROR reading from socket" );
    }
    printf( "> %s", recv_buffer );
    if ( n && recv_buffer[n-1] != '\n' ) puts("");
    n = write( newsockfd, "Got it", 6 );
    if ( n < 0 ) {
      error( "ERROR writing to socket" );
    }
  } while ( strncmp( recv_buffer, "quit", 4 ) != 0 );
  return 0;
}
