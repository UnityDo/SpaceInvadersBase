Training — parámetros y pasos para tuning con Optuna

Resumen rápido
- Objetivo: afinar parámetros de la IA (p. ej. `AI_OCCLUSION_PENALTY`, `AI_ENEMY_W`, `AI_ENEMY_H`) usando Optuna ejecutando partidas en modo autoplay y consumiendo los logs JSON por run.
- Resultado por run: `logs/run_<seed>.json` (contiene métricas usadas por el optimizador).

Checklist previo
- Código compilado (el ejecutable `SpaceInvaders.exe` debe reflejar los cambios de telemetría).
- Entorno Python creado en `.venv` (contiene `python.exe`, `optuna`, `tqdm`).
- El script de tuning está en `tools/tune_params_optuna.py`.

Métricas escritas en cada run (per-run JSON)
- seed
- duration_seconds: segundos de la partida
- max_level
- lives
- score
- powerups_collected
- enemy_hits_taken
- shots_fired
- time_idle
- powerup_pickup_count
- powerup_pickup_latency_avg

Pasos rápidos (corrida corta de prueba)
1) Abrir terminal en la carpeta del juego:

```cmd
cd /d "c:\Users\Usuario\Documents\UnReal\Clases\Preguntas\CPlusplus_Games\SpaceInvaders"
```

2) Activar virtualenv (opcional) y ejecutar el script de tuning (usa el Python dentro de `.venv`):

```cmd
.venv\Scripts\python.exe tools\tune_params_optuna.py
```

- El script está preparado para usar `SEEDS_PER_TRIAL` y `N_TRIALS` definidos en su cabecera; para una prueba corta ajusta a 2 seeds / 5 trials.
- El script lanza `SpaceInvaders.exe --autoplay --seed <seed>` y lee `logs/run_<seed>.json`.

Ejecutar tuning en modo no supervisado (recomendado)
- Usa `--autoplay` para que la IA juegue en lugar de un humano.
- Si quieres evitar que aparezca la ventana y que el proceso quede bloqueado en una pantalla, usa `--headless` (el juego ya parsea `--headless` y `--autoplay`).
- Tras cada final de nivel el juego escribe `logs/run_<seed>.json` y, si corre en autoplay/headless, cierra el proceso automáticamente.

Ajustar el objetivo del optimizador
- El objetivo compuesto por defecto (archivo `tools/tune_params_optuna.py`) es:

  objective = duration_seconds*1.0 + enemy_hits*8.0 + avg_pickup_latency*0.5 - powerups_collected*6.0

- Para priorizar vidas sobre duración sube `w_h` (p. ej. 12.0). Para priorizar recoger powerups sube el valor absoluto de `w_p`.
- Edita los pesos en `tools/tune_params_optuna.py` y vuelve a lanzar.

Validación de parámetros elegidos
1) Ejecuta el tuning (largo) y guarda `study.best_params`.
2) Haz una validación con el mejor set: ejecutar `SpaceInvaders.exe --autoplay --seed <seed>` en un loop para 50–200 seeds y recopilar métricas.
3) Comparar medias (duration, enemy_hits, powerups_collected) vs baseline.

Cómo detener una corrida
- En la terminal presiona Ctrl+C para parar el script Python (Optuna). También puedes cerrar procesos con el Task Manager si algo queda atascado.

Problemas conocidos y recomendaciones
- Si ves muchos runs sin telemetría extendida, esos logs corresponden a ejecutables compilados antes del parche. Recompila y elimina logs viejos para comparaciones limpias.
- El juego puede abrir ventanas por cada run; usar `--headless` reduce interacción visual. Si el juego no soporta totalmente headless (rendering), el flag ya evita que el proceso se quede en la pantalla del nivel (implementamos un exit automático tras escribir el log).
- Para resultados robustos usa N_TRIALS >= 30 y SEEDS_PER_TRIAL >= 3 (más seeds reduce ruido, pero aumenta coste).

Ubicaciones importantes
- Script de tuning: `tools/tune_params_optuna.py`
- Logs por run: `logs/run_<seed>.json`
- Código IA: `tools/ai/AIController.h`
- Telemetría y salida JSON: `Core/Game.cpp` -> `SaveGameHistoryEntry()`

Sugerencia práctica
- Hacer 2 fases: 1) corrida corta (5–10 trials) para validar pipeline; 2) corrida larga (30–100 trials) para explorar; 3) validación estadística extensa del mejor resultado.

Contacto
- Si quieres, lanzo el tuning largo ahora y monitorizo el progreso; dime cuántos trials/seeds deseas.
