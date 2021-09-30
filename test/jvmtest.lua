local jvm_loader=assert(package.loadlib('./libluajava-1.1.dylib','luaopen_jvm'))
local javavm =jvm_loader()
local vm =javavm.create("-Djava.class.path=./luajava-1.1.jar")
print'create ok'
local luajava=assert(vm:luajava())
System = assert(luajava.bindClass)("java.lang.System")
System.out:println("Hello, world!")
ok,Console = pcall(luajava.bindClass,'org.keplerproject.luajava.Console')
if not ok then 
	print('got class:',Console)
end
-- ok,err=pcall(dofile, './test/awtTest.lua')
Console:main(luajava.newArray('string','./test/awtTest.lua'))
--ok,err=pcall(dofile, './test/swingTest.lua')
if not ok then 
--	print(err)
end

-- javavm destroying before lua_close will lead to crash during lua_close
-- collectgarbage()
-- javavm.destroy()
print('done')
