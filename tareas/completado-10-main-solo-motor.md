# Tarea 10: publicar motor limpio en main

Estado: completado; smoke tests Linux, prueba CDP/H.264/AAC y compilaciones Linux/Windows verificadas; promoción a main solicitada.

Objetivo: mover al branch main el motor C++ con navegador Chromium/CDP, MP4/H.264/AAC nativo, AVI, documentación y tests técnicos; excluir proyectos de video, audios, fotografías, composiciones y renders de ejemplos.

Criterios: no incluir ruta samples/ ni docs/images de demostraciones; reemplazar dependencia de pruebas sobre los ejemplos por fixtures sintéticas autocontenidas; actualizar README y docs; compilar Linux/Windows y probar el motor; publicar main sin force push y comprobar el árbol remoto.
