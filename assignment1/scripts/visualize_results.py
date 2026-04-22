#!/usr/bin/env python3

import json
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
import matplotlib.ticker as ticker
from pathlib import Path
import sys

import matplotlib
matplotlib.use('Agg')

plt.style.use('seaborn-v0_8-whitegrid')
params = {
    'axes.labelsize': 12,
    'axes.titlesize': 13,
    'legend.fontsize': 11,
    'xtick.labelsize': 10,
    'ytick.labelsize': 10,
    'lines.linewidth': 2.5,
    'lines.markersize': 8,
    'figure.dpi': 300,
    'axes.grid': True,
    'grid.alpha': 0.3
}
plt.rcParams.update(params)

SCRIPT_DIR = Path(__file__).parent

if len(sys.argv) > 1:
    arg = sys.argv[1]
    if arg.startswith('@'):
        arg = arg[1:]

    data_path = Path(arg)
    if not data_path.is_absolute():
        data_path = SCRIPT_DIR.parent / arg
        if not data_path.exists():
            data_path = Path(arg)

    if not data_path.exists() or not data_path.suffix == '.json':
        sys.exit(f"Error: File not found or not a JSON file: {arg}")

    DATA_FILE = data_path
else:
    sys.exit("Usage: python visualize_results.py <path_to_json_file>\nExample: python visualize_results.py @benchmarks/increment.json")

IMG_DIR = SCRIPT_DIR.parent / "report" / "res" / "img"
IMG_DIR.mkdir(parents=True, exist_ok=True)

print(f"\nProcessing {DATA_FILE}...")
with open(DATA_FILE) as f:
    data = json.load(f)

def save_plot(filename, fig):
    path = IMG_DIR / filename
    fig.savefig(path, bbox_inches='tight', dpi=300)
    print(f"Saved: {path}")
    plt.close(fig)

