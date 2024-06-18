#include <iostream>
#include <srtp.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

#include "util.h"

int main() {
    int status;
    // SRTP session for the transmitter
    srtp_t srtp;
    srtp_policy_t policy;
    uint8_t key[SRTP_MASTER_KEY_LEN] = SRTP_MASTER_KEY;
    uint32_t ssrc = 0xdeadbeef;

    printf("Using %s\n", srtp_get_version_string());

    // Initialize SRTP library
    status = srtp_init();
    if (status) {
        printf("error: srtp initialization failed with error code %d\n",
               status);
        exit(1);
    }

    memset(&policy, 0x0, sizeof(srtp_policy_t));
    srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy.rtp);
    srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy.rtcp);
    policy.key = (uint8_t *)key;
    policy.ssrc.type = ssrc_specific;
    policy.ssrc.value = ssrc;
    policy.window_size = 0;
    policy.allow_repeat_tx = false;
    policy.next = NULL;

    if (srtp_create(&srtp, &policy) != srtp_err_status_ok) {
        std::cerr << "Error creating SRTP sender session" << std::endl;
        return -1;
    }

    // Create a UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return -1;
    }

    // Set up the receiver address
    sockaddr_in receiver_addr;
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &receiver_addr.sin_addr);

    // RTP packet (header + payload)
    unsigned char rtp_packet[RTP_HEADER_LEN + 10] = {
        0x80, 0x60, 0x00, 0x01,  // RTP header
        0x00, 0x00, 0x00, 0x01,  // Sequence number and timestamp
        0x00, 0x00, 0x00, 0x01,  // SSRC
        'H', 'e', 'l', 'l', 'o'  // Payload
    };

    // Protect the RTP packet with SRTP
    size_t rtp_packet_len = RTP_HEADER_LEN + 5;  // Header + payload length
    size_t srtp_packet_len = 0;
    if (srtp_protect(srtp, rtp_packet, rtp_packet_len, rtp_packet, &srtp_packet_len, 0) != srtp_err_status_ok) {
        std::cerr << "Error protecting RTP packet" << std::endl;
        return -1;
    }

    // Send the RTP packet
    if (sendto(sockfd, rtp_packet, rtp_packet_len, 0, (sockaddr*)&receiver_addr, sizeof(receiver_addr)) < 0) {
        std::cerr << "Error sending packet" << std::endl;
        return -1;
    }

    std::cout << "RTP packet sent" << std::endl;

    // Clean up
    close(sockfd);
    srtp_dealloc(srtp);
    srtp_shutdown();

    return 0;
}