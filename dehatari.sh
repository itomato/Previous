#!/bin/bash

for file in $(ack -l Hatari --ignore-file=is:dehatari.sh)
do
	sed -i 's/Hatari/Previous/g' $file
done

for file in $(ack -l hatari --ignore-file=is:dehatari.sh)
do
	sed -i 's/hatari/previous/g' $file
done

for file in $(find ./ -name "hatari*")
do
	mv $file $(echo $file | sed 's/hatari/previous/g')
done

for file in $(find ./ -name "hatari*")
    do mv $(echo $file | sed 's/hatari/previous/')
done


for file in $(find ./ -name "Hatari*")
    do mv $(echo $file | sed 's/Hatari/Previous/')
done
