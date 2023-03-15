#pragma once

#include <stdint.h>

typedef struct Request Request_t;

#define NUM_REQUESTS 3
extern const Request_t REQUEST_GET;
extern const Request_t REQUEST_PUT;
extern const Request_t REQUEST_UNSUPPORTED;
extern const Request_t *requests[NUM_REQUESTS];

const char *request_get_str(const Request_t *);
