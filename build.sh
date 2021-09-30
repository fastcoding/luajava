#!/bin/sh
LUAJIT=$(which luajit)
JVM_DIR=/Library/Java/JavaVirtualMachines
LUA_DIR=$(cd $(dirname $LUAJIT)/..;pwd) 
LUA_LIBDIR=$LUA_DIR/lib
LUA_INCLUDES=$LUA_DIR/include/luajit-2.1
JAVA_HOME=$JVM_DIR/$(ls $JVM_DIR|tail -n1)/Contents/Home
make LUA_DIR=$LUA_DIR \
     LUA_LIBDIR=$LUA_LIBDIR \
     LUA_INCLUDES=$LUA_INCLUDES \
     JAVA_HOME=$JAVA_HOME  "$@"
     
