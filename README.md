# Intro

Protocols serve as the bedrock of communication between electronic devices, ensuring that data transmission occurs smoothly and reliably. Without a standardized protocol, discrepancies in data formatting and transfer methods can lead to communication errors, hindering effective interaction between devices. In essence, protocols define a set of rules, formats, and conventions for transmitting data across networks or between components within a system. They provide a universal language that allows disparate systems to interact seamlessly, whether it involves transferring files, sending messages, or controlling hardware. The focus of this project is to develop a protocol that enables communication between a Raspberry Pi Pico and a PC via the USB serial port, aiming to establish a robust and efficient channel for data transmission between these two devices.

# How to use

First, clone the repository locally with the following command:

```bash
$ git clone https://gitlab.uwe.ac.uk/y2-abuhummos/cap-protocol
$ cd cap_protocol
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

# Architecture

## USB Serial

USB serial communication offers advantages over alternatives like GPIO pulsing or UART. It's easier to use with standardized APIs and libraries, reducing development time and leveraging existing drivers. It provides higher data transfer rates, facilitating real-time data exchange and large transfers. It's compatible across operating systems like Windows, macOS, and Linux, ensuring seamless integration without additional drivers. It demands minimal hardware requirements, primarily relying on the microcontroller's USB interface or a USB-to-serial converter chip, unlike GPIO pulsing, which may require extra hardware, increasing system complexity and cost.

## Protocol Structure

| Start byte | Length  | Version | Data Type | Data           | CRC    | End byte |
| ---------- | ------- | ------- | --------- | -------------- | ------ | -------- |
| 1 Byte     | 2 Bytes | 1 Byte  | 1 Byte    | variable Bytes | 1 Byte | 1 Byte   |

### Header

1. **Start byte (1 Byte)**: This byte serves as a synchronization marker, indicating the start of a new packet. It's crucial for the receiving end to detect the beginning of a packet reliably.

2. **Length (2 Bytes)**: This field specifies the length of the entire packet, including all subsequent fields. Using 2 bytes allows for a maximum packet length of 65535 bytes, which should be sufficient for most applications.

3. **Version (1 Byte)**: This field indicates the version of the protocol being used. Having versioning in the protocol allows for backward compatibility and future expansion if needed.

4. **Data Type (1 Byte)**: This byte specifies the type of data being transmitted in the packet. By designating specific values or bit patterns to represent various payload types like commands, acknowledgements, or structured data (structs), the protocol gains the ability to handle diverse communication needs. It also helps both ends of the communication channel to interpret the payload correctly.

### Payload

5. **Data (Variable Bytes)**: This is where the actual payload resides. This field can accept any type of data, from acknowledgements to strings to structured data. By making the length of this field variable, the protocol accommodates different types and lengths of data efficiently. It can vary from 0 bytes up to `0xFFFF - 7` which is 65528 bytes

### Footer data

6. **CRC (1 Byte)**: The CRC (Cyclic Redundancy Check) byte ensures data integrity by providing a checksum of the packet contents. It allows the receiving end to verify if the packet was received without errors.

7. **End byte (1 Byte)**: Similar to the start byte, this byte marks the end of the packet. Its presence helps in properly parsing and identifying the end of each packet.

# Implementation
