#!/bin/zsh

inputs=""
filter="\""

files=(*.mp3)

for ((i = 1; i <= $#files; i++));
do
    j=$((i-1))
    inputs="${inputs} -i ${files[i]}"
    filter="${filter}[${j}:a:0]"
done

filter="${filter}concat=n=${#files}:a=1:v=0[outa]\""

command=(ffmpeg $inputs -filter_complex $filter -map '"[outa]"' output_2.m4b)

echo "Command executed:"
echo "$command[@]"
echo " "
"${command[@]}"
