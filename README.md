# Intro

Protocols are the foundation of communication between electronic devices, ensuring that data transmission runs smoothly and reliably. Without a standardized protocol, differences in data formatting and transfer methods can cause communication errors, preventing effective device interaction. Protocols are essentially a set of rules, formats, and conventions for sending data across networks or between system components. They provide a universal language that enables disparate systems to communicate, whether transferring files, sending messages, or controlling hardware. The goal of this project is to create a protocol that allows a Raspberry Pi Pico and a PC to communicate via the USB serial port, with the goal of establishing a reliable and efficient channel for data transmission between these two devices.

# How to use

First, clone the repository locally with the following command:

```bash
$ git clone https://gitlab.uwe.ac.uk/y2-abuhummos/cap-protocol
$ cd cap-protocol
```

next a build, run and view output scripts are provided, to build:

```bash
$ ./build.sh
```

to run, first make sure to unplug the pi, and plug it again with the reset button pressed then:

```bash
$ ./run.sh
```

otherwise you can drag and drop the file into the pi.

and finally to view output:

```bash
$ ./out.sh
```

or you can run the python file

```bash
$ sudo python protocol.py
```

note that you cant run the python file and call receive, and run the `out.sh` script at the same time

# Architecture

## USB Serial

USB serial communication has advantages over other options such as GPIO pulsing or UART. It is easier to use thanks to standardized APIs and libraries, which reduce development time and allow for the reuse of existing drivers. It offers faster data transfer rates, allowing for real-time data exchange and large transfers. It is compatible with a variety of operating systems, including Windows, macOS, and Linux, ensuring flawless compatibility with no additional drivers. It requires minimal hardware, relying primarily on the microcontroller's USB interface or a USB-to-serial converter chip, as opposed to GPIO pulsing, which may necessitate additional hardware, increasing system complexity and cost.

## Protocol Structure

| Start byte | Length  | Version | Data Type | Data           | CRC    | End byte |
| ---------- | ------- | ------- | --------- | -------------- | ------ | -------- |
| 1 Byte     | 2 Bytes | 1 Byte  | 1 Byte    | variable Bytes | 1 Byte | 1 Byte   |

### Header

1. **Start byte (1 Byte)**: This byte serves as a synchronization marker, indicating the start of a new packet. It's needed by the receiving end to detect the beginning of a packet reliably.

2. **Length (2 Bytes)**: This field specifies the length of the entire packet, including all subsequent fields. Using 2 bytes allows for a maximum packet length of 65535 bytes, which should be sufficient for most applications.

3. **Version (1 Byte)**: This field indicates the version of the protocol being used. Having versioning in the protocol allows for backward compatibility and future expansion if needed.

4. **Data Type (1 Byte)**: This byte specifies the type of data being transmitted in the packet. By designating specific values or bit patterns to represent various payload types like commands, acknowledgements, or structured data (structs), the protocol gains the ability to handle diverse communication needs. It also helps both ends of the communication channel to interpret the payload correctly.

### Payload

5. **Data (Variable Bytes)**: This is where the actual payload resides. This field can accept any type of data, from acknowledgements to strings to structured data. By making the length of this field variable, the protocol accommodates different types and lengths of data efficiently. Its size can vary from 0 bytes up to `0xFFFF - 7` which is 65528 bytes

### Footer data

6. **CRC (1 Byte)**: The CRC (Cyclic Redundancy Check) byte ensures data integrity by providing a checksum of the packet contents. It allows the receiving end to verify if the packet was received without errors.

7. **End byte (1 Byte)**: Similar to the start byte, this byte marks the end of the packet. Its presence helps in properly parsing and identifying the end of each packet.

# Implementation

## Initialization

The protocol operates on the Pico side using standard I/O functions `putchar` for `sending` and getchar for receiving. Therefore, the initialization process involves initializing the `stdio` library. Additionally, the LED on the Pico is initialized to indicate to the user whether the Pico is connected and ready to receive.

An important step in the initialization function is disabling the translation of newline feeds (`\n`) into carriage return-line feeds (`\r\n`). This is necessary due to a programming nuance with the Pico. Failure to disable this translation can cause issues in the protocol, particularly when a packet has a length of 10, resulting in an overflow scenario.

