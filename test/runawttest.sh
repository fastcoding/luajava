#!/bin/sh
LD_LIBRARY_PATH=$(cd ..;$pwd)
echo $LD_LIBRARY_PATH
java -cp "../luajava-1.1.jar" -Djava.library.path=.. org.keplerproject.luajava.Console awtTest.lua
