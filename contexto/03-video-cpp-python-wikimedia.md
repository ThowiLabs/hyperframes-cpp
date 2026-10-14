# Fecha
2026-10-14

# Objetivo
Crear un video educativo vertical de seis escenas sobre qué es C++ y por qué puede superar a Python en tareas computacionalmente intensivas, con imágenes de Wikimedia Commons y voz en español mexicano.

# Decisiones tomadas
- Composición nueva en `samples/cpp-vs-python/`, sin alterar el motor ni la rama estable `main`.
- Diseño editorial de alto contraste (fondo azul oscuro, acentos verde menta, cian y dorado) y seis escenas distintas.
- Renderizar con `theme=browser` para aprovechar HTML/CSS/GSAP auténticos; usar C++ para codificar directamente MP4 H.264/AAC.
- 720x1280 a 24 FPS, 75 segundos según duraciones verificadas de seis WAV originales.
- Fotografías de Wikimedia Commons con enlaces, autores y licencias verificados vía MediaWiki API; atribución en video y `CREDITS.md`.
- Narración en español mexicano con `edge-tts`. FFmpeg solo en preproducción para convertir MP3 a WAV PCM 24 kHz; no se usa FFmpeg en la exportación C++.
- No inventar multiplicadores de rendimiento; explicar diferencias de implementación y aplicaciones de NumPy.

# Arquitectura actual
`download_wikimedia.py` obtiene imágenes y datos de Commons → `prepare_voice.py` produce PCM + `timeline.json` → `create_html.py` inserta tiempos en `index.template.html` → `index.html` con GSAP original provisto localmente → binario C++ + navegador Chromium → fotogramas → codificador H.264/AAC integrado → MP4.

# Librerías usadas
C++17 renderer preexistente, Chromium 155 instalado en sandbox, GSAP 3.14.2 local no versionado bajo licencia propia, Python stdlib para API Commons, edge-tts solo creación de voz, FFmpeg/ffprobe solo conversión de voz/validación.

# Archivos importantes modificados
Se añaden `samples/cpp-vs-python/{README.md,CREDITS.md,index.template.html,index.html,create_html.py,prepare_voice.py,download_wikimedia.py,timeline.json,assets/img/*,assets/wikimedia_sources.json}` y un archivo `.gitignore` que excluye audio WAV/MP3/GSAP y renders.

# Problemas encontrados
Tras el cambio de MCP el sandbox conservó /workspace, pero Chromium y FFmpeg no estaban instalados. Se restauraron. Las primeras conversiones de voz fallaron durante instalación y se repitieron tras finalizar los paquetes.
El lienzo sigue basado en 720x1280 y se usa código compilado previamente.

# Soluciones implementadas
- Fotografías descargadas y licencias validadas (CC0, CC BY 2.0, CC BY 4.0).
- TTS en 6 WAV PCM mono a 24 kHz, suma ~75 s; duración final redondeada a 75 s para sincronía audio/video.
- Capturas de navegador en escenas 1, 3, 5 y 6 visualmente comprobadas.
- Motor C++ en producción renderizando la composición completa a 720x1280, 24 FPS.

# Pendientes
Confirmar duración y cantidad de cuadros MP4 con ffprobe, decodificación completa sin errores, y URL de descarga temporal HTTPS. Los MP4 y WAV generados no se versionan.

# Próximos pasos
Verificar el archivo final y publicar enlace; realizar commit en rama de demo, notificar finalización por Telegram.

# Entrega validada
Archivo final `/workspace/hyperframes-cpp/entrega/22-cpp-vs-python-wikimedia-720p-24fps.mp4` (3,381,328 bytes). Codec video H.264 720x1280, 24fps, 1800 cuadros; audio AAC mono 24kHz, ambas pistas 75.000 segundos. ffprobe validó codecs/duración y ffmpeg decodificó como prueba QA sin errores. URL temporal https://f8d79758bdf2cd5af6.gradio.live/22-cpp-vs-python-wikimedia-720p-24fps.mp4 . Créditos https://f8d79758bdf2cd5af6.gradio.live/22-cpp-vs-python-wikimedia-CREDITS.md . El navegador se instaló nuevamente tras migración MCP. La rama es demo/cpp-vs-python-wikimedia para aislar la composición del desarrollo general.
