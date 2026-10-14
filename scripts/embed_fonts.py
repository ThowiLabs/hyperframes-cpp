#!/usr/bin/env python3
"""Genera un encabezado C++ con fuentes locales, solo en el entorno de compilacion.

Se evita distribuir por separado los archivos binarios de fuentes.
DejaVu Sans: licencia incluida en assets/fonts/LICENSE.txt.
"""
from pathlib import Path
import sys

root = Path(__file__).resolve().parent.parent
font_dir = Path(sys.argv[1]) if len(sys.argv) > 1 else Path("/usr/share/fonts/truetype/dejavu")
fonts = [("regular", font_dir / "DejaVuSans.ttf"),
         ("bold", font_dir / "DejaVuSans-Bold.ttf")]
out = root / "font_embedded.hpp"
for _, p in fonts:
    if not p.is_file():
        raise SystemExit(f"No se encuentra la fuente del sistema: {p}")
with out.open("w", encoding="utf-8") as f:
    f.write("#pragma once\n// Fuentes DejaVu originales embebidas. "
            "Consultar assets/fonts/LICENSE.txt.\nnamespace bundled {\n")
    for name, p in fonts:
        source = p.read_bytes()
        if len(source) > 2_000_000 or source[:4] not in (b"\x00\x01\x00\x00", b"OTTO"):
            raise SystemExit(f"Fuente TTF/OTF inesperada: {p}")
        f.write(f"static const unsigned char {name}[] = {{\n")
        for index in range(0, len(source), 24):
            f.write(",".join(f"0x{byte:02x}" for byte in source[index:index + 24]) + ",\n")
        f.write("};\n")
    f.write("}\n")
print(f"Encabezado generado: {out}")
