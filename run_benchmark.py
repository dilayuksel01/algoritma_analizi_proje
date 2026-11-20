import os
import re
import shutil
import subprocess
import tempfile
import matplotlib.pyplot as plt
import numpy as np

# Set writable MPLCONFIGDIR for matplotlib on Windows
os.environ["MPLCONFIGDIR"] = tempfile.gettempdir()

# Paths
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
SAMPLE_SET_DIR = os.path.join(BASE_DIR, "data", "sample_set")
MAIN_DOC = os.path.join(SAMPLE_SET_DIR, "main_doc.txt")
DOCS_DIR = os.path.join(SAMPLE_SET_DIR, "documents")
EXE_PATH = os.path.join(BASE_DIR, "plagiarism.exe")
REPORT_DIR = os.path.join(BASE_DIR, "docs", "report")
FIGURES_DIR = os.path.join(REPORT_DIR, "figures")

os.makedirs(FIGURES_DIR, exist_ok=True)

# Document sizes to benchmark (5 sizes)
DOC_COUNTS = [4, 8, 12, 16, 20]
NUM_RUNS = 7  # 7 runs per size, drop min & max, average the middle 5

all_doc_files = sorted([f for f in os.listdir(DOCS_DIR) if f.endswith(".txt")])

results = []

print("=" * 60)
print("CMP3005 Plagiarism Detector - Empirical Benchmark")
print("=" * 60)
print(f"Testing document sizes: {DOC_COUNTS}")
print(f"Number of runs per size: {NUM_RUNS} (trimmed mean)")
print("-" * 60)

for count in DOC_COUNTS:
    selected_docs = all_doc_files[:count]
    
    # Create temp execution directory
    with tempfile.TemporaryDirectory() as temp_dir:
        # Copy main_doc.txt
        shutil.copy(MAIN_DOC, os.path.join(temp_dir, "main_doc.txt"))
        
        # Copy selected documents
        temp_docs_dir = os.path.join(temp_dir, "documents")
        os.makedirs(temp_docs_dir, exist_ok=True)
        for doc in selected_docs:
            shutil.copy(os.path.join(DOCS_DIR, doc), os.path.join(temp_docs_dir, doc))
        
        brute_times = []
        fast_times = []
        
        for run_idx in range(NUM_RUNS):
            proc = subprocess.run(
                [EXE_PATH, "--benchmark"],
                cwd=temp_dir,
                capture_output=True,
                text=True
            )
            
            output = proc.stderr
            brute_match = re.search(r"Brute Force time\s*:\s*([\d\.]+)\s*ms", output)
            fast_match = re.search(r"Efficient time\s*:\s*([\d\.]+)\s*ms", output)
            
            if brute_match and fast_match:
                brute_times.append(float(brute_match.group(1)))
                fast_times.append(float(fast_match.group(1)))
            else:
                print(f"Warning: could not parse output on run {run_idx}: {output}")
        
        # Trimmed mean: drop min and max, average remainder
        if len(brute_times) >= 3:
            brute_times.sort()
            fast_times.sort()
            avg_brute = sum(brute_times[1:-1]) / (len(brute_times) - 2)
            avg_fast = sum(fast_times[1:-1]) / (len(fast_times) - 2)
        else:
            avg_brute = sum(brute_times) / len(brute_times)
            avg_fast = sum(fast_times) / len(fast_times)
        
        speedup = avg_brute / avg_fast if avg_fast > 0 else 0
        
        results.append({
            "docs": count,
            "brute_ms": avg_brute,
            "fast_ms": avg_fast,
            "speedup": speedup
        })
        
        print(f"Docs: {count:2d} | Brute Force: {avg_brute:6.2f} ms | Efficient: {avg_fast:5.2f} ms | Speedup: {speedup:4.2f}x")

print("=" * 60)

# 1. Save Markdown table
md_path = os.path.join(REPORT_DIR, "benchmark_table.md")
with open(md_path, "w", encoding="utf-8") as f:
    f.write("# Empirical Analysis - Execution Time Comparison\n\n")
    f.write("| Document Count ($D$) | Brute Force Time (ms) | Efficient Time (ms) | Speedup Factor |\n")
    f.write("|:-------------------:|:---------------------:|:-------------------:|:--------------:|\n")
    for r in results:
        f.write(f"| {r['docs']} | {r['brute_ms']:.2f} ms | {r['fast_ms']:.2f} ms | **{r['speedup']:.2f}x** |\n")

# 2. Save CSV
csv_path = os.path.join(REPORT_DIR, "benchmark_data.csv")
with open(csv_path, "w", encoding="utf-8") as f:
    f.write("documents,brute_force_ms,efficient_ms,speedup\n")
    for r in results:
        f.write(f"{r['docs']},{r['brute_ms']:.3f},{r['fast_ms']:.3f},{r['speedup']:.3f}\n")

print(f"Saved table to: {md_path}")
print(f"Saved CSV to: {csv_path}")

# 3. Generate High-Quality Publication Plot
docs = [r["docs"] for r in results]
brute = [r["brute_ms"] for r in results]
fast = [r["fast_ms"] for r in results]

plt.figure(figsize=(9, 5.5), dpi=300)

plt.plot(docs, brute, marker='o', linewidth=2.2, color='#D32F2F', label='Brute Force (Exhaustive O(Sm · Sd · L))')
plt.plot(docs, fast, marker='s', linewidth=2.2, color='#1976D2', label='Efficient (Hashing & Inverted Index)')

plt.title('Empirical Analysis: Brute Force vs. Efficient Hashing Algorithm', fontsize=13, fontweight='bold', pad=14)
plt.xlabel('Number of Input Documents (D)', fontsize=11, fontweight='semibold')
plt.ylabel('Execution Time (milliseconds)', fontsize=11, fontweight='semibold')
plt.xticks(DOC_COUNTS, [f"{d} docs" for d in DOC_COUNTS], fontsize=10)
plt.yticks(fontsize=10)
plt.grid(True, linestyle='--', alpha=0.6)

# Annotate speedup on each point
for d, b, f_val, r in zip(docs, brute, fast, results):
    plt.annotate(
        f"{r['speedup']:.1f}x faster",
        xy=(d, f_val),
        xytext=(0, -18),
        textcoords="offset points",
        ha='center',
        fontsize=9,
        fontweight='bold',
        color='#0D47A1'
    )

plt.legend(frameon=True, facecolor='white', framealpha=0.95, loc='upper left', fontsize=10.5)
plt.tight_layout()

plot_png = os.path.join(FIGURES_DIR, "benchmark_plot.png")
plot_pdf = os.path.join(FIGURES_DIR, "benchmark_plot.pdf")

plt.savefig(plot_png, dpi=300)
plt.savefig(plot_pdf)
plt.close()

print(f"Saved plot PNG to: {plot_png}")
print(f"Saved plot PDF to: {plot_pdf}")
print("Benchmark completed successfully!")
