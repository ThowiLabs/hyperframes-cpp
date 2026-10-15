# HyperFrames C++ v1.0.1 — archivos de distribución

Esta rama contiene únicamente los ejecutables y la skill empaquetados, no el código fuente ni videos.

- `hyperframes-cpp-v1.0.1-linux-x64.tar.gz`: compilado Linux x64 y skill.
- `hyperframes-cpp-v1.0.1-windows-x64.zip`: compilado Windows x64 y skill (no probado todavía en Windows real).
- `hyperframes-cpp-v1.0.1-agent-skill.zip`: skill portátil para Claude Code y agentes compatibles.
- `SHA256SUMS.txt`: comprobación de integridad SHA-256.

Novedad: descubrimiento de navegador Chromium/Chrome/Edge/Brave instalado por PATH, rutas comunes y procesos activos; rutas Unicode Windows y comando `--browser-process-check`.

El motor ejecuta un navegador headless nuevo y nunca se conecta a pestañas o perfiles de un usuario que ya tenga Chrome abierto. El navegador debe estar instalado para el tema browser. La exportación MP4 no invoca FFmpeg.

La integración actual no permite adjuntar archivos binarios directamente al panel Assets de GitHub Releases; estos archivos se enlazan desde el release 1.0.1 en https://github.com/ThowiLabs/hyperframes-cpp/releases/tag/v1.0.1 .
