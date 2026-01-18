import math
import struct

sample_rate = 11025
frequency = 440  # Concert A
duration = 1.0   # 1 second
amplitude = 32760

# Generate raw i16 samples
samples = []
for i in range(int(sample_rate * duration)):
    # Sine formula: A * sin(2 * pi * f * t)
    t = i / sample_rate
    sample = int(amplitude * math.sin(2 * math.pi * frequency * t))
    samples.append(sample)

# Save as raw file to inspect/play
with open("sine_11k.raw", "wb") as f:
    for s in samples:
        f.write(struct.pack('<h', s)) # Little Endian i16