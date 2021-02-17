#!/usr/bin/env python3
import random
import sys

def write_binary(word, size=1):
    sys.stdout.buffer.write(word.to_bytes(size, byteorder='little'))

data = bytearray()

def gen(depth):
    action = random.randint(0, 5)

    if depth >= 20 or (depth >= 10 and action == 0): # a
        data.append(ord('a'))
    elif action <= 3: # plus
        gen(depth + 1)
        data.append(ord('+'))
        gen(depth + 1)
    else: # parens
        data.append(ord('['))
        gen(depth + 1)
        data.append(ord(']'))

data.append(ord('('))
gen(0)
data.append(ord(')'))

print(f'Input size: {len(data)} elements', file=sys.stderr)

write_binary(ord('b'))
write_binary(2) # Version
write_binary(1) # Dimension
sys.stdout.buffer.write("  u8".encode()) # Type
write_binary(len(data), 8) # Size
sys.stdout.buffer.write(data) # Data
