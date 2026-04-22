#!/bin/bash
set -e

EXE="./bin/polynomial"
OUT="benchmarks/polynomial.json"

SIZES=(1000 10000 100000)
THREADS=(2 4 8 16)

echo "{" > "$OUT"
echo '  "results": [' >> "$OUT"

first=true

extract_time() {
    echo "$1" | awk '/sec/ { print $NF }' | sed 's/sec//'
}

extract_equal() {
    if echo "$1" | grep -q "Equal"; then
        echo "yes"
    else
        echo "no"
    fi
}

for SZ in "${SIZES[@]}"; do

    echo "Running size=$SZ..." >&2

    base_output=$($EXE "$SZ" 1)

    create_time=$(echo "$base_output" | awk '/Create polynomials/ { print $3 }')

    for TH in "${THREADS[@]}"; do
        echo "  Threads=$TH..." >&2

        output=$($EXE "$SZ" "$TH")

        serial_time=$(echo "$output" | awk '/Serial product/ { print $3 }')
        parallel_time=$(echo "$output" | awk '/Parallel product/ { print $3 }')
        equal=$(extract_equal "$output")

        if [ "$first" = true ]; then
            first=false
        else
            echo "," >> "$OUT"
        fi

        echo "    { \"size\": $SZ, \"threads\": $TH, \"create_time\": $create_time, \"serial_time\": $serial_time, \"parallel_time\": $parallel_time, \"equal\": \"$equal\" }" >> "$OUT"
    done

done

echo "  ]" >> "$OUT"
echo "}" >> "$OUT"

echo "Saved to $OUT"