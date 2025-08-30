import argparse
import json
import os
import random
import subprocess
import time
from statistics import mean, stdev

parser = argparse.ArgumentParser()
parser.add_argument('--n', type=int, default=30, help='Number of validation runs')
parser.add_argument('--timeout', type=int, default=120, help='Per-run timeout seconds')
parser.add_argument('--study-name', default='space_inv')
parser.add_argument('--storage', default='sqlite:///Data/optuna.db')
parser.add_argument('--seed-start', type=int, default=None)
args = parser.parse_args()

# locate paths
script_dir = os.path.dirname(__file__)
proj_root = os.path.abspath(os.path.join(script_dir, '..', '..'))
exe_path = os.path.join(proj_root, 'SpaceInvaders.exe')
logs_dir = os.path.join(proj_root, 'logs')
os.makedirs(logs_dir, exist_ok=True)

# load best params from optuna
try:
    import optuna
    study = optuna.load_study(study_name=args.study_name if 'args' in globals() else 'space_inv', storage=args.storage if 'args' in globals() else 'sqlite:///Data/optuna.db')
    best = study.best_trial.params
except Exception as e:
    # fallback: try defaults or exit
    print('ERROR: could not load optuna study:', e)
    raise SystemExit(1)

# normalize param names
occlusion_penalty = str(best.get('occlusion_penalty'))
enemy_w = str(best.get('enemy_w'))
enemy_h = str(best.get('enemy_h'))

print('Best params loaded:', best)
print(f'Will run {args.n} headless runs using {exe_path}')

results = []

for i in range(args.n):
    seed = args.seed_start + i if args.seed_start is not None else random.randint(1, 2**31-1)
    env = os.environ.copy()
    env['AI_OCCLUSION_PENALTY'] = occlusion_penalty
    env['AI_ENEMY_W'] = enemy_w
    env['AI_ENEMY_H'] = enemy_h

    print(f'Run {i+1}/{args.n} seed={seed} ...', flush=True)
    try:
        subprocess.run([exe_path, '--autoplay', '--headless', '--seed', str(seed)], env=env, check=True, timeout=args.timeout)
    except subprocess.TimeoutExpired:
        print(f'  TIMEOUT for seed {seed}')
        results.append({'seed': seed, 'error': 'timeout'})
        continue
    except subprocess.CalledProcessError as e:
        print(f'  PROCESS ERROR for seed {seed}:', e)
        results.append({'seed': seed, 'error': 'process_error'})
        continue

    # try to load log
    log_file = os.path.join(logs_dir, f'run_{seed}.json')
    if not os.path.exists(log_file):
        # try to find any recent run (best-effort)
        print(f'  log file for seed {seed} not found at {log_file}', flush=True)
        # fallback: search for newest file
        candidates = [os.path.join(logs_dir, fn) for fn in os.listdir(logs_dir) if fn.startswith('run_') and fn.endswith('.json')]
        if not candidates:
            results.append({'seed': seed, 'error': 'no_log'})
            continue
        log_file = max(candidates, key=os.path.getmtime)
        print(f'  using fallback log {os.path.basename(log_file)}')

    try:
        with open(log_file, 'r', encoding='utf-8') as fh:
            j = json.load(fh)
    except Exception as e:
        print('  ERROR reading log:', e)
        results.append({'seed': seed, 'error': 'read_error'})
        continue

    j['validated_seed'] = seed
    j['log_file_used'] = os.path.basename(log_file)
    results.append(j)
    time.sleep(0.1)

# aggregate
latencies = [r.get('powerup_pickup_latency_avg') for r in results if isinstance(r, dict) and r.get('powerup_pickup_latency_avg') is not None]
powerups = [r.get('powerups_collected') for r in results if isinstance(r, dict) and r.get('powerups_collected') is not None]
objectives = [r.get('objective') for r in results if isinstance(r, dict) and r.get('objective') is not None]

summary = {
    'params': best,
    'runs_requested': args.n,
    'runs_completed': len([r for r in results if isinstance(r, dict) and r.get('error') is None and r.get('powerup_pickup_latency_avg') is not None]),
    'latency_mean': mean(latencies) if latencies else None,
    'latency_std': stdev(latencies) if len(latencies) > 1 else None,
    'powerups_mean': mean(powerups) if powerups else None,
    'objective_mean': mean(objectives) if objectives else None,
    'samples': [{'seed': r.get('seed', r.get('validated_seed')), 'latency': r.get('powerup_pickup_latency_avg'), 'powerups': r.get('powerups_collected'), 'log': r.get('log_file_used'), 'error': r.get('error')} for r in results]
}

out_path = os.path.join(logs_dir, 'validation_best.json')
with open(out_path, 'w', encoding='utf-8') as fh:
    json.dump(summary, fh, indent=2)

print('\nValidation summary written to', out_path)
print('latency_mean=', summary['latency_mean'], 'latency_std=', summary['latency_std'], 'powerups_mean=', summary['powerups_mean'])
