/**
 * @File asgn2_helper_funcs.h
 *
 * Interfaces provided as starter code for Assignment 2.
 *
 * @author Andrew Quinn
 */

#pragma once

#include <stdint.h>
#include <sys/types.h>

/** @struct Listener_Socket
 *  @brief This structure represents a socket listening for connections
 */
typedef struct {

    /** @brief The socket for the listening connection. Note: do not use
   *         this directly! Take a look at listener_init and
   *         listener_accept instead!
   */
    int fd;
} Listener_Socket;

/** @brief Initializes a listener socket that listens on the provided
 *         port on all of the interfaces for the host.
 *
 *  @param sock The Listener_Socket to initialize.
 *
 *  @param port The port on which to listen.
 *
 *  @return 0, indicating success, or -1, indicating that it failed to
 *          listen.
 */
int listener_init(Listener_Socket *sock, int port);

/** @brief Accept a new connection and initialize a 5 second timeout
 *
 *  @param sock The Listener_Socket from which to get the new
 *              connection.
 *
 *  @return An socket for the new connection, or -1, if there is an
 *          error. Sets errno according to any errors that occur.
 */
int listener_accept(Listener_Socket *sock);

/** @brief Reads bytes from in into buf until either (1) it has read
 *         nbytes, (2) in is out of bytes to return, (3) in times out,
 *         (4) there is an error reading bytes, or (5) buf contains
 *         string.
 *
 *  @param in The file descriptor or socket from which to read.
 *
 *  @param buf The buffer in which to put read data.
 *
 *  @param nbytes The maximum bytes to read.  Must be less than or
 *         equal to the size of buf.
 *
 *  @param string The string to search for, or NULL, indicating that
 *         there is no string to search for.
 *
 *  @return The number of bytes read, or, -1, indicating an error.
 *          Note: this function treats a timeout as an error.  Sets
 *          errno according to any errors that occur.
 */
ssize_t read_until(int in, char buf[], size_t nbytes, char *string);

/** @brief Writes bytes to out from buf until either (1) it has written
 *         exactly nbytes or (2) it encounters an error on write.
 *
 *  @param out The file descriptor or socket to write to.
 *
 *  @param buf The buffer containing data to write.
 *
 *  @param nbytes The number of bytes to write. Must be less than or
 *         equal to the size of buf.
 *
 *  @return The number of bytes written, or, -1, indicating an error.
 *          Sets errno according to any errors that occur.
 */
ssize_t write_all(int out, char buf[], size_t nbytes);

/** @brief Reads bytes from src and places them in dest until either
 *         (1) it has read/written exactly nbytes, (2) read returns 0,
 *         or (3) it encounters an error on read/write.
 *
 *  @param src The file descriptor or socket from which to read.
 *
 *  @param dst The file descriptor or socket to write to.
 *
 *  @param nbytes The number of bytes to read/write. *
 *
 *  @return The number of bytes written, or, -1, indicating an error.
 *          Sets errno according to any errors that occur.
 */
ssize_t pass_bytes(int src, int dst, size_t nbytes);
