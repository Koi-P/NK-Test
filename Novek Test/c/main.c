
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "net.h"

#include <zlib.h>

#define SERVER_IP "51.11.50.70"
#define SERVER_PORT 5050
#define BUFF_SIZE 256

int main(){

    int sock;

    struct sockaddr_in servername;
    char buff[BUFF_SIZE], *eol_marker, *ptr;
    char *line;
    int i, count, line_size, done;
    unsigned long decompressed_size;

    /* Create the socket. */
    sock = socket (PF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
      perror ("socket (client)");
      exit (EXIT_FAILURE);
    }

    /* Connect to the server. */
    init_sockaddr (&servername, SERVER_IP, SERVER_PORT );
    if ( 0 > connect (sock, (struct sockaddr *) &servername, sizeof (servername) ) )
    {
        perror ("connect (client)");
        exit (EXIT_FAILURE);
    }

    /* Initialize line string buffer variables */
    line_size = 0;
    line = NULL;

    done = 0;
    while ( !done && ( count = read( sock , buff , BUFSIZ - 1 ) ) > 0 ){

        buff[count] = '\0';
        ptr = eol_marker = buff;

        while( ( eol_marker = strchr( eol_marker , '\n' ) ) != NULL ){
            *eol_marker++ = '\0';

            /* Check line string buffer size and extend it if necessary */
            while( line == NULL || ( strlen(ptr) + strlen(line) ) >= line_size ){
                line_size += BUFF_SIZE;
                line = realloc( line , line_size );
                if(line == NULL){
                    perror ("Memory(Realloc)");
                    exit (EXIT_FAILURE);
                }
                /* Clear possible appended string data */
                line[ line_size - BUFF_SIZE ] = '\0';
            }
            strcat( line , ptr );

            /* Echo received data */
            puts("Received line:");
            puts(line);

            count = strlen(line);
            if( count % 2 == 0 ){

                /* Covert line from hex*/
                for( i = 0 ; i < count ; i++ ){

                    if(line[i] >= '0' && line[i] <= '9')
                        line[i] = line[i] - '0';
                    else if(line[i] >= 'a' && line[i] <= 'f')
                        line[i] = 10 + line[i] - 'a';
                    else if(line[i] >= 'A' && line[i] <= 'F')
                        line[i] = 10 + line[i] - 'A';
                    else
                        break;

                    line[i / 2] = i % 2 == 0 ? line[i] << 4 : line[i/2] | line[i];
                }

                decompressed_size = BUFF_SIZE;
                /* If valid hex string attempt decompression */
                if( i && i == count){

                    if( uncompress( (unsigned char *)buff , &decompressed_size , (unsigned char *)line , count / 2 ) == Z_OK ){
                        puts("Decompressed data: ");
                        for(i = 0 ; i < decompressed_size / 2 && i < BUFF_SIZE ; i++ )
                            printf("%02x", (unsigned char) buff[i]);
                        puts("");
                    }
                }
            }

            /*Clear line string buffer */
            line[0] = '\0';
            ptr = eol_marker;
        }


        /* Append partial line(if any) into line string buffer */
        if( ( ptr - buff ) != count ){
            /* Check line string buffer size and extend it if necessary */
            while( line == NULL || ( strlen(ptr) + strlen(line) ) >= line_size ){
                line_size += BUFF_SIZE;
                line = realloc( line , line_size );
                if(line == NULL){
                    perror ("Memory(Realloc)");
                    exit (EXIT_FAILURE);
                }
                /* Clear possible appended string data */
                line[ line_size - BUFF_SIZE ] = '\0';
            }
            strcat( line , ptr );
        }

    }

    close (sock);
    exit (EXIT_SUCCESS);

    return 0;
}


