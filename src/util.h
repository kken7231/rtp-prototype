#ifndef __UTIL_H__
#define __UTIL_H__

#include <srtp.h>

// Define the length of the SRTP master key
#define SRTP_MASTER_KEY_LEN 30

// Define the length of the RTP header
#define RTP_HEADER_LEN 12

// Define the SRTP master key if not already defined
#ifndef SRTP_MASTER_KEY
#define SRTP_MASTER_KEY {0x9e, 0x38, 0x7a, 0x94, 0x01, 0x19, 0x05, 0xd6, 0x44, 0xe9, 0x44, 0xa1, 0x3d, 0xf6, 0x7b, 0x4a, 0xbb, 0x85, 0x44, 0xee, 0xee, 0x57, 0xc7, 0x64, 0xc4, 0x2e, 0x7f, 0x60, 0x33, 0x84}
#endif

// Define the SSRC value if not already defined
#ifndef SSRC_VALUE
#define SSRC_VALUE {0xde, 0xad, 0xbe, 0xef}
#endif

// Define the port number if not already defined
#ifndef PORT
#define PORT 5000
#endif

// Define the length of the RTP buffer
#define RTP_BUF_LEN 1024

// Function to print a hexadecimal string
const char* print_hex(const uint8_t *str, size_t length);

// Function to convert SRTP error status to string
const char* srtp_err_status_to_string(const srtp_err_status_t status);

// Function to initialize SRTP policy with a given key
void srtp_policy_init(srtp_policy_t *policy, uint8_t *key);

#endif // __UTIL_H__