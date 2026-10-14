#!/usr/bin/env python3
"""Produce six Spanish narrator clips and verified duration metadata."""
import json
import os
import subprocess
import shutil
import math
from pathlib import Path

DIR = Path(__file__).resolve().parent
AUDIO = DIR / "assets" / "audio"
AUDIO.mkdir(parents=True, exist_ok=True)

TEXTS = [
    "¿C++ o Python? Los dos permiten programar, pero funcionan de forma diferente. Veamos por qué C++ suele ser más rápido en algunos cálculos.",
    "C++ es un lenguaje que ofrece control sobre la memoria y los recursos del equipo. Por eso se utiliza en motores gráficos, videojuegos y simuladores.",
    "Antes de ejecutarse, el código C++ normalmente se compila. Un compilador lo transforma en instrucciones nativas que el procesador puede ejecutar directamente.",
    "Python, en su implementación más común, utiliza un intérprete y objetos dinámicos. Eso hace que programar sea cómodo, pero puede añadir trabajo extra en operaciones repetitivas.",
    "Sin embargo, Python no siempre pierde. Bibliotecas como NumPy utilizan código compilado y pueden ser muy rápidas. Importan el algoritmo, los datos y la forma de medir.",
    "En resumen: C++ destaca cuando buscas rendimiento y control. Python permite desarrollar con agilidad. Muchas aplicaciones combinan ambos. Elige según tu proyecto, no por un mito.",
]
EDGE = os.environ.get("EDGE_TTS_BIN") or shutil.which("edge-tts") or "/workspace/video-venv/bin/edge-tts"
if not Path(EDGE).exists() and not shutil.which(EDGE):
    raise RuntimeError("Missing edge-tts. Install it for narration, or supply EDGE_TTS_BIN.")
meta = []
start = 0.0
for index, words in enumerate(TEXTS, start=1):
    mp3 = AUDIO / f"segment-{index:02}.mp3"
    wav = AUDIO / f"segment-{index:02}.wav"
    subprocess.run([
        EDGE, "--voice", "es-MX-JorgeNeural", "--rate", "+6%",
        "--text", words, "--write-media", str(mp3)
    ], check=True)
    subprocess.run([
        "ffmpeg", "-hide_banner", "-loglevel", "error", "-y", "-i", str(mp3),
        "-af", "highpass=f=90,lowpass=f=9600", "-ac", "1", "-ar", "24000",
        "-c:a", "pcm_s16le", str(wav)
    ], check=True)
    info = json.loads(subprocess.check_output([
        "ffprobe", "-v", "error", "-show_entries", "format=duration",
        "-of", "json", str(wav)
    ]))
    duration = float(info["format"]["duration"])
    # 0.8s at the tail ensures each phrase finishes before the cut.
    scene_dur = round(duration + 0.85, 2)
    item = {"scene": index, "start": round(start, 2), "duration": scene_dur,
            "voice_seconds": round(duration, 2), "text": words}
    meta.append(item)
    start += scene_dur
    print(index, "voice", item["voice_seconds"], "scene", scene_dur, "total", round(start,2), flush=True)

# Round final duration up to a whole second for exact AAC/video sync.
total = math.ceil(start)
meta[-1]["duration"] = round(meta[-1]["duration"] + total - round(start, 2), 2)
(DIR / "timeline.json").write_text(
    json.dumps({"total": total, "scenes": meta}, indent=2,
               ensure_ascii=False) + "\n", encoding="utf-8"
)
