#!/bin/bash
#
# Prepare linux packages for Previous
# itomato
# Fri Oct 13 15:14:47 PDT 2017
#
###########################################

# Get proper version from src/includes/main.h
# Strip "Previous N.x", replace space with '-'...
VERSION=$(grep PROG_NAME src/includes/main.h | cut -d'"' -f2 | sed 's/ /-/')

# Make binary tarball with new version string
# Previous-N.x

# make a usr/bin directory for the root of a tarball
mkdir -p usr/bin
cp src/Previous usr/bin/

# Create a tar
tar -cvf $VERSION.Linux.tar etc/ usr/ 
# Compress the tar
bzip2 $VERSION.Linux.tar 

# Make distro packages...
build_rpm(){
	which rpmbuild

	case $? in

		1)
			echo "Can't build RPM without rpmbuild."
			exit
			;;
		*)
			rpmbuild -ba previous.spec
			;;
	esac
}


scp $VERSION.Linux.tar.bz2 itomato@juddy.org:/home/itomato/juddy.org/previous/
