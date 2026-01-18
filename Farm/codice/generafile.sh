#!/bin/bash

if [ ! -e generafile ];
then
    echo "Compilare generafile, eseguibile mancante!";
    exit 1
fi
# generafile genera i file file100.dat file150.dat file19.dat file116.dat...
# in modo deterministico
#
j=1
for i in 100 150 19 116 2 1 117 3 5 17 4 16 19 8 10 111 12 13 14 15 18 20; do
    ./generafile file$i.dat $(($i*11 + $j*117)) > /dev/null
    j=$(($j+3))
done

mkdir -p testdir
mv file19.dat file8.dat testdir
mkdir -p testdir/testdir2
mv file111.dat file150.dat testdir/testdir2
