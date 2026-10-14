# Fecha
2026-10-14

# Objetivo
Promover motor C++ (HTML/Chromium/CDP, AVI, H.264/AAC/MP4) al branch público main y retirar los proyectos de video publicados como demostraciones.

# Decisiones tomadas
Se parte de develop/native-compositor (commit 2a48eb1), NO de la rama demo/cpp-vs-python-wikimedia. Evitar ese historial de assets en el main actualizado. No usar force push; fast-forward de main sobre develop con un commit adicional de limpieza. Conservar pruebas técnicas reproducibles; no publicar fotografías, audios, vídeos, HTML de trabajos anteriores, GSAP ni medios externos.

# Arquitectura actual
C++17 con cuatro temas: native classic/news, litehtml y browser CDP, con codecs AVI y MP4. Audio de entrada PCM mono WAV 24k. Para browser se exige Chrome/Chromium/Edge instalado; el modo nativo no requiere navegador. El motor no invoca FFmpeg para exportar.

# Librerías usadas
C++17, STL, stb image/jpeg/truetype, nlohmann/json, litehtml/Gumbo, minih264, minimp4, VisualOn AAC como códigos C compilados estáticamente.

# Archivos importantes modificados
README.md, .gitignore, docs/ARCHITECTURE.md, docs/BROWSER_MODE.md, docs/ROADMAP.md, CONTRIBUTING.md, render_news.hpp, render_scene.hpp, tests/smoke-linux.sh, tests/generate_scene_fixture.py, tests/generate_audio_fixture.py, contexto/04-main-motor-limpio.md, tareas/completado-10-main-solo-motor.md.

# Problemas encontrados
Las pruebas antiguas requerían samples/ley-antimemes, y el README usaba ese ejemplo como instrucción de uso. La rama demo/cpp-vs-python contiene un proyecto audiovisual que no debe llegar a main. Los renderers clásicos tenían títulos de un proyecto anterior codificados directamente.

# Soluciones implementadas
Crear HTML técnico sintético con seis escenas en directorio temporal para smoke tests. Separar generador de audio técnico hacia tests/. Borrar examples de video y fotos preview del árbol main, y el script de descargar GSAP para demo. Generalizar rótulos estáticos de visualización. Reescribir README para usuarios del motor con su propio HTML y medios, aclarando licencias y límites. Mantener tests de lógica y browser con fixture sintética.

# Pendientes
Windows real, endurecimiento de Chrome CDP para HTML no confiable, compatibilidad general HTML/audio multiformato, licencia/patentes y benchmark 60 FPS.

# Próximos pasos
Versionar limpieza y promover commit por fast-forward a main, verificar árbol remoto. Ramas demo y develop se conservan para mantener antecedentes sin volver a incorporar medios a main.
