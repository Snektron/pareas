#!/usr/bin/env python3
import sys

def write_binary(word, size=1):
    sys.stdout.buffer.write(word.to_bytes(size, byteorder='little'))

data = sys.stdin.buffer.read()
if data[-1] == '\n':
    data = data[:-1]

write_binary(ord('b'))
write_binary(2) # Version
write_binary(1) # Dimension
sys.stdout.buffer.write("  u8".encode()) # Type
write_binary(len(data), 8) # Size
sys.stdout.buffer.write(data) # Data
