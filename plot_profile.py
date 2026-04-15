#!/usr/bin/env python3

import argparse
import csv
import os
import matplotlib.pyplot as plt
import matplotlib.ticker as mticker
import numpy as np


def read_data(filepath: str):
    names, times, ticks, instrs = [], [], [], []
    with open(filepath, newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        for row in reader:
            names.append(row["name"])
            times.append(float(row["time_ms"]))
            ticks.append(int(row["ticks"]))
            instrs.append(int(row["instructions"]))
    return names, np.array(times), np.array(ticks), np.array(instrs)


def bar_with_labels(ax, x, values, color, fmt="{:.1f}"):
    bars = ax.bar(x, values, color=color, edgecolor="black", linewidth=0.6)
    for bar, val in zip(bars, values):
        ax.text(
            bar.get_x() + bar.get_width() / 2,
            bar.get_height(),
            fmt.format(val),
            ha="center", va="bottom", fontsize=8, fontweight="bold",
        )
    return bars


def save_fig(fig, path):
    """Сохраняем с достаточными отступами, чтобы ничего не обрезалось."""
    fig.savefig(path, dpi=180, bbox_inches="tight", pad_inches=0.3)
    plt.close(fig)


def plot_all(names, times, ticks, instrs, outdir: str):
    os.makedirs(outdir, exist_ok=True)
    x = np.arange(len(names))

    plt.rcParams.update({
        "font.size": 12,
        "axes.titlesize": 14,
        "axes.labelsize": 12,
        "figure.autolayout": False,
    })
    plt.style.use("seaborn-v0_8-whitegrid")

    W = max(8, len(names) * 1.6)

    fig, ax = plt.subplots(figsize=(W, 5.5))
    bar_with_labels(ax, x, times, "#4C72B0", fmt="{:.1f}")
    ax.set_xticks(x); ax.set_xticklabels(names, rotation=30, ha="right")
    ax.set_ylabel("Время (мс)")
    ax.set_title("Время выполнения по реализациям")
    save_fig(fig, os.path.join(outdir, "time_ms.png"))

    fig, ax = plt.subplots(figsize=(W, 5.5))
    ticks_b = ticks / 1e9
    bar_with_labels(ax, x, ticks_b, "#DD8452", fmt="{:.2f}")
    ax.set_xticks(x); ax.set_xticklabels(names, rotation=30, ha="right")
    ax.set_ylabel(r"Такты ($\times 10^9$)")
    ax.set_title("Процессорные такты по реализациям")
    save_fig(fig, os.path.join(outdir, "ticks.png"))

    fig, ax = plt.subplots(figsize=(W, 5.5))
    instrs_b = instrs / 1e9
    bar_with_labels(ax, x, instrs_b, "#55A868", fmt="{:.2f}")
    ax.set_xticks(x); ax.set_xticklabels(names, rotation=30, ha="right")
    ax.set_ylabel(r"Инструкции ($\times 10^9$)")
    ax.set_title("Количество инструкций (Ir) по реализациям")
    save_fig(fig, os.path.join(outdir, "instructions.png"))

    fig, ax = plt.subplots(figsize=(W + 1, 5.5))
    width = 0.25
    t_norm = times / times.max()
    k_norm = ticks / ticks.max()
    i_norm = instrs / instrs.max()

    ax.bar(x - width, t_norm, width, label="Время",        color="#4C72B0", edgecolor="black", linewidth=0.5)
    ax.bar(x,         k_norm, width, label="Такты",         color="#DD8452", edgecolor="black", linewidth=0.5)
    ax.bar(x + width, i_norm, width, label="Инструкции",    color="#55A868", edgecolor="black", linewidth=0.5)

    ax.set_xticks(x); ax.set_xticklabels(names, rotation=30, ha="right")
    ax.set_ylabel("Относительное значение (макс = 1)")
    ax.set_title("Сравнение реализаций (нормировано)")
    ax.legend()
    save_fig(fig, os.path.join(outdir, "summary.png"))

    print(f"Графики сохранены в {outdir}/")
    print("  time_ms.png, ticks.png, instructions.png, summary.png")


def main():
    parser = argparse.ArgumentParser(description="Визуализация профиля хеш-функций")
    parser.add_argument("csv_file", help="Путь к CSV-файлу с данными")
    parser.add_argument("-o", "--outdir", default="plots", help="Папка для графиков (по умолчанию: plots)")
    args = parser.parse_args()

    names, times, ticks, instrs = read_data(args.csv_file)
    plot_all(names, times, ticks, instrs, args.outdir)


if __name__ == "__main__":
    main()