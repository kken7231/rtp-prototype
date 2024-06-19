#include <iostream>
#include <iomanip>
#include <srtp.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <csignal> // For signal handling
#include <string>
#include "util.h"

// Enumeration for program type: sender or receiver
typedef enum
{
    sender,
    receiver,
    unknown
} program_type;

int sockfd = -1; // Socket file descriptor made global for signal handler access

// Signal handler to close the socket and clean up
void handle_signal(int signal) {
    if (sockfd >= 0) {
        close(sockfd);
        std::cout << "Socket closed due to signal " << signal << std::endl;
    }
    exit(signal);
}

uint16_t sequence_number = 0; // Global sequence number

int main(int argc, char *argv[])
{
    // Register signal handler
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    // Handle SIGINT (Ctrl+C) and SIGTERM
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    // SRTP session for the transmitter
    srtp_t session;
    srtp_policy_t policy;
    uint8_t key[SRTP_MASTER_KEY_LEN] = SRTP_MASTER_KEY;
    uint8_t ssrc_bytes[4] = SSRC_VALUE;

    // Print SRTP version
    printf("Using %s\n", srtp_get_version_string());

    // Initialize SRTP library
    srtp_err_status_t status = srtp_init();
    if (status != srtp_err_status_ok)
    {
        std::cerr << "error: srtp initialization failed with error: " << srtp_err_status_to_string(status) << std::endl;
        return -1;
    }

    program_type prog_type = unknown;
    // Parse command-line arguments to determine program type (sender or receiver)
    while (1)
    {
        char c = getopt(argc, argv, "rs");
        if (c == -1)
        {
            break;
        }
        switch (c)
        {
        case 'r':
            prog_type = receiver;
            break;
        case 's':
            prog_type = sender;
            break;
        }
    }
    if (prog_type == unknown)
    {
        std::cerr << "error: set receiver/sender option" << std::endl;
        return -1;
    }

    // Initialize SRTP policy with the given key
    srtp_policy_init(&policy, key);

    // Print the master key and salt
    printf("set master key/salt to %s/%s\n", print_hex(policy.key, 16), print_hex(policy.key + 16, 14));

    // Create SRTP session
    status = srtp_create(&session, &policy);
    if (status != srtp_err_status_ok)
    {
        std::cerr << "Error creating SRTP session: " << srtp_err_status_to_string(status) << std::endl;
        return -1;
    }

    // Create a UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        std::cerr << "Error creating socket" << std::endl;
        return -1;
    }

    // Set up the receiver address
    sockaddr_in receiver_addr;
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(PORT);

    if (prog_type == sender)
    {
        inet_pton(AF_INET, "127.0.0.1", &receiver_addr.sin_addr);

        // RTP packet (header + payload)
        uint8_t rtp_packet[RTP_HEADER_LEN + 1024] = {
            0x80, 0x60, 0x00, 0x00, // RTP header with sequence number
            0x00, 0x00, 0x00, 0x01,                                     // Timestamp
            ssrc_bytes[0], ssrc_bytes[1], ssrc_bytes[2], ssrc_bytes[3]  // SSRC
        };

        // Main loop for sending RTP packets
        while (true) {
            std::string input;
            std::cout << "Enter a message to send: ";
            std::getline(std::cin, input);

            if (input.empty()) {
                break;
            }

            // Set the sequence number in the RTP packet
            rtp_packet[2] = static_cast<uint8_t>(sequence_number >> 8);
            rtp_packet[3] = static_cast<uint8_t>(sequence_number & 0xFF);

            // Increment sequence number for the next packet
            sequence_number++;

            // Copy the input message to the RTP packet payload
            memcpy(rtp_packet + RTP_HEADER_LEN, input.c_str(), input.size());

            // Protect the RTP packet with SRTP
            size_t rtp_packet_len = RTP_HEADER_LEN + input.size(); // Header + payload length
            size_t srtp_packet_len = sizeof(rtp_packet);
            status = srtp_protect(session, rtp_packet, rtp_packet_len, rtp_packet, &srtp_packet_len, 0);
            if (status != srtp_err_status_ok)
            {
                std::cerr << "Error protecting RTP packet: " << srtp_err_status_to_string(status) << std::endl;
                return -1;
            }

            // Send the RTP packet
            if (sendto(sockfd, rtp_packet, srtp_packet_len, 0, (sockaddr *)&receiver_addr, sizeof(receiver_addr)) < 0)
            {
                std::cerr << "Error sending packet" << std::endl;
                return -1;
            }

            std::cout << "RTP packet sent" << std::endl;
        }
    }
    else if (prog_type == receiver)
    {
        receiver_addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(sockfd, (sockaddr *)&receiver_addr, sizeof(receiver_addr)) < 0)
        {
            std::cerr << "Error binding socket" << std::endl;
            return -1;
        }

        unsigned char rtp_packet[1500];
        // Receive RTP packets in a loop until interrupted
        while (true) {
            socklen_t addr_len = sizeof(receiver_addr);
            size_t srtp_packet_len = recvfrom(sockfd, rtp_packet, sizeof(rtp_packet), 0, (sockaddr *)&receiver_addr, &addr_len);
            if (srtp_packet_len < 0)
            {
                std::cerr << "Error receiving packet" << std::endl;
                continue;
            }
            size_t rtp_packet_len = sizeof(rtp_packet);

            // Print the protected payload
            std::cout << "Protected RTP payload: ";
            for (size_t i = RTP_HEADER_LEN; i < srtp_packet_len; ++i)
            {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)rtp_packet[i];
            }
            std::cout << std::endl;

            // Unprotect the RTP packet with SRTP
            if (srtp_unprotect(session, rtp_packet, srtp_packet_len, rtp_packet, &rtp_packet_len) != srtp_err_status_ok)
            {
                std::cerr << "Error unprotecting RTP packet" << std::endl;
                continue;
            }

            // Print the unprotected payload
            std::cout << "Unprotected RTP payload: ";
            for (size_t i = RTP_HEADER_LEN; i < rtp_packet_len; ++i)
            {
                std::cout << rtp_packet[i];
            }
            std::cout << std::endl;
        }
    }

    // Clean up
    close(sockfd);
    std::cout << "Socket closed normally." << std::endl;
    srtp_dealloc(session);
    srtp_shutdown();

    return 0;
}