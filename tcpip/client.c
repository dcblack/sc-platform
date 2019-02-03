/* A simple client
 */
#include <strings.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>

void error(char *msg)
{
    perror( msg );
    exit( 0 );
}

int main( int argc, char* argv[] )
{
  int sockfd, portno, n;
  struct sockaddr_in serv_addr;
  struct hostent* server;
  char send_buffer[256];
  char recv_buffer[256];
  if ( argc < 3 ) {
    fprintf( stderr, "usage %s hostname port\n", argv[0] );
    exit( 0 );
  }
  portno = atoi( argv[2] );
  sockfd = socket( AF_INET, SOCK_STREAM, 0 );
  if ( sockfd < 0 ) {
    error( "ERROR opening socket" );
  }
  server = gethostbyname( argv[1] );
  if ( server == NULL ) {
    fprintf( stderr, "ERROR, no such host\n" );
    exit( 0 );
  }
  bzero( ( char* ) &serv_addr, sizeof( serv_addr ) );
  serv_addr.sin_family = AF_INET;
  bcopy( ( char* )server->h_addr,
         ( char* )&serv_addr.sin_addr.s_addr,
         server->h_length );
  serv_addr.sin_port = htons( portno );
  if ( connect( sockfd, ( struct sockaddr* )&serv_addr, sizeof( serv_addr ) ) < 0 ) {
    error( "ERROR connecting" );
  }
  printf( "Please enter the message\n" );
  do {
    printf( ": " );
    bzero( send_buffer, 256 );
    fgets( send_buffer, 255, stdin );
    n = write( sockfd, send_buffer, strlen( send_buffer ) );
    if ( n < 0 ) {
      error( "ERROR writing to socket" );
    }
    bzero( recv_buffer, 256 );
    n = read( sockfd, recv_buffer, 255 );
    if ( n < 0 ) {
      error( "ERROR reading from socket" );
    }
    if ( strncmp( recv_buffer, "Got it", 6 ) != 0 ) {
      printf( "%s", recv_buffer );
      if ( n && recv_buffer[n-1] != '\n' ) {
        puts("");
      }
    }
  } while ( strncmp( send_buffer, "quit", 4 ) != 0 );
  return 0;
}
