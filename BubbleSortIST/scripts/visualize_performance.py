import os
import re
import matplotlib.pyplot as plt
import numpy as np

# Debug: Verify np is the NumPy module
print(f"np type: {type(np)}, np.arange: {hasattr(np, 'arange')}")

# Use absolute path based on project root
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), "../.."))
log_dir = os.path.join(project_root, "build")
output_dir = os.path.join(project_root, "data", "output", "plots")
os.makedirs(output_dir, exist_ok=True)

print(f"Project root: {project_root}")
print(f"Log directory: {log_dir}")
print(f"Output directory: {output_dir}")

# Configurations
n_values = [4, 5, 6, 7]
np_values = [1, 2, 4]
threads_values = [1, 2, 4]

# Data storage
total_times = {}
section_times = {}
gprof_times = {}

# Parse log files for total and section times
for n in n_values:
    total_times[n] = {}
    section_times[n] = {}
    for np_val in np_values:
        total_times[n][np_val] = {}
        section_times[n][np_val] = {}
        for threads in threads_values:
            log_file = os.path.join(log_dir, f"run_n{n}_np{np_val}_threads{threads}.log")
            print(f"Checking log file: {log_file}")
            if not os.path.exists(log_file):
                print(f"Warning: Log file not found: {log_file}")
                continue

            # Initialize section times
            section_times[n][np_val][threads] = {
                "Graph": 0.0, "Partition": 0.0, "LocalVertices": 0.0,
                "IST": 0.0, "Gather": 0.0, "Output": 0.0, "Total": 0.0
            }

            with open(log_file, "r") as f:
                log_content = f.read()

                # Extract total time from /usr/bin/time
                time_match = re.search(r"Elapsed \(wall clock\) time \(h:mm:ss or m:ss\): (\d+):(\d+\.\d+)", log_content)
                if time_match:
                    minutes, seconds = float(time_match.group(1)), float(time_match.group(2))
                    total_times[n][np_val][threads] = minutes * 60 + seconds
                else:
                    total_times[n][np_val][threads] = 0.0
                    print(f"Warning: No total time found in {log_file}")

                # Extract section times from MPI_Wtime logs
                timing_match = re.search(
                    r"Timing: Graph=([\d.]+)s, Partition=([\d.]+)s, LocalVertices=([\d.]+)s, "
                    r"IST=([\d.]+)s, Gather=([\d.]+)s, Output=([\d.]+)s, Total=([\d.]+)s",
                    log_content
                )
                if timing_match:
                    section_times[n][np_val][threads]["Graph"] = float(timing_match.group(1))
                    section_times[n][np_val][threads]["Partition"] = float(timing_match.group(2))
                    section_times[n][np_val][threads]["LocalVertices"] = float(timing_match.group(3))
                    section_times[n][np_val][threads]["IST"] = float(timing_match.group(4))
                    section_times[n][np_val][threads]["Gather"] = float(timing_match.group(5))
                    section_times[n][np_val][threads]["Output"] = float(timing_match.group(6))
                    section_times[n][np_val][threads]["Total"] = float(timing_match.group(7))
                else:
                    print(f"Warning: No section times found in {log_file}")

# Parse gprof files for function-level times
for n in n_values:
    gprof_times[n] = {}
    for np_val in np_values:
        gprof_times[n][np_val] = {}
        for threads in threads_values:
            gprof_file = os.path.join(log_dir, f"gprof_n{n}_np{np_val}_threads{threads}.txt")
            print(f"Checking gprof file: {gprof_file}")
            if not os.path.exists(gprof_file):
                print(f"Warning: gprof file not found: {gprof_file}")
                continue

            gprof_times[n][np_val][threads] = {}
            with open(gprof_file, "r") as f:
                lines = f.readlines()
                parsing_flat = False
                for line in lines:
                    if "Flat profile:" in line:
                        parsing_flat = True
                        continue
                    if parsing_flat and line.strip() and not line.startswith("-"):
                        parts = line.split()
                        if len(parts) >= 4 and parts[0].replace(".", "").isdigit():
                            func_name = parts[-1]
                            # Handle fractional time (e.g., '96/5136')
                            self_time_str = parts[2]
                            try:
                                if '/' in self_time_str:
                                    num, denom = map(float, self_time_str.split('/'))
                                    self_time = num / denom
                                else:
                                    self_time = float(self_time_str)
                                gprof_times[n][np_val][threads][func_name] = self_time
                            except ValueError as e:
                                print(f"Error parsing time for {func_name}: {self_time_str} - {e}")
                                gprof_times[n][np_val][threads][func_name] = 0.0

# Plot 1: Total Execution Time
for n in n_values:
    plt.figure(figsize=(10, 6))
    labels = []
    times = []
    for np_val in np_values:
        for threads in threads_values:
            if np_val in total_times[n] and threads in total_times[n][np_val]:
                labels.append(f"np={np_val}, t={threads}")
                times.append(total_times[n][np_val][threads])
    plt.bar(labels, times)
    plt.xlabel("Configuration (MPI Processes, OpenMP Threads)")
    plt.ylabel("Total Execution Time (seconds)")
    plt.title(f"Total Execution Time for n={n}")
    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, f"total_time_n{n}.png"))
    plt.close()

