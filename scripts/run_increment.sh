#!/bin/bash
set -e

cd "$(dirname "$0")/.."
OUT="benchmarks/increment.json"
ITERATIONS=(1000 10000 100000 1000000 10000000)
THREAD_COUNTS=(2 4 8)
TRIALS=10

extract_time() { echo "$2" | grep "$1" | awk -F'Time: ' '{print $2}' | awk '{print $1}' | sed 's/^\./0./'; }

collect_data() {
    local iterations=$1
    local threads=$2
    local res_mutex=""
    local res_rwlock=""
    local res_atomic=""

    echo "Running iterations=$iterations threads=$threads..." >&2
    t_mutex=""
    t_rwlock=""
    t_atomic=""

    for i in $(seq 1 $TRIALS); do
        out=$(LC_ALL=C bin/increment "$iterations" "$threads")

        v_mutex=$(extract_time "Mutex" "$out")
        v_rwlock=$(extract_time "RWLock" "$out")
        v_atomic=$(extract_time "Atomic" "$out")

        sep=$([ -n "$t_mutex" ] && echo "," || echo "")
        t_mutex+="$sep$v_mutex"
        t_rwlock+="$sep$v_rwlock"
        t_atomic+="$sep$v_atomic"
    done

    echo "[$t_mutex]|[$t_rwlock]|[$t_atomic]"
}

echo "Building increment..."
make clean && make increment

sizes_json=""
mutex_json=""
rwlock_json=""
atomic_json=""

for iters in "${ITERATIONS[@]}"; do
    for threads in "${THREAD_COUNTS[@]}"; do
        IFS='|' read -r j_mutex j_rwlock j_atomic <<< "$(collect_data "$iters" "$threads")"

        sep=$([ -n "$sizes_json" ] && echo "," || echo "")
        sizes_json+="${sep}\"${iters}_${threads}t\""

        sep=$([ -n "$mutex_json" ] && echo "," || echo "")
        mutex_json+="$sep$j_mutex"
        rwlock_json+="$sep$j_rwlock"
        atomic_json+="$sep$j_atomic"
    done
done

echo "{\"size\":[${sizes_json}], \"mutex_time\":[${mutex_json}], \"rwlock_time\":[${rwlock_json}], \"atomic_time\":[${atomic_json}]}" > "$OUT"
echo "Saved to $OUT"
