# Roadmap and compatibility policy

`main` contains the current experimental engine. Development and demonstrations may use independent branches or repositories. The main repository does not host video projects or generated media.

| Area | Status |
|---|---|
| Native time-based easing | Implemented |
| 9:16 resampling / resolution selection | Implemented |
| Bounded HTML scene parser and CSS px properties | Implemented, limited |
| litehtml raster adapter | Experimental |
| Browser-assisted Chromium/Chrome/Edge capture | Implemented, requires installed browser |
| H.264/AAC MP4 using embedded C codecs | Experimental, Linux validated |
| Static Linux / cross-compiled Windows builds | Implemented, Windows runtime untested |
| MP3/M4A and other audio inputs | Pending |
| Broader CSS, generic DOM and media compositing | Pending |
| Real Windows QA, fuzzing, CI and security hardening | Pending |
| MP4 codec licensing/patent review | Pending |

No installed FFmpeg binary is required for native output. Browser mode **intentionally** launches an installed compatible browser and is not a general secure sandbox for untrusted HTML. Dependencies added to the build must be redistributable and support static Linux/Windows builds.
