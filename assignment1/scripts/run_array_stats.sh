#!/bin/bash
set -e

cd "$(dirname "$0")/.."
OUT="benchmarks/array_stats.json"
SIZES=(1000 10000 100000 1000000 10000000)
TRIALS=10

extract() { echo "$2" | grep "$1" | awk -F': ' '{print $2}' | awk '{print $1}' | sed 's/^\./0./'; }

collect_data() {
    local mode=$1; local res_init=""; local res_ser=""; local res_par=""
    
    for n in "${SIZES[@]}"; do
        echo "Running $mode N=$n..." >&2
        t_init=""; t_ser=""; t_par=""
        
        for i in $(seq 1 $TRIALS); do
            out=$(LC_ALL=C bin/array_stats "$n")
            
            v_i=$(extract "Initialization" "$out")
            v_p=$(extract "Parallel" "$out")
            v_s=$(extract "Serial" "$out")

            sep=$([ -n "$t_init" ] && echo "," || echo "")
            t_init+="$sep$v_i"; t_par+="$sep$v_p"; t_ser+="$sep$v_s"
        done
        
        sep=$([ -n "$res_init" ] && echo "," || echo "")
        res_init+="$sep[$t_init]"; res_par+="$sep[$t_par]"; res_ser+="$sep[$t_ser]"
    done
    
    echo "$res_init|$res_par|$res_ser"
}

echo "--- Phase 1: Original ---"
make clean && make
IFS='|' read -r j_init j_par_orig j_ser <<< "$(collect_data "Original")"

echo "--- Phase 2: Improved ---"
make clean && make array_stats EXTRA_CFLAGS=-DIMPROVED
IFS='|' read -r _ j_par_imp _ <<< "$(collect_data "Improved")"

echo "{\"size\":[$(echo ${SIZES[*]} | tr ' ' ',')], \"init_time\":[$j_init], \"serial_time\":[$j_ser], \"parallel_time_original\":[$j_par_orig], \"parallel_time_improved\":[$j_par_imp]}" > "$OUT"
echo "Saved to $OUT"
