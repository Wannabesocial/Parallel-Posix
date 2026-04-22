#!/bin/bash
set -e

cd "$(dirname "$0")/.."
OUT="benchmarks/transactions_results.json"
THREADS=8
TRIALS=10

CONFIGS=(
    "10000 75000"
    "100000 750000"
    "1000000 7500000"
)
QUERY_PERCENTAGES=(0 25 50 75 100)

extract() { echo "$2" | grep "$1" | awk -F': ' '{print $2}' | awk '{print $1}' | sed 's/^\./.0/'; }

collect_policy_data() {
    local policy=$1
    local policy_name=$2
    local results=""

    echo "--- Testing Policy $policy ($policy_name) ---" >&2

    for config in "${CONFIGS[@]}"; do
        acc=$(echo "$config" | awk '{print $1}')
        trans=$(echo "$config" | awk '{print $2}')

        for qpct in "${QUERY_PERCENTAGES[@]}"; do
            echo "  Running accounts=$acc transfers=$trans queries=$qpct%..." >&2
            times=""

            for trial in $(seq 1 $TRIALS); do
                out=$(LC_ALL=C ./bin/transactions "$acc" "$trans" "$qpct" "$policy" "$THREADS" 2>&1)

                if echo "$out" | grep -q "Success"; then
                    time_val=$(extract "Execution time" "$out")
                    sep=$([ -n "$times" ] && echo "," || echo "")
                    times+="$sep$time_val"
                else
                    echo "    Trial $trial failed!" >&2
                    sep=$([ -n "$times" ] && echo "," || echo "")
                    times+="${sep}0"
                fi
            done

            sep=$([ -n "$results" ] && echo "," || echo "")
            results+="$sep{\"accounts\":$acc,\"transfers\":$trans,\"query_pct\":$qpct,\"times\":[$times]}"
        done
    done

    echo "$results"
}

echo "Building transactions..." >&2
make clean && make transactions

echo "{" > "$OUT"
echo "  \"thread_count\": $THREADS," >> "$OUT"
echo "  \"trials\": $TRIALS," >> "$OUT"

policies=(1 2 3 4)
policy_names=("coarse_mutex" "fine_mutex" "coarse_rwlock" "fine_rwlock")

for i in "${!policies[@]}"; do
    policy="${policies[$i]}"
    policy_name="${policy_names[$i]}"

    echo "Running policy $policy: $policy_name" >&2
    data=$(collect_policy_data "$policy" "$policy_name")

    if [ $i -eq 0 ]; then
        echo "  \"$policy_name\": [$data]" >> "$OUT"
    else
        echo "  ,\"$policy_name\": [$data]" >> "$OUT"
    fi
done

echo "}" >> "$OUT"

echo "" >&2
echo "Benchmark complete! Results saved to $OUT" >&2
