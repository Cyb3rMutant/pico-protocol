#include "tests.h"
#include "protocol.h"
#include <stdio.h>

int wrong() {
    size_t packet_length = 7;
    uint8_t packet[packet_length];
    uint8_t header[5] = {0xAA, packet_length >> 8, packet_length, 2, 'o'};
    uint8_t footer[2] = {0, 0xBB};
    memcpy(packet, header, 5);
    memcpy(packet + 5, footer, 2);

    // Print packet (just for demonstration)
    for (size_t i = 0; i < packet_length; i++) {
        putchar(packet[i]);
    }
    return packet_length;
}

void run_tests() {
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
    test8();
    test9();
    test10();
    test11();
    test12();
    test13();
    test14();
    test15();
    test16();
    test17();
    test18();
    test19();
    test20();
}

void test1() {
    // Test 1: Test the compute_crc function with bytes.
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    uint8_t crc = compute_crc(data, 5);
    char res[] = "1 ";
    if (crc == 188) {
        res[1] = 't';
    } else {
        res[1] = 'f';
    }
    protocol_send(res, 2);
}

void test2() {
    // Test 2: Test the compute_crc function with an empty array.
    uint8_t data[] = {};
    uint8_t crc = compute_crc(data, 0);
    char res[] = "2 ";
    if (crc == 0x00) {
        res[1] = 't';
    } else {
        res[1] = 'f';
    }
    protocol_send(res, 2);
}

void test3() {
    // Test 3: Test the compute_crc function with a single byte.
    uint8_t data[] = {0x01};
    uint8_t crc = compute_crc(data, 1);
    char res[] = "3 ";
    if (crc == 0x07) {
        res[1] = 't';
    } else {
        res[1] = 'f';
    }
    protocol_send(res, 2);
}

void test4() {
    // Test 4: Test the compute_crc function with a large array.
    uint8_t data[256];
    for (size_t i = 0; i < 256; i++) {
        data[i] = i;
    }
    uint8_t crc = compute_crc(data, 256);
    char res[] = "4 ";
    if (crc == 20) {
        res[1] = 't';
    } else {
        res[1] = 'f';
    }
    protocol_send(res, 2);
}

void test5() {
    // Test 5: Test the compute_crc function with a large array of zeros.
    uint8_t data[256];
    for (size_t i = 0; i < 256; i++) {
        data[i] = 0;
    }
    uint8_t crc = compute_crc(data, 256);
    char res[] = "5 ";
    if (crc == 0x00) {
        res[1] = 't';
    } else {
        res[1] = 'f';
    }
    protocol_send(res, 2);
}

void test6() {
    // Test 6: Test the compute_crc function with a large array of ones.
    uint8_t data[256];
    for (size_t i = 0; i < 256; i++) {
        data[i] = 0xFF;
    }
    uint8_t crc = compute_crc(data, 256);
    char res[] = "6 ";
    if (crc == 36) {
        res[1] = 't';
    } else {
        res[1] = 'f';
    }
    protocol_send(res, 2);
}

void test7() {
    // Test 7: Test the compute_crc function with a string.
    uint8_t data[] = "hello";
    uint8_t crc = compute_crc(data, 5);
    char res[] = "7 ";
    if (crc == 146) {
        res[1] = 't';
    } else {
        res[1] = 'f';
    }
    protocol_send(res, 2);
}

void test8() {
    // Test 8: Test the compute_crc function with a string of zeros.
    uint8_t data[] = "00000";
    uint8_t crc = compute_crc(data, 5);
    char res[] = "8 ";
    if (crc == 119) {
        res[1] = 't';
    } else {
        res[1] = 'f';
    }
    protocol_send(res, 2);
}

void test9() {
    // Test 9: Test the compute_crc function with a long string.
    uint8_t data[] = "The quick brown fox jumps over the lazy dog.";
    uint8_t crc = compute_crc(data, 44);
    char res[] = "9 ";
    if (crc == 131) {
        res[1] = 't';
    } else {
        res[1] = 'f';
    }
    protocol_send(res, 2);
}

