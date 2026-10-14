# Fecha
2026-10-14

# Objetivo
Preparar HyperFrames C++ 1.0.0 para distribución: skill Agent Skills para Claude Code/compatibles, descubrimiento de navegador por rutas/procesos y release GitHub con ejecutables Linux/Windows.

# Decisiones tomadas
- No añadir proyectos de video, imágenes ni audios al repositorio main.
- Implementar skill Markdown frontmatter en .claude/skills/hyperframes-cpp, con manual de producción, TTS pronunciación fonética de palabras/números y ejemplos técnicos.
- Proveer instalador de la misma skill tanto en ~/.claude/skills como ~/.agents/skills.
- Prioridad de navegador: HFC_BROWSER explícito, PATH, rutas de instalación típicas, procesos activos como respaldo. Leer ruta del proceso sin reutilizar pestañas/perfiles.
- Linux: inspección /proc/<pid>/exe. Windows: snapshot Toolhelp32 + QueryFullProcessImageNameW. No se usan herramientas externas para buscar procesos.
- Usar compilación nativa C++17. La GitHub App conectada carece del permiso `workflows`, por lo que GitHub rechazó añadir `.github/workflows/release.yml`; los archivos de distribución se publicarán en rama de assets separada y se vincularán desde GitHub Release hasta que exista carga autenticada de adjuntos. Tag v1.0.0 y release no-prerelease (Latest).
- Paquetes independientes para Linux x64, Windows x64 y skill de agentes con SHA256SUMS.txt, sin subirlos a main.

# Arquitectura actual
Engine CLI C++17, codificación H.264/AAC y AVI integradas, renderizadores classic/news/litehtml/browser. Browser CDP lanza perfil temporal headless y requiere Chromium/Chrome/Edge/Brave disponible. No depende de FFmpeg en runtime de render; entrada PCM mono WAV 24k, no MP3/M4A directo, y no consume JSON arbitrarios.

# Librerías usadas
Librería estándar C++17, POSIX /proc, WinAPI Kernel32 Toolhelp, nlohmann/json, litehtml, minih264, minimp4, vo-aacenc, stb. Sin nuevas dependencias en runtime.

# Archivos importantes modificados
browser_detection.hpp, README.md, docs/BROWSER_MODE.md, docs/RELEASE-v1.0.0.md,
.claude/skills/hyperframes-cpp/SKILL.md,
.claude/skills/hyperframes-cpp/references/WORKFLOWS.md,
scripts/install-skill.sh, scripts/package-release.sh,
Plantilla workflow fuera del repositorio (no publicada por falta de permiso workflows),
tests/browser-discovery-test.cpp, tests/browser-discovery.sh,
contexto/05-version-1-release-y-agent-skill.md,
tareas/completado-11-release-v1-skill.md.

# Problemas encontrados
La detección anterior no leía rutas de navegadores en procesos activos, aunque resolvía PATH/ubicaciones comunes. El proyecto no tenía una skill reutilizable. GitHub App publica commits y releases pero no expone adjuntos binarios, y el push de workflows requiere un permiso que falta. Solución de distribución: branch `release-assets-v1.0.0` con binarios/skill y checksums, y enlaces directos en GitHub Release; conservar instrucción para `gh release upload` a futuro.

# Soluciones implementadas
Se añadió descubrimiento por procesos sin intervenir en sesiones del usuario. Se validó el path override, PATH y un ejecutable de proceso real en /proc, y compiló la rama Windows del detector. Se redactó Agent Skill con ruta de binario de PATH como primera opción, fonética TTS y comandos verificados de vídeo. El script local empaqueta binarios y skill con checksum; GitHub Releases enlaza los paquetes versionados mientras no haya API de carga de assets en el conector.

# Pendientes
Verificación de ejecución del binario en Windows nativo; compatibilidad MacOS (no hay binarios oficiales); MP3/M4A de entrada sin conversión; revisión formal de patentes de codecs, pruebas visuales automatizadas; robustez mayor del protocolo CDP para mensajes fragmentados.

# Próximos pasos
Compilar Linux/Windows, probar smoke + browser, empaquetar, hacer commit/push en main, publicar rama de binarios, crear tag v1.0.0, publicar GitHub Release y verificar enlaces.
