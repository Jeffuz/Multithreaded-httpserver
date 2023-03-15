# Multi-Threaded HTTP Server

## Overview

This is a Multi-threaded HTTP server that can serve multiple clients simultaneously. It handles GET and PUT requests and creates an audit log that identifies the linearization of the server. 

## Design Decisions

- Used a thread pool to handle incoming requests, with a fixed number of threads (4).
- Implemented a global queue to store incoming requests and synchronize access to it using a mutex.
- Used POSIX file locks to handle file I/O and avoid race conditions.
- Implemented an audit log to track incoming requests and their responses.

## Functions

- handle_connection(int connfd): Handles a new incoming connection by parsing the request, handling it, and sending the appropriate response.
- handle_get(conn_t *conn): Handles GET requests by getting the URI, opening the file, and sending the file content as a response.
- handle_put(conn_t *conn): Handles PUT requests by getting the URI, opening the file, and writing the content sent in the request to the file.
- handle_unsupported(conn_t *conn, const Request_t *req): Handles unsupported requests by sending a NOT IMPLEMENTED response.
- audit_log(const char *method, conn_t *conn, const Response_t *res): Logs incoming requests and their responses to stderr
- *worker(): Worker thread function that retrieves requests from the global queue and handles them.

## Data Structures

- queue_t: Queue implemented using a circular buffer
- conn_t: A struct that represents a connection, containing the file descriptor, request, and response.
- Request_t: An enum that represents the different types of requests (GET, PUT, etc.).
- Response_t: An enum that represents the different types of responses 

## Modules
- http_helper_funcs.h: Contains helper functions for handling HTTP requests and responses.
- connection.h: Defines the conn_t struct and functions for handling connections.
- debug.h: Contains debugging macros for logging.
- queue.h: Defines the queue_t struct and functions for handling the global queue.
- request.h: Defines the Request_t enum and functions for handling HTTP requests.
- response.h: Defines the Response_t enum and functions for handling HTTP responses.