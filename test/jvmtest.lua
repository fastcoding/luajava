local jvm_loader=assert(package.loadlib('./libluajava-1.1.dylib','luaopen_jvm'))
local javavm =jvm_loader()
local vm =javavm.create("-Djava.class.path=luajava-1.1.jar")
print'create ok'
for i, v in pairs(_G )do
print('key:',i)
end
do
	luajava=assert(vm:luajava())
	System = luajava.bindClass("java.lang.System")
	System.out:println("Hello, world!")
end
-- javavm destroying before lua_close will lead to crash during lua_close
-- collectgarbage()
-- javavm.destroy()
print('done')
