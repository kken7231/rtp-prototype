#include <iostream>
#include <srtp.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

#define SRTP_MASTER_KEY_LEN 30
#define RTP_HEADER_LEN 12

#ifndef PORT
#define PORT 5000
#endif

int main() {
    // Initialize SRTP library
    srtp_init();

    // SRTP session for the receiver
    srtp_t srtp_receiver;
    srtp_policy_t policy_receiver;
    unsigned char key_receiver[SRTP_MASTER_KEY_LEN] = { /* Your 30-byte key here */ };

    srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy_receiver.rtp);
    srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy_receiver.rtcp);
    policy_receiver.ssrc.type = ssrc_any_inbound;
    policy_receiver.ssrc.value = 0;
    policy_receiver.key = key_receiver;
    policy_receiver.next = NULL;

    if (srtp_create(&srtp_receiver, &policy_receiver) != srtp_err_status_ok) {
        std::cerr << "Error creating SRTP receiver session" << std::endl;
        return -1;
    }

    // Create a UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return -1;
    }

    // Bind the socket to a port
    sockaddr_in receiver_addr;
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(PORT);
    receiver_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (sockaddr*)&receiver_addr, sizeof(receiver_addr)) < 0) {
        std::cerr << "Error binding socket" << std::endl;
        return -1;
    }

    // Receive the RTP packet
    unsigned char rtp_packet[1500];
    socklen_t addr_len = sizeof(receiver_addr);
    size_t srtp_packet_len = recvfrom(sockfd, rtp_packet, sizeof(rtp_packet), 0, (sockaddr*)&receiver_addr, &addr_len);
    if (srtp_packet_len < 0) {
        std::cerr << "Error receiving packet" << std::endl;
        return -1;
    }
    size_t rtp_packet_len = 0;

    // Unprotect the RTP packet with SRTP
    if (srtp_unprotect(srtp_receiver, rtp_packet, srtp_packet_len, rtp_packet, &rtp_packet_len) != srtp_err_status_ok) {
        std::cerr << "Error unprotecting RTP packet" << std::endl;
        return -1;
    }

    // Print the payload
    std::cout << "Received RTP payload: ";
    for (int i = RTP_HEADER_LEN; i < rtp_packet_len; ++i) {
        std::cout << rtp_packet[i];
    }
    std::cout << std::endl;

    // Clean up
    close(sockfd);
    srtp_dealloc(srtp_receiver);
    srtp_shutdown();

    return 0;
}