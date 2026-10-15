# HyperFrames C++ v1.0.1 — detección de navegador por procesos

Actualización de compatibilidad para Linux x64 y Windows x64 sobre v1.0.0.

## Cambios

- La búsqueda automática del navegador incluye ejecutables de procesos Chrome/Chromium/Edge/Brave ya activos, **sin conectarse a sus pestañas ni reutilizar su perfil**.
- En Linux consulta `/proc/<pid>/exe`; en Windows enumera procesos y obtiene rutas con `QueryFullProcessImageNameW`.
- Windows utiliza API Unicode para `PATH`, `HFC_BROWSER`, directorios de instalación y lanzamiento de Chrome mediante `CreateProcessW`, evitando fallos con nombres de usuario que contengan acentos o caracteres no ASCII.
- Nuevo comando `--browser-process-check` para comprobar únicamente la ruta de un navegador ejecutándose.
- Sin dependencias añadidas, ni cambios en formato HTML ni códecs.

## Diagnóstico

```bash
hyperframes-cpp-native --browser-check
hyperframes-cpp-native --browser-process-check
```

El primer comando aplica la búsqueda completa: `HFC_BROWSER`, `PATH`, instalaciones habituales y, como alternativa, procesos. El segundo consulta exclusivamente procesos activos.

El navegador sigue siendo una **dependencia externa del modo browser**. El ejecutable lanza una instancia headless con perfil temporal e independiente.

## Plataformas y límites

Linux x64 verificado con pruebas de proceso y rutas con espacios/UTF-8, además de pruebas funcionales del motor. Windows x64 compilado cruzado; se requiere validación en un Windows real. Solo navegadores basados en Chromium. La codificación MP4 H.264/AAC continúa sin invocar FFmpeg. El audio de entrada sigue siendo WAV PCM mono 24 kHz.

## Paquetes

Los binarios verificados Linux/Windows y la Agent Skill están alojados en la rama `release-assets-v1.0.1`, enlazada desde el release. El proveedor conectado no autoriza subir adjuntos directamente a la sección Assets de GitHub Releases. SHA256SUMS.txt permite comprobar la integridad.
