#ifndef HMAC_H
#define HMAC_H

#include <stdint.h>
#include <stdlib.h>

int hmac_sha1(uint8_t* digest, uint8_t* key, uint8_t* message, size_t keylen, size_t msglen);

#endif /* HMAC_H */
