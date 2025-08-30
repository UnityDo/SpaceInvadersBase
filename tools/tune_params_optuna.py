import optuna, subprocess, json, os, time, random, argparse
from pathlib import Path

ROOT = Path(r"c:\Users\Usuario\Documents\UnReal\Clases\Preguntas\CPlusplus_Games\SpaceInvaders")
GAME_EXE = ROOT / "SpaceInvaders.exe"
LOGS_DIR = ROOT / "logs"
LOGS_DIR.mkdir(exist_ok=True)


def compute_objective_from_log(data):
    duration = float(data.get("duration_seconds", 0.0))
    enemy_hits = float(data.get("enemy_hits_taken", 0))
    pickup_avg = float(data.get("powerup_pickup_latency_avg", 0.0))
    powerups = float(data.get("powerups_collected", 0))
    return duration + enemy_hits * 8.0 + pickup_avg * 0.5 - powerups * 6.0


def run_game_with_params(params, seed, timeout=180):
    env = os.environ.copy()
    env["AI_OCCLUSION_PENALTY"] = str(params["occlusion_penalty"])
    env["AI_ENEMY_W"] = str(params["enemy_w"])
    env["AI_ENEMY_H"] = str(params["enemy_h"])
    cmd = [str(GAME_EXE), "--autoplay", "--seed", str(seed)]
    logfile = LOGS_DIR / f"run_{seed}.json"
    if logfile.exists():
        try:
            logfile.unlink()
        except Exception:
            pass
    try:
        proc = subprocess.run(cmd, env=env, capture_output=True, text=True, timeout=timeout)
    except subprocess.TimeoutExpired:
        print(f"[tune] Run seed {seed} timed out after {timeout}s")
        data = {"seed": seed}
        data["params"] = params
        data["objective"] = None
        try:
            logfile.write_text(json.dumps(data))
        except Exception:
            pass
        return data

    data = {}
    if logfile.exists():
        try:
            data = json.loads(logfile.read_text())
        except Exception as e:
            print("[tune] Failed to parse run log:", e)
    else:
        out = proc.stdout.strip()
        if out:
            try:
                data = json.loads(out.splitlines()[-1])
            except Exception:
                data = {}

    if data is None:
        data = {}

    data["params"] = params
    data["seed"] = seed
    try:
        data["objective"] = compute_objective_from_log(data)
    except Exception:
        data["objective"] = None

    try:
        logfile.write_text(json.dumps(data))
    except Exception as e:
        print("[tune] Failed to write run log:", e)

    return data


def objective(trial, seeds_per_trial, timeout):
    params = {
        "occlusion_penalty": trial.suggest_float("occlusion_penalty", 0.0, 20.0),
        "enemy_w": trial.suggest_float("enemy_w", 20.0, 80.0),
        "enemy_h": trial.suggest_float("enemy_h", 15.0, 50.0),
    }
    seeds = [random.randint(1, 1000000) for _ in range(seeds_per_trial)]
    vals = []
    for s in seeds:
        print(f"[tune] trial {trial.number} seed {s} params {params}")
        data = run_game_with_params(params, s, timeout=timeout)
        obj = data.get("objective")
        if obj is None:
            # fallback: if no objective, use negative score so higher scores are better
            obj = -float(data.get("score", 0))
        vals.append(obj)
    avg = sum(vals) / len(vals)
    print(f"[tune] trial {trial.number} avg_obj={avg}")
    return avg


def parse_args():
    p = argparse.ArgumentParser()
    p.add_argument("--trials", type=int, default=int(os.environ.get("N_TRIALS", 30)))
    p.add_argument("--seeds", type=int, default=int(os.environ.get("SEEDS_PER_TRIAL", 3)))
    p.add_argument("--timeout", type=int, default=int(os.environ.get("RUN_TIMEOUT", 180)))
    p.add_argument("--study-name", type=str, default=os.environ.get("OPTUNA_STUDY_NAME", "space_inv"))
    p.add_argument("--storage", type=str, default=os.environ.get("OPTUNA_STORAGE", "sqlite:///Data/optuna.db"))
    return p.parse_args()


if __name__ == "__main__":
    args = parse_args()
    N_TRIALS = args.trials
    SEEDS_PER_TRIAL = args.seeds
    TIMEOUT = args.timeout
    STUDY_NAME = args.study_name
    STORAGE = args.storage

    # Ensure Data folder exists for sqlite DB
    data_dir = ROOT / "Data"
    try:
        data_dir.mkdir(exist_ok=True)
    except Exception:
        pass

    print(f"[tune] GAME_EXE={GAME_EXE}")
    print(f"[tune] logs dir = {LOGS_DIR}")
    print(f"[tune] Optuna storage = {STORAGE}, study = {STUDY_NAME}")

    study = optuna.create_study(storage=STORAGE, study_name=STUDY_NAME, load_if_exists=True, direction="minimize")
    try:
        study.optimize(lambda t: objective(t, SEEDS_PER_TRIAL, TIMEOUT), n_trials=N_TRIALS, n_jobs=1)
    except KeyboardInterrupt:
        print("[tune] interrupted by user")

    print("Best params:", study.best_params)
    print("Best value (minimized):", study.best_value)
