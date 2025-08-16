README — Tests IA para SpaceInvaders

Propósito

Este directorio contiene un controlador IA ligero (`AIController`) pensado para pruebas automatizadas y tuning fuera del juego principal. Este README explica cómo ejecutar partidas automatizadas, cómo garantizar reproducibilidad y sugerencias para recoger métricas.

Resumen rápido

- Archivo IA principal: `tools/ai/AIController.h` (implementación inline).
- Flags de ejecución soportadas por el juego (en `Game::Init`):
  - `--autoplay` : ejecutar con el controlador IA en lugar del humano.
  - `--seed N`   : semilla numérica para reproducibilidad (opcional).
  - `--headless` : intención de ejecutar sin ventana (si el juego está adaptado para ello).

Cómo hacer una ejecución simple

1) Compila el juego desde la raíz del proyecto (Windows/cmd):

```cmd
build.bat
```

2) Ejecuta una partida con IA (reproducible con seed):

```cmd
.\SpaceInvaders.exe --autoplay --seed 42
```

Si no pasas `--seed`, la IA generará una semilla basada en el reloj.

Ejecuciones en lote (batch) para recolectar estadísticas

Ejemplo sencillo en `cmd.exe` para ejecutar 50 seeds diferentes y guardar logs por run:

```cmd
for /L %i in (1,1,50) do .\SpaceInvaders.exe --autoplay --seed %i > logs\run_%i.txt
```

(Si lo ejecutas desde un script `.bat`, duplica el `%`: `%%i`.)

Notas sobre headless

- `--headless` está parseado por `Game::Init` pero el juego puede necesitar más cambios para no inicializar la ventana/renderer. Si `--headless` no evita la creación de la ventana, puedes:
  1) Modificar `Game::Init` para saltar `renderer->Init()` y la creación de la ventana cuando `headless==true`.
  2) Asegurarte de que el `AIController` no llame a APIs gráficas.

Reproducibilidad y logging mínimo recomendado

- Pasa siempre `--seed` para reproducir runs.
- Registra las métricas por run en un archivo JSON/CSV (por ejemplo dentro de `logs/`): nivel, seed, drops generados, drops recogidos, duración del nivel, vidas restantes, score.
- Para recopilar métricas desde dentro del juego, añade una función que al final de la partida (o por nivel) escriba un JSON con las métricas en `logs/run_<seed>_<timestamp>.json`.

Ideas para el pipeline de tuning (siguientes pasos)

1) Habilitar modo totalmente headless y logging estructurado.
2) Crear `tools/tune_levels.py` (Python) que haga:
   - lanzar múltiples ejecuciones con `subprocess` y capturar los archivos de métricas,
   - agregar un optimizador (Optuna / Nevergrad) para ajustar parámetros (p. ej. `dropChance` por nivel),
   - evaluar función objetivo (p. ej. diferencia respecto al `target_drops` y estabilidad).
3) Guardar checkpoints y visualizaciones (histogramas, series temporales) para revisar resultados.

Extensión de `AIController`

- Si vas a usar la IA para playtesting serio, amplía la interfaz para recibir observaciones (posiciones de enemigos/powerups). Actualmente `AIController` tiene métodos `ObservePlayerX` y `ObserveEnemiesCount` como ejemplo. Modifica `Game` para rellenar y llamar a esas observaciones antes de `controller->Update(dt)`.

Contacto rápido

Si quieres, implemento ahora:
- modo `--headless` totalmente funcional (evitar renderer),
- logging JSON de métricas por run,
- un pequeño script `tools/tune_levels.py` que haga una pasada de tuning con Optuna.

Elige la siguiente acción y la implemento: `headless` / `logging` / `tuning script` / `ninguna`.
