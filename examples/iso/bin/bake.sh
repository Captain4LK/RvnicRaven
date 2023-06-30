#!/bin/sh

echo \"pal00000.cop\",\"PAL00000\" > data/main.csv
echo \"tex61440.rvr\",\"TEX61440\" >> data/main.csv
echo \"tex61441.rvr\",\"TEX61441\" >> data/main.csv

for f in ../assets/png/*.png; do
   if [ `basename $f .png` -ge 16384 ]; then
      ../../../tools/texture-tool/texture-tool --in $f --pal ../assets/aap-splendor128.pal --out data/tex`basename $f .png`.rvr;
   else
      ../../../tools/texture-tool/texture-tool --in $f --pal ../assets/aap-splendor128.pal --out data/tex`basename $f .png`.rvr;
   fi

   echo \"tex`basename $f .png`.rvr\",\"TEX`basename $f .png`\" >> data/main.csv;
done
