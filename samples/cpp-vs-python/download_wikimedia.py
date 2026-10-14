#!/usr/bin/env python3
"""Download openly licensed Wikimedia Commons images with source attribution.

Uses only MediaWiki API / Python standard library.
"""
from __future__ import annotations
import html
import json
import re
from pathlib import Path
from urllib.parse import urlencode
from urllib.request import Request, urlopen

ROOT = Path(__file__).resolve().parent
OUT = ROOT / "assets" / "img"
OUT.mkdir(parents=True, exist_ok=True)

FILES = {
    "silicon.jpg": "File:Silicon die.jpg",
    "server.jpg": "File:Rear of rack at NERSC data center - closeup.jpg",
    "technician.jpg": "File:Technician with laptop working on server rack at NERSC.jpg",
    "processors.jpg": "File:Intel Microprocessors over time.jpg",
}
API = "https://commons.wikimedia.org/w/api.php"
AGENT = "HyperFramesCppEducationalVideo/1.0 (Wikimedia Commons attribution metadata fetch)"

def get(url: str) -> bytes:
    req = Request(url, headers={"User-Agent": AGENT})
    with urlopen(req, timeout=35) as response:
        return response.read()

def plain(text: str) -> str:
    return re.sub(r"<[^>]+>", "", html.unescape(text)).strip()

credits = [
    "# Créditos y licencias de imágenes",
    "",
    "Imágenes descargadas de Wikimedia Commons para un video explicativo.",
    "Las fotografías se utilizan con recortes, subtítulos y superposiciones.",
    "En cada caso se conserva enlace, autor atribuido y licencia original.",
    "",
]
records = []
for name, title in FILES.items():
    params = {
        "action": "query", "format": "json", "titles": title,
        "prop": "imageinfo", "iiprop": "url|size|extmetadata",
        "iiurlwidth": 1280,
    }
    data = json.loads(get(API + "?" + urlencode(params)))
    page = next(iter(data["query"]["pages"].values()))
    if "imageinfo" not in page:
        raise RuntimeError("Wikimedia Commons file unavailable: " + title)
    info = page["imageinfo"][0]
    meta = info["extmetadata"]
    license_name = plain(meta.get("LicenseShortName", {}).get("value", "unknown"))
    author = plain(meta.get("Artist", {}).get("value", "unknown"))
    license_url = meta.get("LicenseUrl", {}).get("value", "")
    if not any(lic in license_name for lic in ("CC0", "CC BY", "Public domain")):
        raise RuntimeError("Unexpected license for " + title + ": " + license_name)
    file_url = info.get("thumburl") or info["url"]
    photo = get(file_url)
    if len(photo) < 10000 or len(photo) > 15_000_000:
        raise RuntimeError("Unexpected image size: " + title)
    (OUT / name).write_bytes(photo)
    page_url = info["descriptionurl"]
    records.append({
        "asset": name, "file": title, "author": author, "license": license_name,
        "license_url": license_url, "commons_page": page_url,
        "downloaded_url": file_url, "bytes": len(photo),
    })
    credits.extend([
        "## " + title.removeprefix("File:"),
        "",
        "- **Autor:** " + author,
        "- **Licencia:** " + license_name + ((" — " + license_url) if license_url else ""),
        "- **Fuente:** " + page_url,
        "- **Archivo local:** assets/img/" + name,
        "",
    ])
    print(name, "=>", len(photo), "bytes", license_name, author[:100])

(ROOT / "CREDITS.md").write_text("\n".join(credits), encoding="utf-8")
(ROOT / "assets" / "wikimedia_sources.json").write_text(
    json.dumps(records, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
)
