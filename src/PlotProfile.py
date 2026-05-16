#!/usr/bin/env python3

import argparse
import csv
import os
import matplotlib.pyplot as plt
import matplotlib.ticker as mticker
import numpy as np

# ---------------------------------------------------------------------------
# I/O
# ---------------------------------------------------------------------------

def read_data(filepath: str):
    names, times, time_stds, ticks, ticks_stds, instrs = [], [], [], [], [], []
    with open(filepath, newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        for row in reader:
            names.append(row["name"])
            times.append(float(row["time_ms"]))
            time_stds.append(float(row.get("time_std", 0)))
            ticks.append(int(row["ticks"]))
            ticks_stds.append(int(row.get("ticks_std", 0)))
            instrs.append(int(row["instructions"]))
    return (
        names,
        np.array(times),
        np.array(time_stds),
        np.array(ticks),
        np.array(ticks_stds),
        np.array(instrs),
    )


def save_fig(fig, path):
    fig.savefig(path, dpi=180, bbox_inches="tight", pad_inches=0.3)
    plt.close(fig)


# ---------------------------------------------------------------------------
# Label helpers
# ---------------------------------------------------------------------------

def add_value_labels(ax, bars, values, fmt="{:.2f}", fontsize=8):
    """Print value above each bar."""
    for bar, val in zip(bars, values):
        ax.text(
            bar.get_x() + bar.get_width() / 2,
            bar.get_height(),
            fmt.format(val),
            ha="center", va="bottom", fontsize=fontsize, fontweight="bold",
        )


def add_delta_annotations(ax, bars, values, fontsize=7,
                           color_down="#27ae60", color_up="#c0392b"):
    """Print % change relative to previous bar in the middle of each bar."""
    for i, (bar, val) in enumerate(zip(bars, values)):
        if i == 0 or values[i - 1] == 0:
            continue
        delta = (val - values[i - 1]) / values[i - 1] * 100
        color = color_down if delta < 0 else color_up
        sign  = "▼" if delta < 0 else "▲"
        ax.text(
            bar.get_x() + bar.get_width() / 2,
            bar.get_height() * 0.5,
            f"{sign}{abs(delta):.1f}%",
            ha="center", va="center", fontsize=fontsize,
            color=color, fontweight="bold",
            bbox=dict(boxstyle="round,pad=0.15", facecolor="white",
                      edgecolor="none", alpha=0.78),
        )


# ---------------------------------------------------------------------------
# Individual plots
# ---------------------------------------------------------------------------

def plot_time(ax, x, names, times, time_stds):
    bars = ax.bar(x, times, yerr=time_stds, capsize=5,
                  color="#7FB3E0", edgecolor="black", linewidth=0.6,
                  error_kw=dict(elinewidth=1.2, ecolor="#2471a3", capthick=1.5))
    add_value_labels(ax, bars, times, fmt="{:.1f}")
    add_delta_annotations(ax, bars, times)
    ax.set_xticks(x)
    ax.set_xticklabels(names, rotation=30, ha="right")
    ax.set_ylabel("Время (мс)")
    ax.set_title("Время выполнения по реализациям")
    if time_stds.any():
        ax.annotate("Планки погрешностей: ±σ (стд. отклонение)",
                    xy=(0.01, 0.97), xycoords="axes fraction",
                    fontsize=7, color="#555", va="top")


def plot_ticks(ax, x, names, ticks, ticks_stds):
    ticks_b     = ticks / 1e9
    ticks_std_b = ticks_stds / 1e9
    bars = ax.bar(x, ticks_b, yerr=ticks_std_b, capsize=5,
                  color="#F0B27A", edgecolor="black", linewidth=0.6,
                  error_kw=dict(elinewidth=1.2, ecolor="#b7770d", capthick=1.5))
    add_value_labels(ax, bars, ticks_b, fmt="{:.2f}")
    add_delta_annotations(ax, bars, ticks_b)
    ax.set_xticks(x)
    ax.set_xticklabels(names, rotation=30, ha="right")
    ax.set_ylabel(r"Такты ($\times 10^9$)")
    ax.set_title("Процессорные такты по реализациям")
    if ticks_stds.any():
        ax.annotate("Планки погрешностей: ±σ (стд. отклонение)",
                    xy=(0.01, 0.97), xycoords="axes fraction",
                    fontsize=7, color="#555", va="top")


def plot_instructions(ax, x, names, instrs):
    """Instructions come from valgrind Ir — deterministic, no error bars."""
    instrs_b = instrs / 1e9
    bars = ax.bar(x, instrs_b,
                  color="#82E0AA", edgecolor="black", linewidth=0.6)
    add_value_labels(ax, bars, instrs_b, fmt="{:.2f}")
    add_delta_annotations(ax, bars, instrs_b)
    ax.set_xticks(x)
    ax.set_xticklabels(names, rotation=30, ha="right")
    ax.set_ylabel(r"Инструкции ($\times 10^9$)")
    ax.set_title("Количество инструкций (Ir) по реализациям")
    ax.annotate("Детерминированный счётчик valgrind — погрешность не применима",
                xy=(0.01, 0.97), xycoords="axes fraction",
                fontsize=7, color="#555", va="top")


def plot_summary(ax, x, names, times, ticks, instrs):
    width = 0.25
    t_norm = times  / times.max()
    k_norm = ticks  / ticks.max()
    i_norm = instrs / instrs.max()

    bars_t = ax.bar(x - width, t_norm, width, label="Время",
                    color="#7FB3E0", edgecolor="black", linewidth=0.5)
    bars_k = ax.bar(x,          k_norm, width, label="Такты",
                    color="#F0B27A", edgecolor="black", linewidth=0.5)
    bars_i = ax.bar(x + width,  i_norm, width, label="Инструкции",
                    color="#82E0AA", edgecolor="black", linewidth=0.5)

    for bars, vals in [(bars_t, t_norm), (bars_k, k_norm), (bars_i, i_norm)]:
        add_value_labels(ax, bars, vals, fmt="{:.2f}", fontsize=7)
        add_delta_annotations(ax, bars, vals, fontsize=6)

    ax.set_xticks(x)
    ax.set_xticklabels(names, rotation=30, ha="right")
    ax.set_ylabel("Относительное значение (макс = 1)")
    ax.set_title("Сравнение реализаций (нормировано)")
    ax.legend()


# ---------------------------------------------------------------------------
# Console report
# ---------------------------------------------------------------------------

def print_report(names, times, time_stds, ticks, ticks_stds, instrs):
    baseline_idx = next(
        (i for i, n in enumerate(names) if "-O3" in n or n == "-O3"), 1
    )

    show_idxs = [i for i, n in enumerate(names)
                 if i != baseline_idx and "-O0" not in n and n != "-O0"]

    print(f"\nВыигрыш относительно baseline ({names[baseline_idx]}):")
    print(f"  {'Реализация':<35} {'Время':>18} {'Такты':>18} {'Инструкции':>18}")
    print(f"  {'-' * 89}")

    for i in show_idxs:
        gain_t = (times[baseline_idx]  - times[i])  / times[baseline_idx]  * 100
        gain_k = (ticks[baseline_idx]  - ticks[i])  / ticks[baseline_idx]  * 100
        gain_i = (instrs[baseline_idx] - instrs[i]) / instrs[baseline_idx] * 100
        ratio_t = times[baseline_idx]  / times[i]
        ratio_k = ticks[baseline_idx]  / ticks[i]
        ratio_i = instrs[baseline_idx] / instrs[i]
        print(f"  {names[i]:<35} {gain_t:>+7.2f}% ({ratio_t:.2f}x)"
              f" {gain_k:>+7.2f}% ({ratio_k:.2f}x)"
              f" {gain_i:>+7.2f}% ({ratio_i:.2f}x)")

    label_w, col_w = 52, 24
    all_idxs = [baseline_idx] + show_idxs

    print("\nУскорение относительно предыдущей версии:")
    header = (f"  {'Переход':<{label_w}} "
              f"{'Время':>{col_w}} {'Такты':>{col_w}} {'Инструкции':>{col_w}}")
    print(header)
    print("  " + "-" * (label_w + 3 * (col_w + 1)))

    for prev, curr in zip(all_idxs[:-1], all_idxs[1:]):
        gain_t  = (times[prev]  - times[curr])  / times[prev]  * 100
        gain_k  = (ticks[prev]  - ticks[curr])  / ticks[prev]  * 100
        gain_i  = (instrs[prev] - instrs[curr]) / instrs[prev] * 100
        ratio_t = times[prev]  / times[curr]
        ratio_k = ticks[prev]  / ticks[curr]
        ratio_i = instrs[prev] / instrs[curr]

        label = f"{names[prev]}  ->  {names[curr]}"
        if len(label) > label_w:
            label = label[:label_w - 3] + "..."

        col_t = f"{gain_t:+7.2f}% ({ratio_t:5.2f}x)"
        col_k = f"{gain_k:+7.2f}% ({ratio_k:5.2f}x)"
        col_i = f"{gain_i:+7.2f}% ({ratio_i:5.2f}x)"
        print(f"  {label:<{label_w}} {col_t:>{col_w}} {col_k:>{col_w}} {col_i:>{col_w}}")

    # σ table
    if time_stds.any():
        print(f"\n  {'Реализация':<35} {'Время ср., мс':>15} {'±σ мс':>10}"
              f" {'Такты ср.':>15} {'±σ тактов':>12}")
        print(f"  {'-' * 87}")
        for i, name in enumerate(names):
            print(f"  {name:<35} {times[i]:>15.2f} {time_stds[i]:>10.2f}"
                  f" {ticks[i]:>15,.0f} {ticks_stds[i]:>12,.0f}")


# ---------------------------------------------------------------------------
# Main orchestrator
# ---------------------------------------------------------------------------

def plot_all(names, times, time_stds, ticks, ticks_stds, instrs, outdir):
    os.makedirs(outdir, exist_ok=True)
    x = np.arange(len(names))
    W = max(8, len(names) * 1.6)

    plt.rcParams.update({
        "font.size": 12,
        "axes.titlesize": 14,
        "axes.labelsize": 12,
        "figure.autolayout": False,
    })
    plt.style.use("seaborn-v0_8-whitegrid")

    # --- time ---
    fig, ax = plt.subplots(figsize=(W, 5.5))
    plot_time(ax, x, names, times, time_stds)
    save_fig(fig, os.path.join(outdir, "time_ms.png"))

    # --- ticks ---
    fig, ax = plt.subplots(figsize=(W, 5.5))
    plot_ticks(ax, x, names, ticks, ticks_stds)
    save_fig(fig, os.path.join(outdir, "ticks.png"))

    # --- instructions ---
    fig, ax = plt.subplots(figsize=(W, 5.5))
    plot_instructions(ax, x, names, instrs)
    save_fig(fig, os.path.join(outdir, "instructions.png"))

    # --- summary ---
    fig, ax = plt.subplots(figsize=(W + 1, 5.5))
    plot_summary(ax, x, names, times, ticks, instrs)
    save_fig(fig, os.path.join(outdir, "summary.png"))

    print(f"\nГрафики сохранены в {outdir}/")
    print("  time_ms.png, ticks.png, instructions.png, summary.png")

    print_report(names, times, time_stds, ticks, ticks_stds, instrs)


def main():
    parser = argparse.ArgumentParser(description="Визуализация профиля хеш-таблицы")
    parser.add_argument("csv_file", help="Путь к CSV-файлу с данными")
    parser.add_argument("-o", "--outdir", default="plots",
                        help="Папка для графиков (по умолчанию: plots)")
    args = parser.parse_args()

    names, times, time_stds, ticks, ticks_stds, instrs = read_data(args.csv_file)
    plot_all(names, times, time_stds, ticks, ticks_stds, instrs, args.outdir)


if __name__ == "__main__":
    main()