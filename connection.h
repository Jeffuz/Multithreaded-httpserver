#pragma once

#include "response.h"
#include "request.h"

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct Conn conn_t;

// Constructor
conn_t *conn_new(int connfd);

// Destructor
void conn_delete(conn_t **conn);

// Parse the data from connection. Checks static correctness (i.e.,
// that each field fits within our required bounds), but does not
// check for semantic correctness (e.g., does not check that a URI is
// not a directory).
const Response_t *conn_parse(conn_t *conn);

//////////////////////////////////////////////////////////////////////
// Functions that get stuff we might need elsewhere from a connection

// Return the RequestType from parsing.
const Request_t *conn_get_request(conn_t *conn);

// Return URI from parsing.
char *conn_get_uri(conn_t *conn);

// Return the value for the header field named header.  Only
// implemented for header named "Content-Length" and "Request-Id".
char *conn_get_header(conn_t *conn, char *header);

//////////////////////////////////////////////////////////////////////
// Functions that help get data from a connection

// write the data form the connection into the file (fd).
const Response_t *conn_recv_file(conn_t *conn, int fd);

//////////////////////////////////////////////////////////////////////
// Functions that help write responses to the client:

// send a message body from the file (fd)
const Response_t *conn_send_file(conn_t *conn, int fd, uint64_t count);

// send canonical message for a response type
const Response_t *conn_send_response(conn_t *conn, const Response_t *res);

//Functions for debugging:
char *conn_str(conn_t *conn);
