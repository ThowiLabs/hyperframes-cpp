#!/usr/bin/env python3
"""Fill HTML timing markers from voice durations, keeping scene timings contiguous."""
import json
from pathlib import Path

ROOT=Path(__file__).resolve().parent
values=json.loads((ROOT/"timeline.json").read_text(encoding="utf-8"))
scenes=values["scenes"]
assert len(scenes)==6 and all(s["duration"]>3.0 for s in scenes)
assert abs(values["total"] - sum(s["duration"] for s in scenes))<0.05

html=(ROOT/"index.template.html").read_text(encoding="utf-8")
for scene in scenes:
    n=scene["scene"]
    html=html.replace("__START%d__" % n, str(scene["start"]))
    html=html.replace("__DUR%d__" % n,str(scene["duration"]))
html=html.replace("__TOTAL__", str(values["total"]))
html=html.replace("__SCENES_META__", json.dumps(
    [{"start":s["start"],"duration":s["duration"]} for s in scenes],
    separators=(",",":")))
assert "__TOTAL__" not in html and "__START" not in html and "__DUR" not in html and "__SCENES_META__" not in html
(ROOT/"index.html").write_text(html,encoding="utf-8")
print("Saved index.html, duration",values["total"],"seconds")
