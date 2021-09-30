#!/bin/sh
testdir=$(dirname $0)
LD_LIBRARY_PATH=$(cd $testdir/..;pwd)
echo $LD_LIBRARY_PATH
java -cp "$testdir/../luajava-1.1.jar" -Djava.library.path=$testdir/.. org.keplerproject.luajava.Console $testdir/awtTest.lua
