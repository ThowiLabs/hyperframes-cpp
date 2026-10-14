# Production workflows for HyperFrames C++

## Installation for Claude Code and compatible Agent Skills runtimes

The canonical skill is `.claude/skills/hyperframes-cpp/SKILL.md`. Copy that **entire folder**, including `references/`, to `~/.claude/skills/hyperframes-cpp/` and/or `~/.agents/skills/hyperframes-cpp/`. The release contains a portable skill ZIP and installer. Do not copy only the Markdown file when it references this document.

## Composition workflow

1. Write a short storyboard with 1 clear idea per scene. Collect target duration, format, FPS and sources.
2. For each scene, prepare on-screen text, technically correct commands and **separate spoken-TTS text**. Spell numbers and abbreviations according to their intended pronunciation.
3. Download authorized visual media under `assets/img/` with authorship, source URL and applicable license in `CREDITS.md`.
4. Create or synthesize scene-by-scene audio. Listen to uncertain words, then decode to PCM signed 16-bit, mono, 24000 Hz WAV files under `assets/audio/segment-01.wav` etc.
5. Measure actual duration of each WAV using its sample count. Add a reasonable short tail after speech and calculate cumulative start times; scene metadata lives in HTML `data-start` and `data-duration` (seconds).
6. Build a valid local HTML document with style rules and timed scene sections. For GSAP, set `window.__timelines.main` to a paused timeline and include licensed JS in the input project.
7. Call `--inspect`. Render screenshots from early/mid/late scene timestamps. Confirm text does not clip; no unsupported images or missing scripts.
8. Preview 3-5 seconds, then render full MP4 using browser theme. Confirm native encoding reaches exit code 0.
9. Verify final streams by playback or probing, and deliver MP4 plus optional editable media archive. Do not commit audiovisual content to engine `main`.

## Minimal HTML test example

```html
<!doctype html>
<html lang="es">
<head>
  <meta charset="utf-8">
  <script src="assets/vendor/gsap.min.js"></script>
  <style>
    html,body {width:720px;height:1280px;overflow:hidden;margin:0;background:#091925;color:white;}
    .scene {position:absolute;inset:0;padding:65px;}
    .wordline {font:900 70px Arial;margin-top:260px;}
    .sub {font:24px Arial;margin-top:40px;}
  </style>
</head>
<body>
  <section class="scene" data-start="0" data-duration="5">
    <div class="kicker">SECCIÓN</div><div class="wordline">Hola, Linux</div>
    <div class="sub">Primera escena de cinco segundos.</div>
  </section>
  <section class="scene" data-start="5" data-duration="5">
    <div class="kicker">OTRA SECCIÓN</div><div class="wordline">Continúa</div>
    <div class="sub">Segunda escena de cinco segundos.</div>
  </section>
  <script>
    const timeline=gsap.timeline({paused:true});
    timeline.fromTo(".scene:nth-of-type(1) .wordline",
      {opacity:0,y:55},{opacity:1,y:0,duration:1},0.3);
    timeline.fromTo(".scene:nth-of-type(2) .wordline",
      {opacity:0,y:55},{opacity:1,y:0,duration:1},5.3);
    window.__timelines={main:timeline};
  </script>
</body>
</html>
```

This is a *technical illustration*, not a video project committed to the engine repository. The JS animation asset must be provided separately under its own license.

## TTS pronunciation versus display correctness

```json
{
  "title": "C++ y SSH, puerto 2222",
  "command_on_screen": "ssh -p 2222 usuario@servidor",
  "spoken_narration": "Hoy veremos ce más más y ese ese hache. Usa el puerto dos mil doscientos veintidós para este ejemplo."
}
```

This JSON is optional production metadata used by a **user-created** generation script, NOT a format directly supported by the engine executable.

TTS can mistake:
- `22` vs `2.2`: “veintidós” versus “dos punto dos”.
- `2026`: “dos mil veintiséis”.
- `192.168.1.5`: read each IP octet deliberately.
- `01/06/2026`: say full day, month, year in words to avoid locale ambiguity.
- `C++`: “ce más más”.
- `sshd -t`: “ese ese hache de, guion te”, but show `sshd -t` exactly in on-screen terminal graphics.

Test with the exact TTS provider/voice; do not apply untested phonetic substitutions automatically. Code blocks and names on screen should stay technically accurate.

## Audio conversion during preproduction

HyperFrames v1.0.0 accepts WAV PCM signed 16-bit, mono, 24 kHz. If the TTS generated MP3, any approved decoder may produce WAV. An optional **preparation only** example, when FFmpeg is available:

```bash
ffmpeg -hide_banner -loglevel error -y -i voice.mp3 \
  -ac 1 -ar 24000 -c:a pcm_s16le assets/audio/segment-01.wav
```

FFmpeg is **not required** by the C++ MP4 renderer; preparation and delivery are different workflows. Music and sound effects must currently be mixed with the narration WAV before export. Always respect media rights.

## Browser troubleshooting

- Empty output: inspect HTML structure and `window.__timelines.main`, relative file URLs, GSAP asset and complete timeline.
- Browser not found: try `--browser-check`, `HFC_BROWSER`, common install paths or process discovery. Firefox/Safari are not the current CDP backend.
- Preview and full export differ: ensure the correct FPS and time values, and avoid non-deterministic JS operations.
- Codec option fails: the executable may be an older AVI-only build; download the full native H.264/AAC release.
- Windows: executable is cross-compiled but not yet runtime-tested on a real Windows host.
- Rendering hostile HTML is unsafe: do not run third-party JS as root.
