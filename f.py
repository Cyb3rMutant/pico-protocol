import serial
import struct
from time import sleep

NO_ERROR = 0
CRC = 1
VERSION = 2
ENDING = 3
TYPE = 4


class CustomProtocol:
    def __init__(self, address: str = "/dev/ttyACM0", port: int = 115200):
        """Initializes the communication module and establishes a connection."""
        self.__address = address
        self.__port = port

    def connect(self):
        """Opens a connection for communication."""
        self.__ser = serial.Serial(self.__address, self.__port)
        self.send_open()

    def compute_crc(self, data: bytes):
        """Computes CRC-8 checksum for the given data."""
        crc = 0
        for byte in data:
            crc ^= byte
            for _ in range(8):
                if crc & 0x80:
                    crc = (crc << 1) ^ 0x07
                else:
                    crc <<= 1
        return crc & 0xFF

    def send(self, payload: bytes):
        """Constructs and sends a packet."""
        packet_length = len(payload) + 6
        header = struct.pack(">BBBB", 0xAA, packet_length, 2, ord("d"))
        packet = header + payload + struct.pack(">BB", 0, 0xBB)
        crc = self.compute_crc(packet)
        packet = header + payload + struct.pack(">BB", crc, 0xBB)
        self.__ser.write(packet)

    def send_ack(self, err):
        """Constructs and sends a packet."""
        packet_length = 1 + 6
        header = struct.pack(">BBBB", 0xAA, packet_length, 2, ord("a"))
        crc = self.compute_crc(header + struct.pack(">BBB", err, 0, 0xBB))
        footer = struct.pack(">BBB", err, crc, 0xBB)
        packet = header + footer
        self.__ser.write(packet)

    def send_open(self):
        """Constructs and sends a packet."""
        packet_length = 6
        header = struct.pack(">BBBB", 0xAA, packet_length, 2, ord("o"))
        crc = self.compute_crc(header + struct.pack(">BB", 0, 0xBB))
        footer = struct.pack(">BB", crc, 0xBB)
        packet = header + footer
        self.__ser.write(packet)

    def send_close(self):
        """Constructs and sends a packet."""
        packet_length = 6
        header = struct.pack(">BBBB", 0xAA, packet_length, 2, ord("c"))
        crc = self.compute_crc(header + struct.pack(">BB", 0, 0xBB))
        footer = struct.pack(">BB", crc, 0xBB)
        packet = header + footer
        self.__ser.write(packet)

    def send_echo(self, payload: bytes):
        """Constructs and sends a packet."""
        packet_length = len(payload) + 6
        header = struct.pack(">BBBB", 0xAA, packet_length, 2, ord("e"))
        packet = header + payload + struct.pack(">BB", 0, 0xBB)
        crc = self.compute_crc(packet)
        packet = header + payload + struct.pack(">BB", crc, 0xBB)
        self.__ser.write(packet)

    def receive(self):
        """Receives and parses a packet."""
        start_marker = self.__ser.read(1)
        if start_marker != b"\xAA":
            # Discard bytes until start marker is found
            while start_marker != b"\xAA":
                start_marker = self.__ser.read(1)

        length_byte = self.__ser.read(1)
        packet_length = struct.unpack(">B", length_byte)[0]

        protocol_version = self.__ser.read(1)
        if protocol_version != b"\x02":
            self.send_ack(VERSION)
            print(f"incorrect protocol version: {protocol_version}")
        message_type = self.__ser.read(1)

        payload_length = packet_length - 6
        payload = self.__ser.read(payload_length)

        received_crc = self.__ser.read(1)
        computed_crc = self.compute_crc(
            start_marker
            + length_byte
            + protocol_version
            + message_type
            + payload
            + struct.pack(">BB", 0, 0xBB)
        )

        if received_crc != struct.pack(">B", computed_crc):
            self.send_ack(CRC)
            print(
                f"incorrect crc: got {received_crc} , expected {struct.pack('>B', computed_crc)}"
            )

        end_marker = self.__ser.read(1)
        if end_marker != b"\xBB":
            self.send_ack(ENDING)
            print(f"not the last bit {end_marker}")
            pass
        match message_type:
            case b"a":
                if payload == b"\x01":
                    return b"success"
                if payload == b"\x00":
                    return b"fail"
            case b"d":
                return payload
            case b"o":
                self.send_open()
                return b"open"
            case b"c":
                self.send_close()
                return b"close"
            case b"e":
                print("echo")
                self.send(payload)
                return b"echo"
            case _:
                self.send_ack(TYPE)
                print("wrong")

        return payload

    def disconnect(self):
        """Closes the connection."""
        self.send_close()
        self.__ser.close()

    def cleanup(self):
        """Cleans up resources used by the communication module."""
        del self.__ser
        del self.__address
        del self.__port

    def test(self):
        packet_length = 6
        header = struct.pack(">BBBB", 0xAA, packet_length, 2, ord("t"))
        crc = self.compute_crc(header + struct.pack(">BB", 0, 0xBB))
        footer = struct.pack(">BB", crc, 0xBB)
        packet = header + footer
        self.__ser.write(packet)
        for _ in range(100):
            print(self.receive())
            # sleep(1)


p = CustomProtocol()
p.connect()
p.receive()
p.receive()
# # print(p.compute_crc(b"The quick brown fox jumps over the lazy dog."))
# # p.send_echo(b"hello world")
# # print(p.receive())
p.test()
# print(p.compute_crc(bytes([0xFF for _ in range(256)])))

# p.send(b"hello world!")
# p.send_ack(NO_ERROR)
# sleep(1)
# p.send_close()
# print(p.receive())
# print(p.receive())
# print(p.receive())
