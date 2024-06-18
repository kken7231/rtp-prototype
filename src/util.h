#ifndef __UTIL_H__
#define __UTIL_H__

#include <srtp.h>

#define SRTP_MASTER_KEY_LEN 30

#ifndef SRTP_MASTER_KEY
#define SRTP_MASTER_KEY \
    {                   \
    }
#endif

#define RTP_HEADER_LEN 12

#ifndef PORT
#define PORT 5000
#endif

int initialize(strp_t *srtp, srtp_policy_t *policy) {};

#endif