from os import write
from time import sleep
import serial
import struct


class CustomProtocol:
    def __init__(self, address: str = "/dev/ttyACM0", port: int = 115200):
        """Initializes the communication module and establishes a connection."""
        self.__address = address
        self.__port = port

    def connect(self):
        """Opens a connection for communication."""
        self.__ser = serial.Serial(self.__address, self.__port)

    def compute_checksum(self, payload: bytes):
        payload_size = len(payload)
        packet_type = 7
        size = len(payload) + 6

        packet = struct.pack(">HH", packet_type, size) + payload
        if len(packet) % 2:
            packet = packet + b"\x00"

        total_sum = 0
        for i in range(0, len(packet), 2):
            t = int.from_bytes(packet[i : i + 2], byteorder="big")
            total_sum += int.from_bytes(packet[i : i + 2], byteorder="big")

        checksum = ~total_sum & 0xFFFF

        return checksum

    def send(self, payload: bytes):
        """Sends data over the established connection."""
        packet_type = 7
        checksum = self.compute_checksum(payload)
        size = len(payload) + 6

        packet = struct.pack(">HHH", packet_type, checksum, size) + payload

        self.__ser.write(packet)

    def receive(self):
        """Receives data from the established connection."""
        header = self.__ser.read(6)
        (packet_type, checksum, size) = struct.unpack(">HHH", header)
        if packet_type != 7:
            return
        payload = self.__ser.read(size - 6)
        if checksum != self.compute_checksum(payload):
            return
        return payload

    def disconnect(self):
        """Closes the connection."""
        self.__ser.close()

    def cleanup(self):
        """Cleans up resources used by the communication module."""
        del self.__ser
        del self.__address
        del self.__port


p = CustomProtocol()
p.connect()

p.send(b"qwertyuiop[]asdfghjkl;'zxcvbnm,./1234567890-=")
print(p.receive())
print(p.receive())
