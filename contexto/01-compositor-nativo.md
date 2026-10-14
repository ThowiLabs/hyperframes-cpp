# Fecha
2026-10-14

# Objetivo
Evolucionar el prototipo publicado de HyperFrames C++ hacia un compositor nativo profesional con animación independiente de FPS, resoluciones de salida configurables y primeras reglas CSS vinculadas al dibujo; preservar el branch main estable.

# Decisiones tomadas
- Desarrollo en `develop/native-compositor`, sin sobrescribir main.
- Escalonar compatibilidad: motor de animación determinista + subconjunto CSS real antes de sumar dependencias grandes como litehtml/Blend2D o QuickJS.
- Mantener AVI MJPEG/RLE8/RGB24 y audio PCM disponibles; H.264/MP4 nativo es trabajo futuro con revisión de licencias/patentes y pruebas reales.
- El lienzo lógico es 720x1280; el archivo de salida puede escalarse a resoluciones 9:16 pares desde 180x320 hasta 1080x1920. Otros aspectos quedan rechazados hasta implementar layout responsivo real.
- Reutilizar las bibliotecas stb y las fuentes integradas; no agregar dependencias nuevas de ejecución.

# Arquitectura actual
- `animation_timeline.hpp`: curvas easing y tracks de movimiento, escala y opacidad, deterministas por tiempo.
- `output_scale.hpp`: validación de resoluciones y escalado bilineal RGB/BGR independiente del códec.
- `css_properties.hpp`: lector acotado de declaraciones CSS px, con coincidencia de selector exacto y última declaración que coincida.
- `html_composition.hpp`: carga escenas, textos y tamaño/posición de layout para el tema clásico.
- `render_scene.hpp` y `render_news.hpp`: dos temas; usan interpolación de animación nativa.
- `main.cpp` y `avi_rle8.hpp`: AVI y códecs ahora admiten dimensiones de salida distintas al lienzo lógico.

# Librerías usadas
Solo C++17 estándar y las dos bibliotecas de cabecera stb vendorizadas; ninguna nueva dependencia de ejecución. Compiladores g++ Linux y MinGW-w64 Windows x64.

# Archivos importantes modificados
animation_timeline.hpp, output_scale.hpp, css_properties.hpp, avi_rle8.hpp, main.cpp, html_composition.hpp, render_scene.hpp, render_news.hpp, tests/*.cpp, tests/smoke-linux.sh, README.md, docs/ARCHITECTURE.md, docs/ROADMAP.md, tareas/*.md.

# Problemas encontrados
- El proyecto original es un renderizador de un subconjunto conocido y no procesa CSS/GSAP/DOM generales.
- Su código contiene medidas W/H constantes y cabezal AVI 32-bit.
- Ejecutar JavaScript de composiciones HTML no equivale a ofrecer una DOM de navegador.
- AVI MJPEG es portable, pero no es MP4 y ciertos tamaños excederán el límite RIFF de 4 GiB.

# Soluciones implementadas
- `Timeline::Track` permite secuencias no superpuestas, easing lineal/power/expo/back y muestreo no dependiente de frames.
- Estilos CSS reales de los elementos conocidos afectan posiciones y fuente del renderizador clásico; prueba automatizada modifica `font-size` y comprueba que AVI cambia.
- 360x640 MJPEG, 720x1280 RLE8 y 1080x1920 MJPEG verificados mediante ffprobe con duración 67 s y pista PCM.
- Compilación Linux estática y Windows x64 cruzada exitosa, suite smoke y tests unitarios correctos.

# Pendientes
1. Integrar layout HTML/CSS completo o litehtml detrás de interfaz controlada; no dar compatibilidad universal sin pruebas.
2. Render avanzado (Blend2D) con fondo, sombras y mezcla alpha de mayor fidelidad.
3. Códec H.264/AAC y muxer MP4 nativo, con revisión de licencias de distribución.
4. Runtime QuickJS opt-in con DOM limitada, cuotas y aislamiento, nunca ejecutar HTML no confiable como root.
5. Windows real y reproducción en múltiples players, seguridad del parser y benchmark a 60 FPS.

# Próximos pasos
Probar litehtml y Blend2D en una rama experimental nueva, comparar fotogramas reales y promover los cambios solo si compilan sin dependencias dinámicas tanto Linux como Windows. Conservar el branch main tal como estaba al comenzar esta tarea.
