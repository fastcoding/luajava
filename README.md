luajava
=======

LuaJava is a scripting tool for Java. The goal of this tool is to allow scripts written in Lua to manipulate components developed in Java. 

It allows Java components to be accessed from Lua using the same syntax that is used for accessing Lua`s native objects, without any need 
for declarations or any kind of preprocessing.  LuaJava also allows Java to implement an interface using Lua. This way any interface can be
implemented in Lua and passed as parameter to any method, and when called, the equivalent function will be called in Lua, and it's result 
passed back to Java.

Added:

On macos, luajit v2.1 has to be used - or , crash at luaL_newstate()

Bugfix:
Unable to callback to java object in coroutine: replaced getExistingState with attach(pointer) to create a new LuaState object in java


Updated on 30 Sep 2021:
- added support to create javavm from lua. see `test/jvmtest.lua`.
  