For instance, data sent from the Pico might appear as follows:

```python
b'\xaa\x00\n\x02d'
```

However, when received on the PC, it will unexpectedly translate to:

```python
b'\xaa\x00\r\n\x02d'
```

## Error checking

### start and end bytes

Packets are encapsulated with start and end bytes, represented by 0xAA and 0xBB respectively. These bytes serve as markers for the receiver, indicating where to commence reading. In cases of corrupted data transmissions or overflows, the start and end bytes provide a reference point for proper packet extraction.

For example, if the following data was sent:

```python
b'hello\xaa\x00\r\n\x02d'
```

The receiver disregards the initial characters until encountering the start byte (0xAA), after which it proceeds to process the rest of the packet.

## Cyclic Redundancy Check (CRC)

Another error-handling mechanism that is used is CRC It entails creating a fixed-size, sequence of bits based on the data under consideration, which is then appended to the data. Upon receipt, the receiver recalculates the CRC and compares it to the one received. If they match, the data was most likely transmitted correctly. CRC is preferred over checksums because of its superior error detection capabilities, particularly in detecting burst errors that occur frequently during data transmission. CRC detects a broader range of errors with a higher probability, making it more robust for ensuring data integrity in communication protocols.

for a simplified example if we have a message represented by a sequence of bits: `100100` and our common divisor used by all sides is `1101`. We want to calculate a 3-bit CRC for this message.

first we append zeros to the message equal to the number of CRC bits): `100100000`, then perform polynomial division and the remainder after division is the CRC.

Here's the division:

```
       111101
     -----------
1101 | 100100000
       1101     ^
       ---------
        1000
        1101    ^
        --------
         1010
         1101   ^
         -------
          1110
          1101  ^
          ------
           0110
           0000 ^
           -----
            1100
            1101^
            ----
             001
```

So, the remainder is `001`, which is our 3-bit CRC.

## sending

The sending operation involves constructing the header data, copying the string to be sent into the packet, and calculating the CRC with an empty field, as described in the CRC section. Sending occurs through a loop, with each byte in the array sent using `putchar`.

## receiving

Unlike sending, receiving is a multi-step process. It begins by waiting for a start byte, followed by reading the two length bytes. Bit shifting is then performed to align the bytes and determine the actual length. For example if we have this received:

```
0x03
0xE8
```

they get converted to `uint16_t` and the first bit shifted to the left to make space then added, like this

```
0x0300
0x00E8+
------
0x03E8
```

Subsequently, a packet array is created to hold the header and payload. The version and type are read, and the size of the payload is calculated. A loop is then executed to read all payload data. Next, the CRC is read into a temporary variable, and both the packet array and space for the CRC are left empty to calculate and compare the received CRC. Finally, the end byte is checked to complete the packet reception process, and the payload is processed based on the type byte.

## payload types

The protocol currently supports various payload types, each with a unique description and payload format:

| type  | description        | payload                         |
| ----- | ------------------ | ------------------------------- |
| `'a'` | acknowledgement    | integer representing error code |
| `'d'` | data               | string representing normal data |
| `'o'` | open a connection  | -                               |
| `'o'` | close a connection | -                               |
| `'e'` | echo the payload   | string to be echoed back        |
| `'t'` | run the unit tests | -                               |

This design facilitates extensibility, allowing developers to define additional types as needed. For instance, commands beyond those listed can be easily incorporated, as demonstrated by the echo type. Furthermore, in theory, the protocol should support sending entire structs through the payload field, although this capability has not been tested due to limitations in Python's struct creation compared to C.

# Testing

unit tests have been provided on the pico side, with the tests running on its hardware, then the result is sent back to the connected pc, it was hard to get any data processing done due to the I/O being used for transimiting data

to run the tests:

```python
p = CustomProtocol()
p.connect()
p.test()
```

# CLI

Again, it was not possible to develop a cli on the pico side due to I/O being occupied, though one could be potentially developed on the python side. the farthest interactive thing done, is the echo method