void test10() {
    // Test 10: Test the protocol_send_open function.
    int packet_length = protocol_send_open();
    protocol_receive();
    char res[] = "10 ";
    if (packet_length == 7) {
        res[2] = 't';
    } else {
        res[2] = 'f';
    }
    protocol_send(res, 3);
}

void test11() {
    // Test 11: Test the protocol_send_close function.
    int packet_length = protocol_send_close();
    protocol_receive();
    char res[] = "11 ";
    if (packet_length == 7) {
        res[2] = 't';
    } else {
        res[2] = 'f';
    }
    protocol_send(res, 3);
}

void test12() {
    // Test 12: Test the protocol_send_ack function with NO_ERROR.
    int packet_length = protocol_send_ack(NO_ERROR);
    char res[] = "12 ";
    if (packet_length == 8) {
        res[2] = 't';
    } else {
        res[2] = 'f';
    }
    protocol_send(res, 3);
}

void test13() {
    // Test 13: Test the protocol_send function with a small payload.
    uint8_t payload[] = {0x00, 0x01, 0x02};
    int packet_length = protocol_send(payload, 3);
    char res[] = "13 ";
    if (packet_length == 10) {
        res[2] = 't';
    } else {
        res[2] = 'f';
    }
    protocol_send(res, 3);
}

void test14() {
    // Test 14: Test the protocol_send function with a large payload.
    uint8_t payload[240];
    for (size_t i = 0; i < 240; i++) {
        payload[i] = 96;
    }
    int packet_length = protocol_send(payload, 240);
    char res[] = "14 ";
    if (packet_length == 247) {
        res[2] = 't';
    } else {
        res[2] = 'f';
    }
    protocol_send(res, 3);
}

void test15() {
    // Test 15: Test the protocol_send function with a string.
    uint8_t payload[] = "hello";
    int packet_length = protocol_send(payload, 5);
    char res[] = "15 ";
    if (packet_length == 12) {
        res[2] = 't';
    } else {
        res[2] = 'f';
    }
    protocol_send(res, 3);
}

void test16() {
    // Test 16: Test the protocol_send function with an empty payload.
    uint8_t payload[] = {};
    int packet_length = protocol_send(payload, 0);
    char res[] = "16 ";
    if (packet_length == 7) {
        res[2] = 't';
    } else {
        res[2] = 'f';
    }
    protocol_send(res, 3);
}

void test17() {
    // Test 17: Test the protocol_receive function after sending an open
    // message.
    protocol_send_open();
    int bytes_received = protocol_receive();
    char res[] = "17 ";
    if (bytes_received == 7) {
        res[2] = 't';
    } else {
        res[2] = 'f';
    }
    protocol_send(res, 3);
}

void test18() {
    // Test 18: Test the protocol_receive function after sending a close
    // message.
    protocol_send_close();
    int bytes_received = protocol_receive();
    char res[] = "18 ";
    if (bytes_received == 7) {
        res[2] = 't';
    } else {
        res[2] = 'f';
    }
    protocol_send(res, 3);
}

void test19() {
    // Test 19: Test the protocol_receive function after sending an echo
    // message.
    uint8_t payload[] = {0x00, 0x01, 0x02};
    protocol_send_echo(payload, 3);
    int bytes_received = protocol_receive();
    printf("%d", bytes_received);
    char res[] = "19 ";
    if (bytes_received == 7) {
        res[2] = 't';
    } else {
        res[2] = 'f';
    }
    protocol_send(res, 3);
}
void test20() {
    // Test 20: Test the protocol_receive function after sending a wrong crc
    // message.
    uint8_t payload[] = {0x00, 0x01, 0x02};
    wrong();
    int bytes_received = protocol_receive();
    char res[] = "20 ";
    if (bytes_received == 7) {
        res[2] = 't';
    } else {
        res[2] = 'f';
    }
    protocol_send(res, 3);
}
