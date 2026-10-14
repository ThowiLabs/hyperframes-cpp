#!/usr/bin/env python3
"""Generate placeholder narration WAV fixtures for local testing.

These short tone pulses are synthetic test data, not speech or licensed media.
"""
from pathlib import Path
import math
import struct
import sys
import wave

destination = Path(sys.argv[1]) if len(sys.argv) > 1 else Path("assets/audio")
destination.mkdir(parents=True, exist_ok=True)
sample_rate = 24000
for i in range(1, 7):
    path = destination / f"segment-{i:02d}.wav"
    with wave.open(str(path), "wb") as audio:
        audio.setnchannels(1)
        audio.setsampwidth(2)
        audio.setframerate(sample_rate)
        samples = bytearray()
        for n in range(sample_rate // 3):
            t = n / sample_rate
            envelope = min(1.0, t * 20.0) * min(1.0, (1.0 / 3.0 - t) * 20.0)
            level = int(2500 * envelope * math.sin(2 * math.pi * (320 + 20 * i) * t))
            samples.extend(struct.pack("<h", level))
        audio.writeframes(samples)
    print(path)
