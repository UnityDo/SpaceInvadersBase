import json
import glob
import os
from statistics import mean, stdev

logs_dir = os.path.join(os.path.dirname(__file__), '..', '..', 'logs')
logs_dir = os.path.abspath(logs_dir)
pattern = os.path.join(logs_dir, 'run_*.json')
files = glob.glob(pattern)

groups = {}
no_params = []

for f in files:
    try:
        with open(f, 'r', encoding='utf-8') as fh:
            j = json.load(fh)
    except Exception as e:
        print(f'WARN: could not read {f}: {e}')
        continue
    key = None
    if 'params' in j and isinstance(j['params'], dict):
        # create a deterministic key from params
        items = sorted(j['params'].items())
        key = tuple(items)
    else:
        key = ('__no_params__',)
    entry = {
        'file': os.path.basename(f),
        'seed': j.get('seed'),
        'latency': j.get('powerup_pickup_latency_avg'),
        'powerups': j.get('powerups_collected'),
        'duration': j.get('duration_seconds'),
        'objective': j.get('objective')
    }
    groups.setdefault(key, []).append(entry)

# summarize
summary = []
for k, v in groups.items():
    latencies = [e['latency'] for e in v if e['latency'] is not None]
    powers = [e['powerups'] for e in v if e['powerups'] is not None]
    objs = [e['objective'] for e in v if e['objective'] is not None]
    s = {
        'key': k,
        'count': len(v),
        'latency_mean': mean(latencies) if latencies else None,
        'latency_std': stdev(latencies) if len(latencies) > 1 else None,
        'powerups_mean': mean(powers) if powers else None,
        'objective_mean': mean(objs) if objs else None,
        'samples': [e['file'] for e in v]
    }
    summary.append(s)

# sort by latency_mean asc (lower is better)
summary_sorted = sorted([s for s in summary if s['latency_mean'] is not None], key=lambda x: x['latency_mean'])

print('\nLogs analyzed: %d files in %s\n' % (len(files), logs_dir))
if not summary_sorted:
    print('No runs with latency metrics found.')
else:
    print('Top candidates by average powerup_pickup_latency_avg (lower is better):\n')
    for s in summary_sorted[:10]:
        key = s['key']
        if key == ('__no_params__',):
            key_str = 'NO_PARAMS (baseline / older runs)'
        else:
            key_str = ','.join([f'{k}={v}' for k,v in key])
        print(f"- {key_str}\n  samples={s['count']}, latency_mean={s['latency_mean']:.3f}, latency_std={s['latency_std']}, powerups_mean={s['powerups_mean']}, objective_mean={s['objective_mean']}")

# attempt to read optuna study best params if present
try:
    import optuna
    storage = 'sqlite:///Data/optuna.db'
    study = optuna.load_study(study_name='space_inv', storage=storage)
    print('\nOptuna study found: best value =', study.best_value)
    print('Best params:', study.best_trial.params)
except Exception as e:
    print('\nOptuna study not loaded or not found:', e)

print('\nDone.')
