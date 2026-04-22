#!/bin/bash
set -e

EXE="./bin/barrier"

OUT="benchmarks/barrier.json"

LOOPS=(1000 10000 100000 1000000)
THREADS=(2 4 8)
POLICIES=(0 1 2 3)

LOOPS_EXTRA=(1000)
THREADS_EXTRA=(2 4 8 16 32)

TRIALS=5

run_once() {
    $EXE "$1" "$2" "$3"
}

echo "{" > "$OUT"
echo '  "results": [' >> "$OUT"

first_entry=true

for L in "${LOOPS[@]}"; do
    for T in "${THREADS[@]}"; do
        for P in "${POLICIES[@]}"; do

            echo "Running loop=$L threads=$T policy=$P..." >&2

            sum=0

            for i in $(seq 1 $TRIALS); do
                value=$(run_once "$L" "$T" "$P")
                sum=$(awk -v a="$sum" -v b="$value" 'BEGIN { printf("%.6f", a+b) }')
            done

            avg=$(awk -v s="$sum" -v t="$TRIALS" 'BEGIN { printf("%.6f", s/t) }')

            if [ "$first_entry" = true ]; then
                first_entry=false
            else
                echo "," >> "$OUT"
            fi

            echo "    { \"loops\": $L, \"threads\": $T, \"policy\": $P, \"avg_time\": $avg }" >> "$OUT"

        done
    done
done


# --- Second batch (loops=1000, threads extended) ---
for L in "${LOOPS_EXTRA[@]}"; do
    for T in "${THREADS_EXTRA[@]}"; do
        for P in "${POLICIES[@]}"; do

            echo "Running EXTRA loop=$L threads=$T policy=$P..." >&2

            sum=0
            for i in $(seq 1 $TRIALS); do
                value=$(run_once "$L" "$T" "$P")
                sum=$(awk -v a="$sum" -v b="$value" 'BEGIN { printf("%.6f", a+b) }')
            done
            avg=$(awk -v s="$sum" -v t="$TRIALS" 'BEGIN { printf("%.6f", s/t) }')

            echo "," >> "$OUT"
            echo "    { \"loops\": $L, \"threads\": $T, \"policy\": $P, \"avg_time\": $avg }" >> "$OUT"
        done
    done
done

echo "  ]" >> "$OUT"
echo "}" >> "$OUT"

echo "Saved to $OUT"