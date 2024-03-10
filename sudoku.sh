#!/bin/bash

for option in 1 2 3; do
    data_file="option_${option}_runtimes.dat"
    graph_file="option_${option}_graph.png"

    total_time=0
    iterations=50

    echo "Option $option Runtimes:"
    for i in $(seq 1 $iterations); do
        start_time=$(date +%s.%N)
        ./main $option
        end_time=$(date +%s.%N)
        duration=$(echo "$end_time - $start_time" | bc)
        total_time=$(echo "$total_time + $duration" | bc)

        # Print '*' for each second of runtime
        seconds=$(echo "$duration/1" | bc)
        printf "%3s | %s\n" "$i" "$(yes '*' | head -n $seconds | tr -d '\n')"
        echo "$i $duration" >> "$data_file"
    done

    avg_time=$(echo "$total_time/$iterations" | bc -l)
    echo -e "\nAverage execution time for option $option: $avg_time seconds\n"
done

# Plot graphs after the 3rd option using Python and matplotlib
if command -v python3 &> /dev/null; then
    python3 - <<EOF
import matplotlib.pyplot as plt

for option in [1, 2, 3]:
    data_file = f"option_{option}_runtimes.dat"
    graph_file = f"option_{option}_graph.png"

    with open(data_file, 'r') as f:
        data = [line.split() for line in f.readlines()]

    iterations, runtimes = zip(*data)
    plt.plot(iterations, runtimes, label=f'Option {option} Runtimes')

plt.title('Runtimes for Options 1, 2, and 3')
plt.xlabel('Iteration')
plt.ylabel('Runtime (seconds)')
plt.legend()
plt.savefig('combined_graph.png')
plt.show()
EOF
fi