if "thread_count" in data and "coarse_mutex" in data:
    print("Format: Transactions (Bank Benchmark)")
    
    thread_count = data.get('thread_count', 'Unknown')
    policies = [k for k in data.keys() if k not in ['thread_count', 'trials']]
    
    pretty_names = {
        'coarse_mutex': 'Coarse Mutex', 'fine_mutex': 'Fine Mutex',
        'coarse_rwlock': 'Coarse RWLock', 'fine_rwlock': 'Fine RWLock'
    }
    
    parsed = {p: {} for p in policies}
    configs = set()
    
    for policy in policies:
        for entry in data[policy]:
            key = (entry['accounts'], entry['transfers'], entry['query_pct'])
            configs.add(key)
            parsed[policy][key] = np.mean(entry['times'])

    sorted_configs = sorted(list(configs))
    unique_accounts = sorted(list(set(c[0] for c in sorted_configs)))
    unique_queries = sorted(list(set(c[2] for c in sorted_configs)))
    max_transfers = max(c[1] for c in sorted_configs)

    target_acc = unique_accounts[-1]

    fig, ax = plt.subplots(figsize=(10, 6))
    colors = plt.cm.tab10(np.linspace(0, 1, len(policies)))
    markers = ['o', 's', '^', 'D']
    
    for idx, policy in enumerate(policies):
        x_vals = []
        time_vals = []
        
        for q in unique_queries:
            key = (target_acc, max_transfers, q)
            if key in parsed[policy]:
                time_s = parsed[policy][key]
                time_vals.append(time_s)
                x_vals.append(q)
        
        ax.plot(x_vals, time_vals, marker=markers[idx], label=pretty_names.get(policy, policy), 
                color=colors[idx], markersize=8, linewidth=2.5)

    ax.set_xlabel("Query Percentage (Read-Only %)")
    ax.set_ylabel("Execution Time (Seconds)")
    ax.set_title(f"Execution Time vs Read/Write Ratio\n({target_acc:,} Accounts, {thread_count} Threads)")
    ax.legend(frameon=True, framealpha=0.9, shadow=True)
    ax.grid(True, linestyle='--', alpha=0.4)

    save_plot(f"{DATA_FILE.stem}_time.png", fig)

    baseline = 'coarse_mutex'
    comparisons = [p for p in policies if p != baseline]
    
    if baseline in parsed:
        fig, axes = plt.subplots(1, len(comparisons), figsize=(5*len(comparisons), 5))
        if len(comparisons) == 1: axes = [axes]
        
        for ax, policy in zip(axes, comparisons):
            grid = np.zeros((len(unique_accounts), len(unique_queries)))
            
            for i, acc in enumerate(unique_accounts):
                for j, q in enumerate(unique_queries):
                    key = (acc, max_transfers, q)
                    if key in parsed[baseline] and key in parsed[policy]:
                        base_t = parsed[baseline][key]
                        curr_t = parsed[policy][key]
                        grid[i, j] = base_t / curr_t
                    else:
                        grid[i, j] = np.nan
            
            im = ax.imshow(grid, cmap='RdBu_r', norm=mcolors.TwoSlopeNorm(vmin=0.1, vcenter=1.0, vmax=np.nanmax(grid)), 
                         aspect='auto')
            
            ax.set_title(f"{pretty_names.get(policy, policy)} vs Coarse Mutex")
            ax.set_xticks(range(len(unique_queries)))
            ax.set_xticklabels(unique_queries)
            ax.set_yticks(range(len(unique_accounts)))
            ax.set_yticklabels([f"{x//1000}k" for x in unique_accounts])
            ax.set_xlabel("Query %")
            if ax == axes[0]: ax.set_ylabel("Account Size")
            
            for i in range(len(unique_accounts)):
                for j in range(len(unique_queries)):
                    val = grid[i, j]
                    color = "white" if abs(val - 1.0) > 0.5 else "black"
                    ax.text(j, i, f"{val:.1f}x", ha="center", va="center", color=color, fontsize=9, fontweight='bold')

        plt.colorbar(im, ax=axes.ravel().tolist(), label='Speedup Factor (x)', fraction=0.02, pad=0.04)
        save_plot(f"{DATA_FILE.stem}_heatmaps.png", fig)

elif "size" in data and ("mutex_time" in data or "rwlock_time" in data):
    print("Format: Increment (Mutex vs RWLock) - New Format")

    sizes_raw = data['size']
    stats = {}
    all_iters = set()
    all_threads = set()

    for idx, size_str in enumerate(sizes_raw):
        parts = size_str.split('_')
        iterations = int(parts[0])
        threads = int(parts[1].replace('t', ''))

        all_iters.add(iterations)
        all_threads.add(threads)

        if 'mutex_time' in data and idx < len(data['mutex_time']):
            time_vals = data['mutex_time'][idx]
            if time_vals:
                stats[('mutex', iterations, threads)] = np.mean(time_vals)

        if 'rwlock_time' in data and idx < len(data['rwlock_time']):
            time_vals = data['rwlock_time'][idx]
            if time_vals:
                stats[('rwlock', iterations, threads)] = np.mean(time_vals)

        if 'atomic_time' in data and idx < len(data['atomic_time']):
            time_vals = data['atomic_time'][idx]
            if time_vals:
                stats[('atomic', iterations, threads)] = np.mean(time_vals)

    sorted_iters = sorted(list(all_iters))
    sorted_threads = sorted(list(all_threads))

    target_threads = 8

    if target_threads not in sorted_threads:
        print(f"Warning: {target_threads} threads not found in data. Available: {sorted_threads}")
        target_threads = sorted_threads[-1]

    fig, ax = plt.subplots(1, 1, figsize=(10, 6))

    colors = {'mutex': '#1f77b4', 'rwlock': '#d62728', 'atomic': '#2ca02c'}
    markers = {'mutex': 'o', 'rwlock': 's', 'atomic': '^'}
    labels = {'mutex': 'Mutex', 'rwlock': 'RWLock', 'atomic': 'Atomic'}

    for method in ['mutex', 'atomic', 'rwlock']:
        x_vals, y_vals = [], []
        for it in sorted_iters:
            if (method, it, target_threads) in stats:
                x_vals.append(it)
                y_vals.append(stats[(method, it, target_threads)] * 1000)  # Convert to ms

        if x_vals:
            ax.plot(x_vals, y_vals, marker=markers[method], label=labels[method],
                   color=colors[method], linewidth=2.5, markersize=8)

    ax.set_xscale('log')
    ax.set_yscale('log')
    ax.set_xlabel('Iterations', fontsize=12)
    ax.set_ylabel('Execution Time (ms)', fontsize=12)
    ax.set_title(f'Synchronization Mechanisms ({target_threads} Threads)', fontsize=13, fontweight='bold')
    ax.grid(True, which="both", linestyle='--', alpha=0.3)
    ax.xaxis.set_major_formatter(ticker.FuncFormatter(lambda x, p: f'{int(x):,}'))
    ax.legend(loc='upper left', fontsize=11, frameon=True, framealpha=0.9)

    plt.tight_layout()

    save_plot(f"{DATA_FILE.stem}_compare.png", fig)

