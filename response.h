#pragma once

#include <stdint.h>

typedef struct Response Response_t;

extern const Response_t RESPONSE_OK;
extern const Response_t RESPONSE_CREATED;
extern const Response_t RESPONSE_BAD_REQUEST;
extern const Response_t RESPONSE_FORBIDDEN;
extern const Response_t RESPONSE_NOT_FOUND;
extern const Response_t RESPONSE_INTERNAL_SERVER_ERROR;
extern const Response_t RESPONSE_NOT_IMPLEMENTED;
extern const Response_t RESPONSE_VERSION_NOT_SUPPORTED;

uint16_t response_get_code(const Response_t *);
const char *response_get_message(const Response_t *);
