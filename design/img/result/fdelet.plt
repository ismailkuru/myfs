#!/usr/bin/gnuplot
set term png
set output "fdelet.png"
set xlabel "file system"
set ylabel "ops/sec"
set style fill solid
set boxwidth 0.5
plot "fdelet.dat" using 2:xtic(1)         notitle with boxes, \
     ''          using 0:($2 + 1000):($2) notitle with labels