elif "size" in data and "serial_time" in data:
    print("Format: Array Statistics")
    
    sizes = np.array(data['size'])
    serial_field = next((k for k in data.keys() if 'serial' in k), None)
    parallel_fields = [k for k in data.keys() if 'parallel' in k]
    
    fig, ax = plt.subplots(figsize=(10, 6))
    colors = plt.cm.tab10(np.linspace(0, 1, len(parallel_fields)))
    
    if serial_field:
        s_mean = np.mean(data[serial_field], axis=1)
        
        for idx, pf in enumerate(parallel_fields):
            p_mean = np.mean(data[pf], axis=1)
            speedup = np.divide(s_mean, p_mean, out=np.zeros_like(s_mean), where=p_mean!=0)
            
            label = pf.replace('parallel_time_', '').replace('_', ' ').title()
            ax.plot(sizes, speedup, marker='o', label=label, color=colors[idx], linewidth=2.5)

        ax.axhline(1, color='red', linestyle='--', alpha=0.5, label='Baseline (1x)')
        ax.set_xscale('log')
        ax.set_xlabel("Array Size")
        ax.set_ylabel("Speedup Factor")
        ax.set_title("Parallel Speedup vs Array Size")
        ax.legend()
        save_plot(f"{DATA_FILE.stem}_speedup.png", fig)

elif "results" in data and isinstance(data["results"], list):
    print("Format: Barrier")
    results = data["results"]
    policy_map = {0: "Sense Reversal", 1: "Pthread", 2: "Central", 3: "Tree"}
    
    loops = sorted(list(set(r['loops'] for r in results)))
    threads = sorted(list(set(r['threads'] for r in results)))
    pols = sorted(list(set(r['policy'] for r in results)))
    
    fig, axes = plt.subplots(1, len(loops), figsize=(6*len(loops), 5))
    if len(loops) == 1: axes = [axes]
    
    for ax, loop_count in zip(axes, loops):
        for idx, p in enumerate(pols):
            x = []
            y = []
            for r in results:
                if r['loops'] == loop_count and r['policy'] == p:
                    x.append(r['threads'])
                    y.append(r['avg_time'])
            
            pairs = sorted(zip(x, y))
            if pairs:
                x, y = zip(*pairs)
                ax.plot(x, y, marker='o', label=policy_map.get(p, str(p)))
        
        ax.set_yscale('log')
        ax.set_xlabel("Threads")
        ax.set_xticks(threads)
        ax.set_title(f"Barrier Overhead ({loop_count} Loops)")
        if ax == axes[0]: 
            ax.set_ylabel("Time (s) - Log Scale")
            ax.legend()
            
    save_plot(f"{DATA_FILE.stem}_log_time.png", fig)

else:
    print("Error: Unknown JSON format")
