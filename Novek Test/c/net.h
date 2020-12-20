
#include <sys/types.h>
#include <netdb.h>

void init_sockaddr( struct sockaddr_in *name , const char *hostname , uint16_t port );