# Plot 2: Section Timing (Stacked Bar)
for n in n_values:
    plt.figure(figsize=(12, 6))
    labels = []
    graph_times = []
    partition_times = []
    local_vertices_times = []
    ist_times = []
    gather_times = []
    output_times = []
    for np_val in np_values:
        for threads in threads_values:
            if np_val in section_times[n] and threads in section_times[n][np_val]:
                labels.append(f"np={np_val}, t={threads}")
                graph_times.append(section_times[n][np_val][threads]["Graph"])
                partition_times.append(section_times[n][np_val][threads]["Partition"])
                local_vertices_times.append(section_times[n][np_val][threads]["LocalVertices"])
                ist_times.append(section_times[n][np_val][threads]["IST"])
                gather_times.append(section_times[n][np_val][threads]["Gather"])
                output_times.append(section_times[n][np_val][threads]["Output"])

    x = np.arange(len(labels))
    plt.bar(x, graph_times, label="Graph")
    plt.bar(x, partition_times, bottom=graph_times, label="Partition")
    plt.bar(x, local_vertices_times, bottom=np.array(graph_times) + np.array(partition_times), label="LocalVertices")
    plt.bar(x, ist_times, bottom=np.array(graph_times) + np.array(partition_times) + np.array(local_vertices_times), label="IST")
    plt.bar(x, gather_times, bottom=np.array(graph_times) + np.array(partition_times) + np.array(local_vertices_times) + np.array(ist_times), label="Gather")
    plt.bar(x, output_times, bottom=np.array(graph_times) + np.array(partition_times) + np.array(local_vertices_times) + np.array(ist_times) + np.array(gather_times), label="Output")
    plt.xlabel("Configuration (MPI Processes, OpenMP Threads)")
    plt.ylabel("Time (seconds)")
    plt.title(f"Section Timing Breakdown for n={n}")
    plt.xticks(x, labels, rotation=45)
    plt.legend()
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, f"section_timing_n{n}.png"))
    plt.close()

# Plot 3: Speedup for n=4, threads=1
if 4 in total_times:
    plt.figure(figsize=(8, 6))
    baseline_time = total_times[4][1][1]  # np=1, threads=1
    np_vals = []
    speedups = []
    for np_val in np_values:
        if np_val in total_times[4] and 1 in total_times[4][np_val]:
            np_vals.append(np_val)
            speedups.append(baseline_time / total_times[4][np_val][1])
    plt.plot(np_vals, speedups, marker="o", label="Speedup")
    plt.plot(np_vals, np_vals, linestyle="--", label="Ideal")
    plt.xlabel("Number of MPI Processes")
    plt.ylabel("Speedup")
    plt.title("Strong Scaling Speedup (n=4, threads=1)")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, f"speedup_n4_threads1.png"))
    plt.close()

# Plot 4: Function-Level Time (Top 5 Functions)
for n in n_values:
    plt.figure(figsize=(12, 6))
    labels = []
    top_functions = set()
    # Collect top 5 functions by time across all configs
    for np_val in np_values:
        for threads in threads_values:
            if np_val in gprof_times[n] and threads in gprof_times[n][np_val]:
                sorted_funcs = sorted(gprof_times[n][np_val][threads].items(), key=lambda x: x[1], reverse=True)[:5]
                top_functions.update(func for func, _ in sorted_funcs)
    top_functions = list(top_functions)[:5]  # Limit to 5

    times_by_func = {func: [] for func in top_functions}
    for np_val in np_values:
        for threads in threads_values:
            if np_val in gprof_times[n] and threads in gprof_times[n][np_val]:
                labels.append(f"np={np_val}, t={threads}")
                for func in top_functions:
                    times_by_func[func].append(gprof_times[n][np_val][threads].get(func, 0.0))

    x = np.arange(len(labels))
    bottom = np.zeros(len(labels))
    for func in top_functions:
        plt.bar(x, times_by_func[func], bottom=bottom, label=func)
        bottom += np.array(times_by_func[func])

    plt.xlabel("Configuration (MPI Processes, OpenMP Threads)")
    plt.ylabel("Self Time (seconds)")
    plt.title(f"Function-Level Time (Top 5 Functions) for n={n}")
    plt.xticks(x, labels, rotation=45)
    plt.legend()
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, f"function_time_n{n}.png"))
    plt.close()

# Plot 5: Parallel Time (Total - Graph)
for n in n_values:
    plt.figure(figsize=(10, 6))
    labels = []
    parallel_times = []
    for np_val in np_values:
        for threads in threads_values:
            if np_val in section_times[n] and threads in section_times[n][np_val]:
                labels.append(f"np={np_val}, t={threads}")
                parallel_time = section_times[n][np_val][threads]["Total"] - section_times[n][np_val][threads]["Graph"]
                parallel_times.append(parallel_time)
    plt.bar(labels, parallel_times)
    plt.xlabel("Configuration (MPI Processes, OpenMP Threads)")
    plt.ylabel("Parallel Execution Time (seconds)")
    plt.title(f"Parallel Time (Total - Graph) for n={n}")
    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, f"parallel_time_n{n}.png"))
    plt.close()

print(f"Plots saved in {output_dir}")