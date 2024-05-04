#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "pico/stdlib.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum errors {
    NO_ERROR = 0,
    CRC = 1,
    VERSION = 2,
    ENDING = 3,
    TYPE = 4,
    OPENED = 5,
    CLOSED = 6,
};

uint8_t compute_crc(uint8_t *data, size_t len);
// Initializes the communication module.
void protocol_init(void);
// Opens a connection for communication.
// Returns a handle to the connection.
int protocol_connect();
// Sends data over an established connection.
// Returns the number of bytes sent.
int protocol_send(const uint8_t *payload, size_t payload_length);
// Sends an acknowledgment over an established connection.
// Returns the number of bytes sent.
int protocol_send_ack(int err);
// Sends an open connection message.
int protocol_send_open();
// Sends a close connection message.
int protocol_send_close();
int protocol_send_echo(const uint8_t *payload, size_t payload_length);
// Receives data from an established connection.
// Returns the number of bytes received.
int protocol_receive();
// Closes the connection.
void protocol_disconnect();

#endif
