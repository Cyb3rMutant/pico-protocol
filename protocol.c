#include "protocol.h"
#include "pico/stdlib.h"
#include "tests.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

const int LED_PIN = 25;
int connected;

/**
 * @brief Computes CRC (Cyclic Redundancy Check) for a given data buffer.
 *
 * @param data Pointer to the data buffer.
 * @param len Length of the data buffer.
 * @return The computed CRC value.
 *
 * @note This function calculates CRC using the polynomial 0x07 (CRC-8).
 */
uint8_t compute_crc(uint8_t *data, size_t len) {
    // Initialize CRC
    uint8_t crc = 0;

    // Iterate through each byte in the data
    for (size_t i = 0; i < len; i++) {
        // XOR CRC with current data byte
        crc ^= data[i];

        // Perform polynomial division
        for (int j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07; // Polynomial: x^8 + x^2 + x + 1 (0x07)
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

/**
 * @brief Initializes the communication module.
 *
 * @return None.
 *
 * @note This function initializes the communication module, configures
 *       standard I/O, sets the direction of the LED pin, and initializes
 *       the connected variable.
 */
void protocol_init(void) {
    // Initialize standard I/O
    stdio_init_all();
    // Set translation mode
    stdio_set_translate_crlf(&stdio_usb, false);
    // Initialize LED pin
    gpio_init(LED_PIN);
    // Set LED pin direction to output
    gpio_set_dir(LED_PIN, GPIO_OUT);
    // Initialize connected variable
    connected = 0;
}
/**
 * @brief Opens a connection for communication.
 *
 * @return 0 upon successful connection establishment.
 *
 * @note This function sets the connected flag, sends an open signal,
 *       and turns on the LED indicating a successful connection.
 */
int protocol_connect() {
    // Set connected flag
    connected = 1;
    // Send open signal
    protocol_send_open();
    // Turn on LED
    gpio_put(LED_PIN, 1);
    return 0;
}

/**
 * @brief Sends data over an established connection.
 *
 * @param payload Pointer to the data to be sent.
 * @param payload_length Length of the data payload.
 * @return The number of bytes sent.
 *
 * @note This function constructs a packet consisting of header, payload,
 *       and footer. It computes CRC for the packet and sends it over the
 *       established connection. For demonstration purposes, the packet is
 *       printed to standard output.
 */
int protocol_send(const uint8_t *payload, size_t payload_length) {
    // Calculate the total packet length including payload, header, and footer
    size_t packet_length = payload_length + 7;
    // Create a buffer for the packet
    uint8_t packet[packet_length];
    // Header contains: start byte, packet length (high byte), packet length
    // (low byte), protocol version, and command
    uint8_t header[5] = {0xAA, packet_length >> 8, packet_length, 2, 'd'};
    // Footer contains: CRC (to be filled later)
    uint8_t footer[2] = {0, 0xBB};
    // Copy header into the packet buffer
    memcpy(packet, header, 5);
    // Copy payload into the packet buffer
    memcpy(packet + 5, payload, payload_length);
    // Copy footer into the packet buffer
    memcpy(packet + 5 + payload_length, footer, 2);

    // Compute CRC for the packet
    uint8_t crc = compute_crc(packet, packet_length);
    // Insert computed CRC into the packet
    packet[5 + payload_length] = crc;

    // Send packet by iterating through each byte in the packet
    for (size_t i = 0; i < packet_length; i++) {
        putchar(packet[i]);
    }
    // Return the total packet length
    return packet_length;
}
/**
 * @brief Sends an acknowledgment packet over the connection.
 *
 * @param err Error code to be included in the acknowledgment packet.
 * @return The number of bytes sent.
 *
 * @note This function constructs an acknowledgment packet with a header,
 *       error code payload, and footer. It computes CRC for the packet
 *       and sends it over the connection. For demonstration purposes,
 *       the packet is printed to standard output.
 */
int protocol_send_ack(int err) {
    // Calculate the total packet length including header and footer
    size_t packet_length = 1 + 7;
    // Create a buffer for the packet
    uint8_t packet[packet_length];
    // Header contains: start byte, packet length (high byte), packet length
    // (low byte), protocol version, and command
    uint8_t header[5] = {0xAA, packet_length >> 8, packet_length, 2, 'a'};
    // Footer contains: CRC (to be filled later)
    uint8_t footer[2] = {0, 0xBB};
    // Copy header into the packet buffer
    memcpy(packet, header, 5);
    // Insert error code into the packet
    packet[5] = err;
    // Copy footer into the packet buffer
    memcpy(packet + 5 + 1, footer, 2);

    // Compute CRC for the packet
    uint8_t crc = compute_crc(packet, packet_length);
    // Insert computed CRC into the packet
    packet[5 + 1] = crc;

    // Send packet by iterating through each byte in the packet
    for (size_t i = 0; i < packet_length; i++) {
        putchar(packet[i]);
    }
    // Return the total packet length
    return packet_length;
}

/**
 * @brief Sends an open signal over the connection.
 *
 * @return The number of bytes sent.
 *
 * @note This function constructs a packet with a header indicating an open
 *       signal and a footer. It computes CRC for the packet and sends it
 *       over the connection. For demonstration purposes, the packet is
 *       printed to standard output.
 */
int protocol_send_open() {
    // Calculate the total packet length including header and footer
    size_t packet_length = 7;
    // Create a buffer for the packet
    uint8_t packet[packet_length];
    // Header contains: start byte, packet length (high byte), packet length
    // (low byte), protocol version, and command
    uint8_t header[5] = {0xAA, packet_length >> 8, packet_length, 2, 'o'};
    // Footer contains: CRC (to be filled later)
    uint8_t footer[2] = {0, 0xBB};
    // Copy header into the packet buffer
    memcpy(packet, header, 5);
    // Copy footer into the packet buffer
    memcpy(packet + 5, footer, 2);

    // Compute CRC for the packet
    uint8_t crc = compute_crc(packet, packet_length);
    // Insert computed CRC into the packet
    packet[5] = crc;

    // Send packet by iterating through each byte in the packet
    for (size_t i = 0; i < packet_length; i++) {
        putchar(packet[i]);
    }
    // Return the total packet length
    return packet_length;
}

/**
 * @brief Sends a close signal over the connection.
 *
 * @return The number of bytes sent.
 *
 * @note This function constructs a packet with a header indicating a close
 *       signal and a footer. It computes CRC for the packet and sends it
 *       over the connection. For demonstration purposes, the packet is
 *       printed to standard output.
 */
int protocol_send_close() {
    // Calculate the total packet length including header and footer
    size_t packet_length = 7;
    // Create a buffer for the packet
    uint8_t packet[packet_length];
    // Header contains: start byte, packet length (high byte), packet length
    // (low byte), protocol version, and command
    uint8_t header[5] = {0xAA, packet_length >> 8, packet_length, 2, 'c'};
    // Footer contains: CRC (to be filled later)
    uint8_t footer[2] = {0, 0xBB};
    // Copy header into the packet buffer
    memcpy(packet, header, 5);
    // Copy footer into the packet buffer
    memcpy(packet + 5, footer, 2);

    // Compute CRC for the packet
    uint8_t crc = compute_crc(packet, packet_length);
    // Insert computed CRC into the packet
    packet[5] = crc;

    // Send packet by iterating through each byte in the packet
    for (size_t i = 0; i < packet_length; i++) {
        putchar(packet[i]);
    }
    // Return the total packet length
    return packet_length;
}

/**
 * @brief Sends an echo packet over the connection.
 *
 * @param payload Pointer to the data to be echoed.
 * @param payload_length Length of the data payload.
 * @return The number of bytes sent.
 *
 * @note This function constructs a packet with a header indicating an echo
 *       operation, payload data, and a footer. It computes CRC for the packet
 *       and sends it over the connection. For demonstration purposes, the
 *       packet is printed to standard output.
 */
int protocol_send_echo(const uint8_t *payload, size_t payload_length) {
    // Calculate the total packet length including payload, header, and footer
    size_t packet_length = payload_length + 7;
    // Create a buffer for the packet
    uint8_t packet[packet_length];
    // Header contains: start byte, packet length (high byte), packet length
    // (low byte), protocol version, and command
    uint8_t header[5] = {0xAA, packet_length >> 8, packet_length, 2, 'e'};
    // Footer contains: CRC (to be filled later)
    uint8_t footer[2] = {0, 0xBB};
    // Copy header into the packet buffer
    memcpy(packet, header, 5);
    // Copy payload into the packet buffer
    memcpy(packet + 5, payload, payload_length);
    // Copy footer into the packet buffer
    memcpy(packet + 5 + payload_length, footer, 2);

    // Compute CRC for the packet
    uint8_t crc = compute_crc(packet, packet_length);
    // Insert computed CRC into the packet
    packet[5 + payload_length] = crc;

    // Send packet by iterating through each byte in the packet
    for (size_t i = 0; i < packet_length; i++) {
        putchar(packet[i]);
    }
    // Return the total packet length
    return packet_length;
}

/**
 * @brief Receives data from an established connection.
 *
 * @return The number of bytes received.
 *
 * @note This function receives a packet from the established connection,
 *       verifies its integrity, processes the packet based on its type, and
 *       takes appropriate actions. It handles acknowledgments, data packets,
 *       open/close signals, echo requests, and test commands. For each received
 *       packet, it sends acknowledgments if necessary and prints relevant
 * messages.
 */
int protocol_receive() {
    // Wait for the start marker
    uint8_t start_marker;
    do {
        start_marker = getchar();
    } while (start_marker != 0xAA);

    // Read packet length
    uint16_t packet_length = getchar() << 8;
    packet_length += getchar();

    // Create a buffer for the packet
    uint8_t packet[packet_length];
    packet[0] = start_marker;
    packet[1] = packet_length >> 8; // Fixed typo: changed << to >>
    packet[2] = packet_length;
    packet[3] = getchar();

    // Check protocol version
    if (packet[3] != 2) {
        protocol_send_ack(VERSION);
        printf("wrong version\n");
    }

    packet[4] = getchar();

    // Read payload
    for (int i = 0; i < packet_length - 7; i++) {
        packet[5 + i] = getchar();
    }

    // Read received CRC
    uint8_t received_crc = getchar();

    // Prepare packet for CRC computation
    packet[packet_length - 2] = 0;
    packet[packet_length - 1] = 0xBB;

    // Compute CRC
    uint8_t computed_crc = compute_crc(packet, packet_length);

    // Compare received CRC with computed CRC
    if (received_crc != computed_crc) {
        protocol_send_ack(CRC);
        printf("incorrect crc: got %d , expected %d\n", received_crc,
               computed_crc);
    }

    // Check end marker
    packet[packet_length - 1] = getchar();
    if (packet[packet_length - 1] != 0xBB) {
        protocol_send_ack(ENDING);
        printf("not the last bit %d\n", packet[packet_length - 1]);
    }

    // Process packet based on command type
    switch (packet[4]) {
    case 'a':
        if (packet[6] == 0) {
            printf("success\n");
        } else if (packet[6] == 1) {
            printf("fail\n");
        }
        break;
    case 'd':
        for (int i = 0; i < packet_length - 7; i++) {
            printf("%c", packet[i + 5]);
        }
        printf("\n");
        break;
    case 'o':
        if (connected == 1) {
            protocol_send_ack(OPENED);
        } else {
            protocol_connect();
        }
        break;
    case 'c':
        if (connected == 1) {
            protocol_disconnect();
        } else {
            protocol_send_ack(CLOSED);
        }
        break;
    case 'e': {
        // Extract payload
        uint8_t payload[packet_length - 7];
        for (int i = 0; i < packet_length - 7; i++) {
            payload[i] = packet[i + 5];
        }
        protocol_send(payload, packet_length - 7);
        break;
    }
    case 't':
        run_tests();
        break;
    default:
        protocol_send_ack(TYPE);
        printf("wrong\n");
    }
    return packet_length;
}

/**
 * @brief Closes the connection.
 *
 * @return None.
 *
 * @note This function updates the connected flag, sends a close signal,
 *       turns off the LED, and performs any other necessary cleanup.
 */
void protocol_disconnect() {
    // Update connection status
    connected = 0;
    // Send close command
    protocol_send_close();
    // Turn off LED
    gpio_put(LED_PIN, 0);
}
