#!/usr/bin/env python3

import argparse
import csv
import os
import matplotlib.pyplot as plt
import matplotlib.ticker as mticker
import numpy as np

def read_data(filepath: str):
    names, times, ticks, instrs = [], [], [], []
    with open(filepath, newline = "", encoding = "utf-8") as f:
        reader = csv.DictReader(f)
        for row in reader:
            names.append(row["name"])
            times.append(float(row["time_ms"]))
            ticks.append(int(row["ticks"]))
            instrs.append(int(row["instructions"]))
    return names, np.array(times), np.array(ticks), np.array(instrs)


def save_fig(fig, path):
    fig.savefig(path, dpi = 180, bbox_inches = "tight", pad_inches = 0.3)
    plt.close(fig)


def add_delta_labels(ax, values, fmt = "{:.1f}", delta_color_up = "#c0392b", delta_color_down = "#27ae60"):
    bars = ax.patches
    for i, (bar, val) in enumerate(zip(bars, values)):
        ax.text(
            bar.get_x() + bar.get_width() / 2,
            bar.get_height(),
            fmt.format(val),
            ha = "center", va = "bottom", fontsize = 8, fontweight = "bold",
        )

        if i > 0:
            prev = values[i - 1]
            if prev == 0:
                continue

            delta = (val - prev) / prev * 100
            color = delta_color_down if delta < 0 else delta_color_up
            sign = "▼" if delta < 0 else "▲"
            ax.text(
                bar.get_x() + bar.get_width() / 2,
                bar.get_height() * 0.5,
                f"{sign}{abs(delta):.1f}%",
                ha = "center", va = "center", fontsize = 7,
                color = color, fontweight = "bold",
                bbox = dict(boxstyle = "round,pad=0.15", facecolor = "white",
                    edgecolor = "none", alpha = 0.75),
            )


def add_delta_labels_grouped(ax, bars, values, delta_color_up = "#c0392b", delta_color_down = "#27ae60"):
    for i, (bar, val) in enumerate(zip(bars, values)):
        ax.text(
            bar.get_x() + bar.get_width() / 2,
            bar.get_height(),
            f"{val:.2f}",
            ha = "center", va = "bottom", fontsize = 7, fontweight = "bold",
        )
        if i > 0:
            prev = values[i - 1]
            if prev == 0:
                continue

            delta = (val - prev) / prev * 100
            color = delta_color_down if delta < 0 else delta_color_up
            sign = "▼" if delta < 0 else "▲"
            ax.text(
                bar.get_x() + bar.get_width() / 2,
                bar.get_height() * 0.5,
                f"{sign}{abs(delta):.1f}%",
                ha = "center", va = "center", fontsize = 6,
                color = color, fontweight = "bold",
                bbox = dict(boxstyle = "round,pad=0.1", facecolor = "white",
                    edgecolor = "none", alpha = 0.8),
            )


