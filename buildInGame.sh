#!/bin/sh

svn revert -R .
cd ./GameCore \
	&& svn propget svn:externals \
	&& svn  propset svn:externals "https://192.168.1.208:4433/svn/ProjectF4/2.Client/Assets/InGame/Res Res" . \
	&& svn propget svn:externals

cd ..

svn up
svn up ./InGame

make clean
make

svn commit ./InGame/bin -m "ready make docker image"
