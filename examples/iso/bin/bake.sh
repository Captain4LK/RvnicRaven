#!/bin/sh

echo \"pal00000.cop\",\"PAL00000\" > data/main.csv
echo \"tex61440.rvr\",\"TEX61440\" >> data/main.csv
echo \"tex61441.rvr\",\"TEX61441\" >> data/main.csv

# Floor
assets=(../assets/floor/*.png)
asset_count=${#assets[*]}
asset_cur=0

for f in ../assets/floor/*.png; do
   ./graphics-builder -t floor -i $f -p ../assets/palette.png --out data/flor`basename $f .png`.grp;
   asset_cur=$((asset_cur + 1))
   printf "\r(%5d/%5d) floor graphics processed (1/6)" $asset_cur $asset_count

   echo \"flor`basename $f .png`.grp\",\"FLOR`basename $f .png`\" >> data/main.csv;
done

echo ""

# Wall
assets=(../assets/wall/*.png)
asset_count=${#assets[*]}
asset_cur=0

for f in ../assets/wall/*.png; do
   ./graphics-builder -t wall -i $f -p ../assets/palette.png --out data/wall`basename $f .png`.grp;
   asset_cur=$((asset_cur + 1))
   printf "\r(%5d/%5d) wall graphics processed  (2/6)" $asset_cur $asset_count

   echo \"wall`basename $f .png`.grp\",\"WALL`basename $f .png`\" >> data/main.csv;
done

echo ""

# Block
assets=(../assets/block/*.png)
asset_count=${#assets[*]}
asset_cur=0

for f in ../assets/block/*.png; do
   ./graphics-builder -t block -i $f -p ../assets/palette.png --out data/blck`basename $f .png`.grp;
   asset_cur=$((asset_cur + 1))
   printf "\r(%5d/%5d) block graphics processed  (3/6)" $asset_cur $asset_count

   echo \"blck`basename $f .png`.grp\",\"BLCK`basename $f .png`\" >> data/main.csv;
done

echo ""

# Slope
assets=(../assets/slope/*.png)
asset_count=${#assets[*]}
asset_cur=0

for f in ../assets/slope/*.png; do
   ./graphics-builder -t slope -i $f -p ../assets/palette.png --out data/slpe`basename $f .png`.grp;
   asset_cur=$((asset_cur + 1))
   printf "\r(%5d/%5d) slope graphics processed  (4/6)" $asset_cur $asset_count

   echo \"slpe `basename $f .png`.grp\",\"SLPE`basename $f .png`\" >> data/main.csv;
done

echo ""

# SSlope
assets=(../assets/sslope/*.png)
asset_count=${#assets[*]}
asset_cur=0

for f in ../assets/sslope/*.png; do
   ./graphics-builder -t sslope -i $f -p ../assets/palette.png --out data/sslp`basename $f .png`.grp;
   asset_cur=$((asset_cur + 1))
   printf "\r(%5d/%5d) sslope graphics processed  (5/6)" $asset_cur $asset_count

   echo \"sslp `basename $f .png`.grp\",\"SSLP`basename $f .png`\" >> data/main.csv;
done

echo ""

# Sprite
assets=(../assets/sprite/*.png)
asset_count=${#assets[*]}
asset_cur=0

for f in ../assets/sprite/*.png; do
   ./graphics-builder -t sprite -i $f -p ../assets/palette.png --out data/sprt`basename $f .png`.grp;
   asset_cur=$((asset_cur + 1))
   printf "\r(%5d/%5d) sprite graphics processed  (6/6)" $asset_cur $asset_count

   echo \"sprt`basename $f .png`.grp\",\"SPRT`basename $f .png`\" >> data/main.csv;
done

echo ""
