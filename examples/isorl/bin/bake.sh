#!/bin/sh

echo \"pal00000.cop\",\"PAL00000\" > data/main.csv
echo \"tex61440.rvr\",\"TEX61440\" >> data/main.csv
echo \"tex61441.rvr\",\"TEX61441\" >> data/main.csv

assets=(../assets/png/*.png)
asset_count=${#assets[*]}
asset_cur=0

for f in ../assets/png/*.png; do
   ../../../tools/texture-tool/texture-tool --trans 255 --in $f --pal ../assets/aap-splendor128.pal --out data/tex`basename $f .png`.rvr;
   asset_cur=$((asset_cur + 1))
   printf "\r(%5d/%5d) processed" $asset_cur $asset_count

   echo \"tex`basename $f .png`.rvr\",\"TEX`basename $f .png`\" >> data/main.csv;
done

echo ""
