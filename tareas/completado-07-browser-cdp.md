# Tarea 07: renderizar HTML/CSS/JS real desde el navegador instalado
Estado: implementado experimentalmente y probado en Linux. Windows x64 compilado cruzado; runtime Windows pendiente.
Implementado: búsqueda Chrome/Chromium/Edge en Linux y Windows, variable HFC_BROWSER; C++ lanza navegador headless aislado en perfil temporal, protocolo CDP/WebSocket localhost, timeline window.__timelines.main.seek(t), cambio de visibilidad entre section.scene, captura JPEG, decode stb_image a BGR e integración con MJPEG o H.264/AAC sin FFmpeg.
Pruebas: capturas reales de escenas 1 y 5. Video browser full 67s 720x1280 24fps con audio original, validado ffprobe/FFmpeg solo QA. Preview 5s y fixture sintética autónoma, tests/browser-integration.sh.
Limitaciones: solo navegadores Chromium, input HTML+JS+GSAP completos, parser temporal para escenas conocidas, requiere navegador en runtime, no seguridad para HTML hostil. Documentación docs/BROWSER_MODE.md y contexto/02-browser-cdp.md.
