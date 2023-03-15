// Library and Custom Headers
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "asgn2_helper_funcs.h"
#include "connection.h"
#include "debug.h"
#include "queue.h"
#include "request.h"
#include "response.h"

// Global Variables
#define MAXLINE     1024
#define NUM_THREADS 4
#define OPTIONS     "t:"

// Global Queue
queue_t *queue;

// Mutex
pthread_mutex_t qlock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t audit = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t plock = PTHREAD_MUTEX_INITIALIZER;

// Function prototypes
void handle_connection(int);
void handle_get(conn_t *);
void handle_put(conn_t *);
void handle_unsupported(conn_t *, const Request_t *);
void audit_log(const char *, conn_t *, const Response_t *);
void *worker();

// Handles a new connection
void handle_connection(int connfd) {
    // Create a new connection object
    conn_t *conn = conn_new(connfd);

    // Parse incoming request
    const Response_t *res = conn_parse(conn);

    // Handle incoming request
    if (res != NULL) {
        // Send response if parsing was successful
        conn_send_response(conn, res);

    } else {
        // If parsing failed, get the request object and handle the request
        // debug("%s", conn_str(conn));
        const Request_t *req = conn_get_request(conn);

        // Handle GET request
        if (req == &REQUEST_GET) {
            handle_get(conn);
        }
        // Handle PUT request
        else if (req == &REQUEST_PUT) {
            handle_put(conn);
        }
        // Handle unsupported request
        else {
            handle_unsupported(conn, req);
        }
    }

    // Delete the connection object
    conn_delete(&conn);
}

// Handles GET requests
void handle_get(conn_t *conn) {

    // Get the URI
    char *uri = conn_get_uri(conn);
    // debug("GET request not implemented. But, we want to get %s", uri);

    const Response_t *res = NULL;

    // Lock Global
    pthread_mutex_lock(&qlock);
    // Open the file.
    int fd = open(uri, O_RDONLY, 0666);
    if (fd < 0) {
        //  Cannot access
        if (errno == EACCES) {
            res = &RESPONSE_FORBIDDEN;
            //  Cannot find the file
        } else if (errno == ENOENT) {
            res = &RESPONSE_NOT_FOUND;
            //  other error?
        } else {
            res = &RESPONSE_INTERNAL_SERVER_ERROR;
        }
        // Unlock Global
        pthread_mutex_unlock(&qlock);
        goto out_get;
    }

    // Lock file for writing
    flock(fd, LOCK_SH);
    // Unlock Global
    pthread_mutex_unlock(&qlock);

    // Get the size of the file.
    struct stat st;
    if (fstat(fd, &st) < 0) {
        res = &RESPONSE_INTERNAL_SERVER_ERROR;
        goto out_get;
    }
    uint64_t size = st.st_size;

    // Check if the file is a directory
    if (S_ISDIR(st.st_mode)) {
        res = &RESPONSE_FORBIDDEN;
        goto out_get;
    }

    // Send the file
    res = conn_send_file(conn, fd, size);
    if (res == NULL) {
        res = &RESPONSE_OK;
        audit_log("GET", conn, res);
    }
    close(fd);
    return;
out_get:

    // Log and send response
    audit_log("GET", conn, res);
    conn_send_response(conn, res);
    close(fd);
}

// Handles unsupported requests
void handle_unsupported(conn_t *conn, const Request_t *req) {
    // debug("handling unsupported request");

    // send responses
    conn_send_response(conn, &RESPONSE_NOT_IMPLEMENTED);
    // Log unsupported request to the audit log.
    const char *method = request_get_str(req);
    audit_log(method, conn, &RESPONSE_NOT_IMPLEMENTED);
}

