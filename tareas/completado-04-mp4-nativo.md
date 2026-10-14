# Tarea 04: MP4 nativo con audio AAC
Estado: implementado experimentalmente con pruebas Linux y Windows cross-build.
minih264 CC0, minimp4 CC0 y VisualOn AAC Apache2 compilados y enlazados en código C/C++; C++ entrega los fotogramas, genera MP4 H.264/AAC directamente sin invocar FFmpeg. Video completo 67s a 720x1280 24fps validado, 1608 frames, 1571 bloques AAC duración igual y decodificación correcta.
Limitaciones: codificador minih264 experimental, rendimiento de 60fps pendiente, cumplimiento de patentes/licencias de H.264/AAC sujeto a revisión.
