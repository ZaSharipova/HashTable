set terminal pngcairo size 1280,720 enhanced font 'Verdana,10'
set output "images/temperature_plot.png"
set title "CPU Core 2 Temperature"
set xlabel "Время (с)"
set ylabel "Температура (°C)"
set grid
plot "temp.txt" using 1:2 with lines title "Core 2" linewidth 2