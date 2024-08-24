FILES="font10x16.h font10x16.xbm sdlgui.c"
for file in $FILES
do
	echo $file
	OLDFILE=$(find ../previous-code -name $file )
	cp $file $OLDFILE
		if [ $? -ne 0 ]
		then
			echo "Failed to replace $file"
		else
			echo "Patched!"

		fi
	#diff ./$file $(find ../previous-code -name $file )
done

