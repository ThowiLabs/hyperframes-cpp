#!/usr/bin/env python3
"""Generate synthetic scene metadata for C++ integration tests.

This is a temporary fixture, not a video project or distributable media.
"""
import sys
from pathlib import Path

if len(sys.argv) != 2:
    raise SystemExit("Usage: generate_scene_fixture.py OUTPUT.html")

page = [
    "<!doctype html>",
    '<html lang="es"><head><meta charset="utf-8">',
    '<style>.upper{top:210px;left:65px}.kicker{height:52px;font-size:17px}',
    '.headline{font-size:68px;line-height:1.08;margin-top:50px}',
    '.sub{margin-top:42px;font-size:24px}</style></head><body>',
]
for index in range(6):
    page.extend([
        f'<section class="scene scene-{index+1}" data-start="{index*2}" '
        f'data-duration="2" style="--a:#47DEF5">',
        '<div class="kicker">MOTOR DE VIDEO</div>',
        f'<div class="headline"><div class="wordline">VIDEO NATIVO {index+1}</div></div>',
        '<div class="sub">Prueba técnica reproducible</div>',
        '<div class="barlabel">SALIDA NATIVA</div>',
        '</section>',
    ])
page.append("</body></html>")
Path(sys.argv[1]).write_text("\n".join(page) + "\n", encoding="utf-8")
