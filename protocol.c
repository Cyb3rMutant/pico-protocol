#include "protocol.h"
#include "pico/stdlib.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

const uint LED_PIN = 25;
int connected;

uint8_t compute_crc(uint8_t *data, size_t len) {
    uint8_t crc = 0;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}
// Initializes the communication module.
void protocol_init(void) {
    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    connected = 0;
}
// Opens a connection for communication.
// Returns a handle to the connection.
int protocol_connect() {
    connected = 1;
    protocol_send_open();
    gpio_put(LED_PIN, 1);
    return 0;
}
// Sends data over an established connection.
// Returns the number of bytes sent.
int protocol_send(const uint8_t *payload, size_t payload_length) {
    size_t packet_length = payload_length + 6;
    uint8_t packet[packet_length];
    uint8_t header[4] = {0xAA, packet_length, 2, 'd'};
    uint8_t footer[2] = {0, 0xBB};
    memcpy(packet, header, 4);
    memcpy(packet + 4, payload, payload_length);
    memcpy(packet + 4 + payload_length, footer, 2);

    // Compute CRC
    uint8_t crc = compute_crc(packet, packet_length);
    packet[4 + payload_length] = crc;

    // Print packet (just for demonstration)
    for (size_t i = 0; i < packet_length; i++) {
        putchar(packet[i]);
    }
    return packet_length;
}
int protocol_send_ack(int err) {
    size_t packet_length = 1 + 6;
    uint8_t packet[packet_length];
    uint8_t header[4] = {0xAA, packet_length, 2, 'a'};
    uint8_t footer[2] = {0, 0xBB};
    memcpy(packet, header, 4);
    packet[4] = err;
    memcpy(packet + 4 + 1, footer, 2);

    // Compute CRC
    uint8_t crc = compute_crc(packet, packet_length);
    packet[4 + 1] = crc;

    // Print packet (just for demonstration)
    for (size_t i = 0; i < packet_length; i++) {
        putchar(packet[i]);
    }
    return packet_length;
}
int protocol_send_open() {
    size_t packet_length = 6;
    uint8_t packet[packet_length];
    uint8_t header[4] = {0xAA, packet_length, 2, 'o'};
    uint8_t footer[2] = {0, 0xBB};
    memcpy(packet, header, 4);
    memcpy(packet + 4, footer, 2);

    // Compute CRC
    uint8_t crc = compute_crc(packet, packet_length);
    packet[4] = crc;

    // Print packet (just for demonstration)
    for (size_t i = 0; i < packet_length; i++) {
        putchar(packet[i]);
    }
    return packet_length;
}
int protocol_send_close() {
    size_t packet_length = 6;
    uint8_t packet[packet_length];
    uint8_t header[4] = {0xAA, packet_length, 2, 'c'};
    uint8_t footer[2] = {0, 0xBB};
    memcpy(packet, header, 4);
    memcpy(packet + 4, footer, 2);

    // Compute CRC
    uint8_t crc = compute_crc(packet, packet_length);
    packet[4] = crc;

    // Print packet (just for demonstration)
    for (size_t i = 0; i < packet_length; i++) {
        putchar(packet[i]);
    }
    return packet_length;
}
int protocol_send_echo(const uint8_t *payload, size_t payload_length) {
    size_t packet_length = payload_length + 6;
    uint8_t packet[packet_length];
    uint8_t header[4] = {0xAA, packet_length, 2, 'e'};
    uint8_t footer[2] = {0, 0xBB};
    memcpy(packet, header, 4);
    memcpy(packet + 4, payload, payload_length);
    memcpy(packet + 4 + payload_length, footer, 2);

    // Compute CRC
    uint8_t crc = compute_crc(packet, packet_length);
    packet[4 + payload_length] = crc;

    // Print packet (just for demonstration)
    for (size_t i = 0; i < packet_length; i++) {
        putchar(packet[i]);
    }
    return packet_length;
}

// Receives data from an established connection.
// Returns the number of bytes received.
int protocol_receive() {
    printf("waiting for data\n");
    uint8_t start_marker;
    do {
        start_marker = getchar();
    } while (start_marker != 0xAA);
    uint8_t packet_length = getchar();
    uint8_t packet[packet_length];
    packet[0] = start_marker;
    packet[1] = packet_length;
    packet[2] = getchar();
    if (packet[2] != 2) {
        protocol_send_ack(VERSION);
        printf("wrong version\n");
    }
    packet[3] = getchar();
    for (int i = 0; i < packet_length - 6; i++) {
        packet[4 + i] = getchar();
    }
    uint8_t received_crc = getchar();
    packet[packet_length - 2] = 0;
    packet[packet_length - 1] = 0xBB;
    uint8_t computed_crc = compute_crc(packet, packet_length);
    if (received_crc != computed_crc) {
        protocol_send_ack(CRC);
        printf("incorrect crc: got %d , expected %d\n", received_crc,
               computed_crc);
    }
    packet[packet_length - 1] = getchar();
    if (packet[packet_length - 1] != 0xBB) {
        protocol_send_ack(ENDING);
        printf("not the last bit %d\n", packet[packet_length - 1]);
    }

    switch (packet[3]) {
    case 'a':
        if (packet[4] == 0) {
            printf("success\n");
        } else if (packet[4] == 1) {
            printf("fail\n");
        }
        break;
    case 'd':
        for (int i = 0; i < packet_length - 6; i++) {
            printf("%c", packet[i + 4]);
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
            protocol_disconnect(1);
        } else {
            protocol_send_ack(CLOSED);
        }
        break;
    case 'e': {
        uint8_t payload[packet_length - 6];
        for (int i = 0; i < packet_length - 6; i++) {
            payload[i] = packet[i + 4];
        }
        protocol_send(payload, packet_length - 6);
        break;
    }
    case 't':
        run_tests();
    default:
        protocol_send_ack(TYPE);
        printf("wrong\n");
    }
    return packet_length;
}
// Closes the connection.
void protocol_disconnect() {
    connected = 0;
    protocol_send_close();
    gpio_put(LED_PIN, 0);
}
