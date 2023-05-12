#!/bin/zsh

function run() {
    for n in {10..90..10} {100..300..50}; {
        start=$(date +%s.%N)
        repeat 100 python3 gengraph.py $n > graph && echo -e '0\n' | ./a.out graph > /dev/null
        end=$(date +%s.%N)
        echo $n $(($end - $start))
    }
}

run > bench.txt

gnuplot -e '
    set xlabel "size";
    set ylabel "time[sec]";
    set terminal pngcairo;
    set output "/tmp/output.png";
    plot "bench.txt" with linespoints title "time[sec]";
' && imgview /tmp/output.png
