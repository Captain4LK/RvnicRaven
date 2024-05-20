#!/bin/sh

echo \"pal00000.cop\",\"PAL00000\" > data/main.csv
echo \"tex61440.rvr\",\"TEX61440\" >> data/main.csv
echo \"tex61441.rvr\",\"TEX61441\" >> data/main.csv

for f in data/*.map; do
   echo \"`basename $f`\",\"MAP`echo $f | tr -d -c 0-9`\" >> data/main.csv;
done

for f in ../assets/png/*.png; do
    ../../../tools/texture-tool/texture-tool --in $f --wall --pal ../assets/palette.png --out data/tex`basename $f .png`.rvr;

   echo \"tex`basename $f .png`.rvr\",\"TEX`basename $f .png`\" >> data/main.csv;
done
