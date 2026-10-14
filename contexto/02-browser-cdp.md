# HyperFrames C++ — browser-backed compositor

Fecha: 2026-10-14
Rama: develop/native-compositor

## Solicitud y decisión
El usuario rechazó la fidelidad del renderizador experimental de litehtml y pidió que el ejecutable detectara el navegador instalado y lo utilizara. Se conservan los temas anteriores como fallback manual, y se añade modo explícito `browser` sin reimplementar todo CSS/GSAP.

## Arquitectura implementada
- `browser_detection.hpp`: detección por PATH en Linux y Windows, rutas de instalación convencionales de Chrome/Edge y variable `HFC_BROWSER`.
- `browser_capture.hpp`: navegador headless Chromium/Chrome/Edge con perfil temporal, puerto CDP en 127.0.0.1, websocket nativo de C++ y búsqueda de `window.__timelines.main`.
- Por cada fotograma, avanza GSAP a `t=n/fps`, sincroniza visibilidad de `section.scene` según sus `data-start`/`data-duration` y captura JPEG vía CDP; decodifica usando stb_image y lo convierte a BGR para el codificador.
- `native_mp4.hpp`: codificación H.264 con minih264 y AAC con VisualOn, mux con minimp4, compilados como código C enlazado al ejecutable C++.
- `CMakeLists.txt`: opciones `HFC_BROWSER_CAPTURE`, `HFC_HTML_LAYOUT`, `HFC_NATIVE_MP4` y enlazado estático para Linux/Windows, sin bibliotecas de GCC externas en Windows.
- `--browser-check`, `--browser-frame HTML t output.png`, argumento theme=browser y clip-seconds opcional para previews.

## Pruebas verificadas
- Chromium 155 detectado y ejecutado en Debian sandbox. Render del HTML **original completo** con GSAP original encontrado en `/workspace/ley-antimemes-mexico/index.html`, no sobre video existente.
- Captura de escena 1 a 4 s y escena 5 a 45 s visualmente verificada. Sincronización de escenas necesaria porque CSS mantiene secciones superpuestas.
- Video 67 s a 720x1280 24 FPS = 1608 frames H.264, 1571 frames AAC mono 24kHz, ambos duration 67.0000 s, verificado con ffprobe y decodificación FFmpeg *como herramienta de QA*, no en runtime.
- Preview 5 s a 720x1280 24 FPS = 120 frames, voz original, verificado por ffprobe y decodificado sin errores.
- 67 s a 1fps también verificado con seis escenas. Importante: es solo diagnóstico, no calidad final.
- Windows x64 compilación cruzada estática superada; objdump importa solamente KERNEL32.dll, WS2_32.dll, msvcrt.dll (DLL del sistema operativo). No probado ejecutado en Windows real.
- Linux `ldd` indica `not a dynamic executable`.
- Browser fixture independiente `tests/fixtures/browser-scene.html` y `tests/browser-integration.sh` pasa con dos timestamps distintos y MP4 H.264/AAC de 2 s.
- Las fuentes GSAP originales se encontraron en proyecto externo en sandbox, no se suben al repo por licencia; el ejemplo del repo carece del asset. En modo browser hay que proporcionar HTML junto con sus assets.

## Pendiente
- Test de runtime Windows real.
- Mejorar rendimiento y estabilidad a 60fps y handling de screenshots WebSocket fragmentados.
- Añadir otros navegadores no-Chromium (Firefox, Safari) mediante APIs compatibles.
- Render de páginas genéricas sin metadata de escenas; actual `hf::load()` sólo acepta formato de escenas específico.
- Seguridad: ejecutar sólo HTML confiable; no sandbox robusto frente a JS hostil y procesos de navegador iniciados como root en sandbox usan --no-sandbox.
- Revisar licencias y patentes H.264/AAC; GSAP licencia independiente con límites para builders visuales competidores.

## Evidencia temporal (HTTP disponible sólo mientras tunnel activo)
- Video completo browser 24fps: Archivo de verificación externo, no versionado
- Preview browser 5s: Archivo de verificación externo, no versionado

## Próximos pasos
1. Validar build limpio en clone y smoke tests.
2. Commit y push en develop/native-compositor, mantener main estable.
3. Escuchar evaluación visual del usuario antes de prometer compatibilidad arbitraria.
