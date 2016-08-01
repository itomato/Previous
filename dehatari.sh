FILES=$(ack -l Hatari)
echo "Upper"
for FILE in $FILES
do
	sed -i .bak 's/Hatari/Previous/g' $FILE
done


echo "lower"
files=$(ack -l previous)

for file in $files
do
	sed -i .bak 's/previous/previous/g' $file
done


for file in $(find ./ -name hatari*) ; do mv $file $(echo $file|sed 's/hatari/previous/') ; done
for file in $(find ./ -name Hatari*) ; do mv $file $(echo $file|sed 's/Hatari/Previous/') ; done
