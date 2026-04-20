import os
import pandas as pd
import matplotlib.pyplot as plt
import re

os.makedirs("images", exist_ok = True)

GROUPS = {
    "strings": [
        ("csv/str_length.csv",     "Длина строки"),
        ("csv/str_symbols.csv",    "Сумма символов"),
        ("csv/str_polynomial.csv", "Полиномиальный"),
        ("csv/str_rol+xor.csv",    "Rol + xor"),
        ("csv/str_ror+xor.csv",    "Ror + xor"),
        ("csv/str_crc32.csv",      "crc32"),
    ],
}

for group_name, files in GROUPS.items():
    n_plots = len(files)
    fig, axes = plt.subplots(1, n_plots, figsize = (6 * n_plots, 4))
    if n_plots == 1:
        axes = [axes]

    variances = []

    for ax, (fname, title) in zip(axes, files):
        if not os.path.exists(fname):
            print(f"Файл не найден: {fname}")
            continue

        df = pd.read_csv(fname)
        counts = df["count"].values

        var = counts.var()
        variances.append((title, var))

        colors = ["#DD8452" if c > counts.mean() else "#4C72B0" for c in counts]
        ax.bar(df["bucket"], counts, color = colors, alpha = 0.85, width = 0.8)
        ax.set_title(f"{title}\nДисперсия: {var:.1f}", fontsize = 10, fontweight = "bold")
        ax.set_xlabel("Бакет")
        ax.set_ylabel("Количество ключей")
        ax.grid(axis = "y", linestyle = "--", alpha = 0.4)

    fig.suptitle(group_name, fontsize = 13, fontweight = "bold")
    fig.tight_layout()

    out = f"images/collisions_{group_name.replace(' ', '_')}.png"
    fig.savefig(out, dpi = 150)
    plt.close(fig)
    print(f"Saved: {out}")

    print(f"\n  {'Функция':<30} {'Дисперсия':>12}")
    print(f"  {'-'*44}")
    for name, var in variances:
        print(f"  {name:<30} {var:>12.1f}")

    print(f"  Идеальная дисперсия (равномерное): 0.0\n")

timing_file = "csv/timing.csv"
if os.path.exists(timing_file):
    df_time = pd.read_csv(timing_file)

    group_map = {"str": "strings"}
    df_time["group"] = df_time["group"].map(group_map)

    subset = df_time[df_time["group"] == "strings"]

    import numpy as np
    x = np.arange(len(subset))
    width = 0.35

    fig, ax_ms = plt.subplots(figsize = (10, 5))
    ax_tk = ax_ms.twinx()

    bars_ms = ax_ms.bar(x - width/2, subset["time_ms"].values, width,
        color = "#4C72B0", alpha = 0.85, label = "Время (мс)")
    bars_tk = ax_tk.bar(x + width/2, subset["time_ticks"].values, width,
        color = "#DD8452", alpha = 0.85, label = "Тики (rdtsc)")

    ax_ms.bar_label(bars_ms, fmt = "%.2f", padding = 3, fontsize = 8)
    ax_tk.bar_label(bars_tk, fmt = "%.2e", padding = 3, fontsize = 8)

    ax_ms.set_xticks(x)
    ax_ms.set_xticklabels(subset["name"].values, rotation = 20)
    ax_ms.set_ylabel("Время (мс)")
    ax_tk.set_ylabel("Тики (rdtsc)")
    ax_ms.grid(axis = "y", linestyle = "--", alpha = 0.4)

    lines_ms, labels_ms = ax_ms.get_legend_handles_labels()
    lines_tk, labels_tk = ax_tk.get_legend_handles_labels()
    ax_ms.legend(lines_ms + lines_tk, labels_ms + labels_tk, loc = "upper left")

    fig.suptitle("Время работы хеш-функций (strings)", fontsize = 13, fontweight = "bold")
    fig.tight_layout()
    fig.savefig("images/timing.png", dpi = 150)
    plt.close(fig)
    print("Saved: images/timing.png")