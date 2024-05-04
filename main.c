#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "protocol.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main() {
    protocol_init();
    while (1) {
        char h[] = "hello world!";
        // protocol_receive();
        // protocol_send(h, strlen(h));
        // protocol_send_ack(1);
        protocol_receive();
        // sleep_ms(250);
        // sleep_ms(1000);
    }
}