def plot_all(names, times, ticks, instrs, outdir: str):
    os.makedirs(outdir, exist_ok = True)
    x = np.arange(len(names))

    plt.rcParams.update({
        "font.size": 12,
        "axes.titlesize": 14,
        "axes.labelsize": 12,
        "figure.autolayout": False,
    })
    plt.style.use("seaborn-v0_8-whitegrid")

    W = max(8, len(names) * 1.6)

    fig, ax = plt.subplots(figsize = (W, 5.5))
    ax.bar(x, times, color = "#7FB3E0", edgecolor = "black", linewidth = 0.6)
    add_delta_labels(ax, times, fmt = "{:.1f}")
    ax.set_xticks(x)
    ax.set_xticklabels(names, rotation = 30, ha = "right")
    ax.set_ylabel("Время (мс)")
    ax.set_title("Время выполнения по реализациям")
    save_fig(fig, os.path.join(outdir, "time_ms.png"))

    fig, ax = plt.subplots(figsize = (W, 5.5))
    ticks_b = ticks / 1e9
    ax.bar(x, ticks_b, color = "#F0B27A", edgecolor = "black", linewidth = 0.6)
    add_delta_labels(ax, ticks_b, fmt = "{:.2f}")
    ax.set_xticks(x)
    ax.set_xticklabels(names, rotation = 30, ha = "right")
    ax.set_ylabel(r"Такты ($\times 10^9$)")
    ax.set_title("Процессорные такты по реализациям")
    save_fig(fig, os.path.join(outdir, "ticks.png"))

    fig, ax = plt.subplots(figsize = (W, 5.5))
    instrs_b = instrs / 1e9
    ax.bar(x, instrs_b, color = "#82E0AA", edgecolor = "black", linewidth = 0.6)
    add_delta_labels(ax, instrs_b, fmt = "{:.2f}")
    ax.set_xticks(x)
    ax.set_xticklabels(names, rotation = 30, ha = "right")
    ax.set_ylabel(r"Инструкции ($\times 10^9$)")
    ax.set_title("Количество инструкций (Ir) по реализациям")
    save_fig(fig, os.path.join(outdir, "instructions.png"))

    fig, ax = plt.subplots(figsize = (W + 1, 5.5))
    width = 0.25
    t_norm = times / times.max()
    k_norm = ticks / ticks.max()
    i_norm = instrs / instrs.max()

    bars_t = ax.bar(x - width, t_norm, width, label = "Время",
        color = "#7FB3E0", edgecolor = "black", linewidth = 0.5)
    bars_k = ax.bar(x, k_norm, width, label = "Такты",
        color = "#F0B27A", edgecolor = "black", linewidth = 0.5)
    bars_i = ax.bar(x + width, i_norm, width, label = "Инструкции",
        color = "#82E0AA", edgecolor = "black", linewidth = 0.5)

    add_delta_labels_grouped(ax, bars_t, t_norm)
    add_delta_labels_grouped(ax, bars_k, k_norm)
    add_delta_labels_grouped(ax, bars_i, i_norm)

    ax.set_xticks(x)
    ax.set_xticklabels(names, rotation = 30, ha = "right")
    ax.set_ylabel("Относительное значение (макс = 1)")
    ax.set_title("Сравнение реализаций (нормировано)")
    ax.legend()
    save_fig(fig, os.path.join(outdir, "summary.png"))

    baseline_idx = None
    for i, name in enumerate(names):
        if "-O3" in name or name == "-O3":
            baseline_idx = i
            break

    if baseline_idx is None:
        baseline_idx = 1

    baseline_time = times[baseline_idx]
    baseline_ticks = ticks[baseline_idx]
    baseline_instrs = instrs[baseline_idx]

    show_idxs = [i for i, name in enumerate(names)
        if i != baseline_idx and "-O0" not in name and name != "-O0"]

    print(f"\nГрафики сохранены в {outdir}/")
    print("  time_ms.png, ticks.png, instructions.png, summary.png")

    print(f"\nВыигрыш относительно baseline ({names[baseline_idx]}):")
    print(f"  {'Реализация':<30} {'Время':>16} {'Такты':>16} {'Инструкции':>16}")
    print(f"  {'-' * 78}")

    for i in show_idxs:
        gain_t = (baseline_time   - times[i])   / baseline_time   * 100
        gain_k = (baseline_ticks  - ticks[i])   / baseline_ticks  * 100
        gain_i = (baseline_instrs - instrs[i])  / baseline_instrs * 100

        ratio_t = baseline_time   / times[i]
        ratio_k = baseline_ticks  / ticks[i]
        ratio_i = baseline_instrs / instrs[i]

        print(f"  {names[i]:<30} {gain_t:>+7.2f}% ({ratio_t:.2f}x)"
              f" {gain_k:>+7.2f}% ({ratio_k:.2f}x)"
              f" {gain_i:>+7.2f}% ({ratio_i:.2f}x)")

    label_width = 50
    col_width = 22

    def trim(text, width):
        return text if len(text) <= width else text[:width - 3] + "..."

    print("\nУскорение относительно предыдущей версии:")

    header = (
        f"  {'Переход':<{label_width}} "
        f"{'Время':>{col_width}} "
        f"{'Такты':>{col_width}} "
        f"{'Инструкции':>{col_width}}"
    )

    print(header)
    print("  " + "-" * (label_width + 3 * (col_width + 1)))

    all_idxs = [baseline_idx] + show_idxs

    for prev, curr in zip(all_idxs[:-1], all_idxs[1:]):
        gain_t = (times[prev] - times[curr]) / times[prev] * 100
        gain_k = (ticks[prev] - ticks[curr]) / ticks[prev] * 100
        gain_i = (instrs[prev] - instrs[curr]) / instrs[prev] * 100

        ratio_t = times[prev] / times[curr]
        ratio_k = ticks[prev] / ticks[curr]
        ratio_i = instrs[prev] / instrs[curr]

        label = f"{names[prev]}  ->  {names[curr]}"
        label = trim(label, label_width)

        col_t = f"{gain_t:+7.2f}% ({ratio_t:5.2f}x)"
        col_k = f"{gain_k:+7.2f}% ({ratio_k:5.2f}x)"
        col_i = f"{gain_i:+7.2f}% ({ratio_i:5.2f}x)"

        print(f"  {label:<{label_width}} "
            f"{col_t:>{col_width}} "
            f"{col_k:>{col_width}} "
            f"{col_i:>{col_width}}")

    print("\nКПД = 2,81 / 14 * 1000 = 200,7")

def main():
    parser = argparse.ArgumentParser(description = "Визуализация профиля хеш-функций")
    parser.add_argument("csv_file", help = "Путь к CSV-файлу с данными")
    parser.add_argument("-o", "--outdir", default = "plots", help = "Папка для графиков (по умолчанию: plots)")
    args = parser.parse_args()

    names, times, ticks, instrs = read_data(args.csv_file)
    plot_all(names, times, ticks, instrs, args.outdir)


if __name__ == "__main__":
    main()