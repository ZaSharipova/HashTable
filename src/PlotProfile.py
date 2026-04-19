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


def add_delta_labels(ax, values, fmt = "{:.1f}", delta_color_up = "#c0392b",
                     delta_color_down = "#27ae60"):
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


def add_delta_labels_grouped(ax, bars, values, delta_color_up = "#c0392b",
                             delta_color_down = "#27ae60"):
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

    # --- Время (мс) ---
    fig, ax = plt.subplots(figsize = (W, 5.5))
    ax.bar(x, times, color = "#7FB3E0", edgecolor = "black", linewidth = 0.6)
    add_delta_labels(ax, times, fmt = "{:.1f}")
    ax.set_xticks(x)
    ax.set_xticklabels(names, rotation = 30, ha = "right")
    ax.set_ylabel("Время (мс)")
    ax.set_title("Время выполнения по реализациям")
    save_fig(fig, os.path.join(outdir, "time_ms.png"))

    # --- Такты ---
    fig, ax = plt.subplots(figsize = (W, 5.5))
    ticks_b = ticks / 1e9
    ax.bar(x, ticks_b, color = "#F0B27A", edgecolor = "black", linewidth = 0.6)
    add_delta_labels(ax, ticks_b, fmt = "{:.2f}")
    ax.set_xticks(x)
    ax.set_xticklabels(names, rotation = 30, ha = "right")
    ax.set_ylabel(r"Такты ($\times 10^9$)")
    ax.set_title("Процессорные такты по реализациям")
    save_fig(fig, os.path.join(outdir, "ticks.png"))

    # --- Инструкции ---
    fig, ax = plt.subplots(figsize = (W, 5.5))
    instrs_b = instrs / 1e9
    ax.bar(x, instrs_b, color = "#82E0AA", edgecolor = "black", linewidth = 0.6)
    add_delta_labels(ax, instrs_b, fmt = "{:.2f}")
    ax.set_xticks(x)
    ax.set_xticklabels(names, rotation = 30, ha = "right")
    ax.set_ylabel(r"Инструкции ($\times 10^9$)")
    ax.set_title("Количество инструкций (Ir) по реализациям")
    save_fig(fig, os.path.join(outdir, "instructions.png"))

    # --- Сводный (нормированный) ---
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

    # --- Итоговый выигрыш ---
    baseline_time   = times[1]
    baseline_ticks  = ticks[1]
    baseline_instrs = instrs[1]

    last = len(names) - 1
    gain_time   = (baseline_time   - times[last])   / baseline_time   * 100
    gain_ticks  = (baseline_ticks  - ticks[last])   / baseline_ticks  * 100
    gain_instrs = (baseline_instrs - instrs[last])  / baseline_instrs * 100

    print(f"\nГрафики сохранены в {outdir}/")
    print("  time_ms.png, ticks.png, instructions.png, summary.png")
    print(f"\nОбщий выигрыш последней реализации ({names[last]}) относительно baseline ({names[1]}):")
    print(f"  Время:        {gain_time:+.2f}%")
    print(f"  Такты:        {gain_ticks:+.2f}%")
    print(f"  Инструкции:   {gain_instrs:+.2f}%")


def main():
    parser = argparse.ArgumentParser(description = "Визуализация профиля хеш-функций")
    parser.add_argument("csv_file", help = "Путь к CSV-файлу с данными")
    parser.add_argument("-o", "--outdir", default = "plots", help = "Папка для графиков (по умолчанию: plots)")
    args = parser.parse_args()

    names, times, ticks, instrs = read_data(args.csv_file)
    plot_all(names, times, ticks, instrs, args.outdir)


if __name__ == "__main__":
    main()