from serial import Serial
import time
import itertools
import struct


s = Serial("/dev/ttyUSB0", 460800)

fields = [
    'c',
    'v',
    'pos',
    'rv',
    'ra',
    'n',
    'r',
    'us',
]

measures = list(itertools.chain(*[[f + 'x', f + 'y'] for f in fields]))

while True:
    x = s.read(66)
    if x[0] != 0xab or x[1] != 0xcd:
        continue

    values = struct.unpack("{}f".format(len(fields * 2)), x[2:])
    z = " ".join(["{}={}".format(m[0], m[1]) for m in zip(measures, values)])
    print(z, flush=True)
