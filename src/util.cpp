#include "util.h"
#include <iostream>
#include <iomanip>
#include <srtp.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

// Function to print a byte array as a hexadecimal string
const char* print_hex(const uint8_t *str, size_t length) {
    std::ostringstream oss;
    for (size_t i = 0; i < length; i++) {
        // Convert each byte to a two-digit hex value
        oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)str[i];
    }
    // Allocate memory for the C-style string
    std::string hexString = oss.str();
    char* cstr = new char[hexString.length() + 1];
    std::strcpy(cstr, hexString.c_str());
    return cstr;
}

// Function to convert SRTP error status to a human-readable string
const char* srtp_err_status_to_string(const srtp_err_status_t status) {
    switch (status) {
        case srtp_err_status_ok:
            return "nothing to report";
        case srtp_err_status_fail:
            return "unspecified failure";
        case srtp_err_status_bad_param:
            return "unsupported parameter";
        case srtp_err_status_alloc_fail:
            return "couldn't allocate memory";
        case srtp_err_status_dealloc_fail:
            return "couldn't deallocate properly";
        case srtp_err_status_init_fail:
            return "couldn't initialize";
        case srtp_err_status_terminus:
            return "can't process as much data as requested";
        case srtp_err_status_auth_fail:
            return "authentication failure";
        case srtp_err_status_cipher_fail:
            return "cipher failure";
        case srtp_err_status_replay_fail:
            return "replay check failed (bad index)";
        case srtp_err_status_replay_old:
            return "replay check failed (index too old)";
        case srtp_err_status_algo_fail:
            return "algorithm failed test routine";
        case srtp_err_status_no_such_op:
            return "unsupported operation";
        case srtp_err_status_no_ctx:
            return "no appropriate context found";
        case srtp_err_status_cant_check:
            return "unable to perform desired validation";
        case srtp_err_status_key_expired:
            return "can't use key any more";
        case srtp_err_status_socket_err:
            return "error in use of socket";
        case srtp_err_status_signal_err:
            return "error in use POSIX signals";
        case srtp_err_status_nonce_bad:
            return "nonce check failed";
        case srtp_err_status_read_fail:
            return "couldn't read data";
        case srtp_err_status_write_fail:
            return "couldn't write data";
        case srtp_err_status_parse_err:
            return "error parsing data";
        case srtp_err_status_encode_err:
            return "error encoding data";
        case srtp_err_status_semaphore_err:
            return "error while using semaphores";
        case srtp_err_status_pfkey_err:
            return "error while using pfkey";
        case srtp_err_status_bad_mki:
            return "error MKI present in packet is invalid";
        case srtp_err_status_pkt_idx_old:
            return "packet index is too old to consider";
        case srtp_err_status_pkt_idx_adv:
            return "packet index advanced, reset needed";
        case srtp_err_status_buffer_small:
            return "out buffer is too small";
        default:
            return "unknown error status";
    }
}

// Function to initialize an SRTP policy with the given key
void srtp_policy_init(srtp_policy_t *policy, uint8_t *key)
{
    // Clear the policy structure
    memset(policy, 0x0, sizeof(srtp_policy_t));
    // Set the RTP crypto policy to use AES_CM_128 with no authentication
    srtp_crypto_policy_set_aes_cm_128_null_auth(&policy->rtp);
    // Set the RTCP crypto policy to the default
    srtp_crypto_policy_set_rtcp_default(&policy->rtcp);
    // Set the SSRC value
    uint32_t ssrc_bytes[4] = SSRC_VALUE;
    policy->ssrc.type = ssrc_specific;
    policy->ssrc.value = ssrc_bytes[0] << 24 | ssrc_bytes[1] << 16 | ssrc_bytes[2] << 8 | ssrc_bytes[3];
    // Assign the key to the policy
    policy->key = key;
    // Set the next policy to NULL
    policy->next = NULL;
}