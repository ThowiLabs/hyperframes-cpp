# Fecha
2026-10-15

# Objetivo
Robustecer detección de navegadores Chrome/Chromium/Edge/Brave mediante ejecutables de procesos en Linux y Windows, y evitar fallos de rutas Unicode en Windows.

# Decisiones tomadas
Conservar prioridad HFC_BROWSER, PATH, instalaciones conocidas, luego procesos. No adjuntarse a sesiones existentes: solo obtener ruta del ejecutable para iniciar navegador headless con perfil independiente. Añadir --browser-process-check como diagnóstico.

# Arquitectura actual
Linux lee symlinks /proc/<pid>/exe y valida la ruta; Windows enumera procesos con CreateToolhelp32Snapshot y QueryFullProcessImageNameW. Rutas externas UTF-8; sistema Windows usa UTF-16 mediante CreateProcessW. Variables PATH/HFC_BROWSER/PROGRAMFILES leídas con GetEnvironmentVariableW.

# Librerías usadas
C++17 STL, Linux /proc, Win32 API, sin dependencias nuevas.

# Archivos importantes modificados
browser_detection.hpp, browser_capture.hpp, main.cpp, tests/browser-discovery.sh, README.md, docs/BROWSER_MODE.md, .claude/skills/hyperframes-cpp/SKILL.md, contexto/06-rutas-procesos-navegador.md, tareas/completado-12-procesos-navegador.md.

# Problemas encontrados
v1.0.0 detectaba rutas de procesos pero usaba CreateProcessA/variables de entorno ANSI en Windows: podía fallar con rutas Unicode o perfiles con caracteres especiales. No existía prueba CLI para inspección independiente de procesos activos.

# Soluciones implementadas
Conversión de ruta desde filesystem UTF-8, uso de CreateProcessW, lectura de rutas de entorno Windows con GetEnvironmentVariableW. Se añade --browser-process-check y prueba Linux de ejecución simulada desde un path UTF-8 con espacios. Cross-build Windows y pruebas de integración requeridos antes de publicar.

# Pendientes
Validar binario con Windows real (Linux solo puede compilar Windows, no ejecutarlo). Mejorar resolución de permisos y validación de binarios que se llamen chrome pero no sean Chromium.

# Próximos pasos
Verificar builds Linux/Windows y regresión, commit en español, push a main. Tag histórico v1.0.0 permanece sin cambios.