// Handles PUT requests
void handle_put(conn_t *conn) {

    // Get the URI from the connection.
    char *uri = conn_get_uri(conn);

    // Initialize a null response.
    const Response_t *res = NULL;

    // Log that the server is handling the "put" request for the given URI.
    // debug("handling put request for %s", uri);

    // Check if the file already exists before attempting to open it.
    bool existed = access(uri, F_OK) == 0;
    // debug("%s existed? %d", uri, existed);

    // Lock global
    pthread_mutex_lock(&plock);

    // Open the file in write mode, creating it if it doesn't exist and setting permissions to 0600.
    int fd = open(uri, O_CREAT | O_WRONLY, 0600);

    // If opening the file fails, set the response to an appropriate error and skip to sending the response.
    if (fd < 0) {
        // debug("%s: %d", uri, errno);
        if (errno == EACCES || errno == EISDIR || errno == ENOENT) {
            res = &RESPONSE_FORBIDDEN;
            pthread_mutex_unlock(&plock);
            goto out;
        } else {
            res = &RESPONSE_INTERNAL_SERVER_ERROR;
            pthread_mutex_unlock(&plock);
            goto out;
        }
    }

    // Lock file for writing
    flock(fd, LOCK_EX);
    // Clear file content
    ftruncate(fd, 0);
    // Unlock Global Lock
    pthread_mutex_unlock(&plock);

    // Receive the file data from the client and set the response based on whether the file already existed or not.
    res = conn_recv_file(conn, fd);
    if (res == NULL && existed) {
        res = &RESPONSE_OK;
    } else if (res == NULL && !existed) {
        res = &RESPONSE_CREATED;
    }

    audit_log("PUT", conn, &RESPONSE_OK);
    close(fd);
    conn_send_response(conn, res);
    return;

out:
    // Send the response to the client
    audit_log("PUT", conn, &RESPONSE_OK);
    conn_send_response(conn, res);
    close(fd);
}

// Handles the audit_log
void audit_log(const char *method, conn_t *conn, const Response_t *res) {
    // Lock the mutex
    pthread_mutex_lock(&audit);
    // Get the request URI and ID
    const char *uri = conn_get_uri(conn);
    const char *request_id = conn_get_header(conn, "Request-Id");
    // Set request id to 0 if not found
    if (request_id == NULL) {
        request_id = "0";
    }
    // Get Status Code
    uint16_t status_code = response_get_code(res);
    // Log the entry to stderr
    fprintf(stderr, "%s,%s,%d,%s\n", method, uri, status_code, request_id);
    // Unlock the mutex
    pthread_mutex_unlock(&audit);
    return;
}

// Worker Threads
void *worker() {
    uintptr_t socket_fd = 0;
    while (1) {
        // Dequeue
        queue_pop(queue, (void **) &socket_fd);
        handle_connection(socket_fd);
        close(socket_fd);
    }
}

// Main
int main(int argc, char **argv) {
    // Check for correct number of command line arguments
    if (argc < 2) {
        warnx("wrong arguments: %s port_num", argv[0]);
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Initialize variables
    int opt = 0;
    int num_threads = NUM_THREADS;
    size_t port;
    size_t num_port;
    char *endptr;

    // Parse command-line options
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 't':
            // Set number of threads
            num_threads = atoi(optarg);
            port = (size_t) strtol(argv[3], NULL, 10);
            break;
        default:
            // Parse port number
            num_port = strtol(argv[optind], &endptr, 10);
            if (endptr && *endptr != '\0') {
                // Check for invalid port number
                warnx("invalid port number: %s", argv[optind]);
                return EXIT_FAILURE;
            }
            port = (size_t) num_port;
            break;
        }
    }

    // Ignore SIGPIPE signals
    signal(SIGPIPE, SIG_IGN);

    // Initialize listener socket
    Listener_Socket sock;
    listener_init(&sock, port);
    queue = queue_new(num_threads);

    // Create Worker Threads
    pthread_t worker_threads[num_threads];
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&worker_threads[i], NULL, worker, NULL);
    }

    // Continuously accept new connections
    while (1) {
        // Accept Connection and add to queue
        uintptr_t connfd = (uintptr_t) listener_accept(&sock);
        queue_push(queue, (void *) connfd);
    }
    return EXIT_SUCCESS;
}
