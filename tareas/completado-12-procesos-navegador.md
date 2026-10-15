# Tarea 12: detección fiable del ejecutable de navegador desde procesos
Estado: completado. Linux/Windows compilados, pruebas de procesos, AVI, MP4 y Chromium superadas.
Objetivo: robustecer el detector Linux/Windows ya introducido en v1.0.0, particularmente rutas Unicode y espacios en Windows, y verificar prioridad y fallback ante navegadores no incluidos en PATH.
Criterios: seguir sin adjuntar ni controlar sesiones activas; usar ruta del ejecutable para lanzar un perfil independiente; no añadir dependencias; tests Linux con proceso sintético + compilación Windows; smoke AVI/MP4/CDP; documentación y push a main tras revisión.

La corrección añade lectura de variables Windows mediante GetEnvironmentVariableW, retorno de rutas UTF-8 y lanzamiento mediante CreateProcessW. Se agregó --browser-process-check. Linux prueba nombres con tildes y espacios mediante ejecutable simulado llamado chromium; Windows se valida por compilación cruzada y el runtime real permanece pendiente.
