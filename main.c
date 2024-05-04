#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
const uint LED_PIN = 25;

uint16_t compute_checksum(uint8_t *payload, size_t payload_size) {
    uint16_t packet_type = 7;
    uint16_t size = payload_size + 6;
    size_t packet_size = payload_size % 2 ? payload_size + 5 : payload_size + 4;
    // Create packet buffer
    uint8_t packet[packet_size];
    packet[1] = packet_type & 0xFF;
    packet[0] = (packet_type >> 8) & 0xFF;
    packet[3] = size & 0xFF;
    packet[2] = (size >> 8) & 0xFF;
    packet[packet_size - 1] = 0x00;
    for (size_t i = 0; i < payload_size; i++) {
        packet[i + 4] = payload[i];
    }

    // Copy payload into buffer

    uint32_t total_sum = 0;
    for (size_t i = 0; i < packet_size; i += 2) {
        uint32_t t = (packet[i] << 8) | packet[i + 1];
        total_sum += t;
    }
    // Compute checksum
    uint16_t checksum = ~(total_sum & 0xFFFF);

    return checksum;
}

// Initializes the communication module.
void protocol_init(void) { stdio_init_all(); }
// Opens a connection for communication.
// Returns a handle to the connection.
int protocol_connect(const char *address, int port);
// Sends data over an established connection.
// Returns the number of bytes sent.
int protocol_send(const uint8_t *data, int dataSize) {
    uint16_t packet_type = 7;
    uint16_t checksum = compute_checksum((uint8_t *)data, dataSize);
    uint16_t size = dataSize + 6;

    uint8_t packet[6];
    packet[1] = packet_type & 0xFF;
    packet[0] = (packet_type >> 8) & 0xFF;
    packet[3] = checksum & 0xFF;
    packet[2] = (checksum >> 8) & 0xFF;
    packet[5] = size & 0xFF;
    packet[4] = (size >> 8) & 0xFF;

    // for (size_t i = 0; i < 6; i++) {
    //     printf("%02x", packet[i]);
    // }
    // for (size_t i = 0; i < dataSize; i++) {
    //     printf("%02x", data[i]);
    // }
    for (size_t i = 0; i < 6; i++) {
        putchar(packet[i]);
    }
    for (size_t i = 0; i < dataSize; i++) {
        putchar(data[i]);
    }
}
// Receives data from an established connection.
// Returns the number of bytes received.
void protocol_receive() {
    char header[6];
    for (int i = 0; i < 6; ++i) {
        header[i] = getchar();
    }

    uint16_t packet_type, checksum, size;
    // Manually extracting fields from the header
    packet_type = (header[0] << 8) | header[1];
    checksum = (header[2] << 8) | header[3];
    size = (header[4] << 8) | header[5];

    if (packet_type != 7) {
        protocol_send("incorrect protocol g", 20);
    }

    size -= 6;
    uint8_t payload[size];

    for (int i = 0; i < size; ++i) {
        payload[i] = getchar();
    }
    int c = compute_checksum(payload, size);
    if (checksum != c) {
        protocol_send("incorrect checksum", 18);
    }
    protocol_send(payload, size);
}
// Closes the connection.
void protocol_disconnect(int connectionHandle);
// Cleans up resources used by the communication module.
void protocol_cleanup(void);

int main() {
    protocol_init();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (1) {
        char h[] = "hello world!";
        protocol_receive();
        protocol_send(h, sizeof(h));
        // gpio_put(LED_PIN, 0);
        // sleep_ms(250);
        // gpio_put(LED_PIN, 1);
        // sleep_ms(1000);
    }
}
