# C++ vs Python: ¿por qué cambia el rendimiento?

Demo educativa vertical de **75 segundos**, seis escenas, narración en español mexicano y cuatro fotografías de Wikimedia Commons. Renderizada con el tema `browser` de HyperFrames C++, que utiliza un navegador Chromium/Chrome/Edge instalado para dibujar HTML, CSS y transiciones de GSAP, mientras los codificadores nativos C/C++ producen MP4 H.264/AAC.

## Estructura

| Archivo | Propósito |
|---|---|
| `index.html` | Composición final HTML (720 × 1280, 24 FPS) |
| `index.template.html` | Plantilla reutilizable para otros tiempos de voz |
| `timeline.json` | Inicio, duración y guion de las seis escenas |
| `download_wikimedia.py` | Descarga fotos y metadatos de autor/licencia desde Wikimedia Commons |
| `prepare_voice.py` | Genera narración española en seis WAV PCM mono 24 kHz, 16 bits |
| `create_html.py` | Regenera `index.html` usando los tiempos reales del audio |
| `CREDITS.md` | Autores, licencias y URL original de cada fotografía |
| `assets/img/` | Fotografías Wikimedia con atribución |
| `assets/audio/` | Archivos de voz generados localmente (no se versionan) |

## Cómo reproducir la demo

Desde la raíz del repositorio:

```bash
./scripts/build-native-mp4.sh
cd samples/cpp-vs-python

# Opcional si los archivos en assets/img ya están disponibles:
python3 download_wikimedia.py

# GSAP tiene licencia independiente: revisar https://gsap.com/standard-license
# El archivo se descarga localmente y nunca se versiona en el repositorio:
mkdir -p assets/vendor
curl -fLsS https://cdnjs.cloudflare.com/ajax/libs/gsap/3.14.2/gsap.min.js \
  -o assets/vendor/gsap.min.js

# La narración requiere edge-tts y FFmpeg únicamente para preparar los WAV:
# pip install edge-tts
python3 prepare_voice.py
python3 create_html.py

../../build/native-linux/hyperframes-cpp-native --browser-check
../../build/native-linux/hyperframes-cpp-native index.html assets/audio \
  cpp-vs-python.mp4 24 h264 browser 720x1280
```

El proceso de exportación de video **no usa FFmpeg**: obtiene los fotogramas de Chrome DevTools Protocol y los codifica con H.264/AAC mediante las bibliotecas incluidas en el ejecutable C++. FFmpeg en este ejemplo solo convierte el audio de narración descargado al WAV PCM esperado por el muxer.

## Guion editorial

1. **C++ vs Python.** Ambos programan, pero ejecutan de forma diferente.
2. **C++ y el hardware.** Control de recursos y aplicaciones exigentes.
3. **Compilación.** Del código fuente a instrucciones nativas.
4. **CPython.** Bytecode, intérprete y objetos dinámicos.
5. **Comparación justa.** Bibliotecas compiladas y benchmarks reales.
6. **Conclusión.** Rendimiento frente a velocidad de desarrollo.

No se afirman multiplicadores de velocidad ni benchmarks ficticios: el resultado depende del problema, algoritmo, compilador, ejecución y librerías. Python puede alcanzar gran rendimiento con bibliotecas nativas.

## Licencias y notas de redistribución

Las imágenes son originales de **Wikimedia Commons**, disponibles bajo CC0, CC BY 2.0 y CC BY 4.0. Se indica autor/licencia en la composición, y los enlaces y descripciones completas están en [CREDITS.md](CREDITS.md). Los recortes y oscurecimientos se realizan como transformaciones visuales del video.

La música no se utiliza. El texto de narración es original para este video; el WAV generado por síntesis de voz no forma parte del código fuente versionado. GSAP se utiliza desde un archivo separado, con licencia propia, y no se incluye en este repositorio.

La demo es orientativa: no evalúa el rendimiento de las bibliotecas ni convierte cualquier HTML sin adaptar. Para programas ejecutables generados por usuarios, validar sus dependencias y licencias por separado.
