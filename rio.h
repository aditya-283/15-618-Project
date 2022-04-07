/* Imported from 18-613: Foundations of Computer Systems (Lab-7 Proxy Lab) */

#ifndef RIO_H
#define RIO_H

#include <stddef.h>    /* size_t */ 
#include <sys/types.h> /* ssize_t */
/* Persistent state for the robust I/O (Rio) package */ 
#define RIO_BUFSIZE 8192 



typedef struct 
{ 
    int rio_fd;                /* Descriptor for this internal buf */ 
    ssize_t rio_cnt;           /* Unread bytes in internal buf */ 
    char *rio_bufptr;          /* Next unread byte in internal buf */ 
    char rio_buf[RIO_BUFSIZE]; /* Internal buffer */ 
} rio_t;

/* Rio (Robust I/O) package */ 
ssize_t rio_readn(int fd, void *usrbuf, size_t n); 
ssize_t rio_writen(int fd, const void *usrbuf, size_t n); 
void rio_readinitb(rio_t *rp, int fd); 
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n); 
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);

#endif /* RIO_H */
