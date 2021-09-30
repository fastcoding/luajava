/*
 * $Id$
 * Provides the Java VM module. See LICENSE.txt for license terms.
 */

#include <stdlib.h>
#include <string.h>
#include <jni.h>
#include <lauxlib.h>
#ifdef LUA_WIN
#include <stddef.h>
#pragma warning(disable : 4996)
#endif
#ifdef LUA_USE_POSIX
#include <stdint.h>
#endif
#include "javavm.h"
/*
 * Java VM parameters.
 */
#define JAVAVM_METATABLE "javavm.metatable"
#define JAVAVM_VM "javavm.vm"
#define JAVAVM_MAXOPTIONS 128
#define JAVAVM_JNIVERSION JNI_VERSION_1_6

/*
 * VM record.
 */
typedef struct vm_rec {
	JavaVM *vm;
	int num_options;
	JavaVMOption options[JAVAVM_MAXOPTIONS];
} vm_rec;

/*
 * Raises an error from JNI.
 */
static int error (lua_State *L, JNIEnv *env, const char *msg) {
	jthrowable throwable;
	jclass throwable_class;
	jmethodID tostring_id;
	jstring string;
	const char *extramsg = NULL;
	
	throwable = (*env)->ExceptionOccurred(env);
	if (throwable) {
		throwable_class = (*env)->GetObjectClass(env, throwable);
		if ((tostring_id = (*env)->GetMethodID(env, throwable_class, "toString", "()Ljava/lang/String;"))) {
			string = (*env)->CallObjectMethod(env, throwable, tostring_id);
			if (string) {
				extramsg = (*env)->GetStringUTFChars(env, string, NULL);
			}
		}
	}
	if (extramsg) {
		lua_pushfstring(L, "%s (%s)", msg, extramsg);
		(*env)->ReleaseStringUTFChars(env, string, extramsg);
	} else {
		lua_pushstring(L, msg);
	}
	return luaL_error(L, lua_tostring(L, -1));
}

/*
 * Releases a VM.
 */
static int release_vm (lua_State *L) {
	vm_rec *vm;
	jmethodID close_id;
	JNIEnv *env;
	int res;
	int i;
	
	/* Get VM */
	vm = luaL_checkudata(L, 1, JAVAVM_METATABLE);
	
	/* Already released? */
	if (!vm->vm) {
		return 0;
	}
	
	/* Check thread */
	if ((*vm->vm)->GetEnv(vm->vm, (void **) &env, JAVAVM_JNIVERSION) != JNI_OK) {
		return luaL_error(L, "invalid thread");
	}
	
	/* Destroy the Java VM */
	res = (*vm->vm)->DestroyJavaVM(vm->vm);
	if (res < 0) {
		return luaL_error(L, "error destroying Java VM: %d", res);
	}
	vm->vm = NULL;

	/* Free options */
	for (i = 0; i < vm->num_options; i++) {
		free(vm->options[i].optionString);
		vm->options[i].optionString = NULL;
	}
	vm->num_options = 0;
	
	return 0;
}

/*
 * Returns a string representation of a VM.
 */
static int tostring_vm (lua_State *L) {
	vm_rec *vm;
	int i;
	
	vm = luaL_checkudata(L, 1, JAVAVM_METATABLE);
	lua_pushfstring(L, "Java VM (%p)", vm->vm);
	luaL_checkstack(L, vm->num_options, NULL);
	for (i = 0; i < vm->num_options; i++) {
		lua_pushfstring(L, "\n\t%s", vm->options[i].optionString);
	}
	lua_concat(L, vm->num_options + 1);
	return 1;
}

static int open_luajava(lua_State *L){
	vm_rec *vm;
	jclass luastateFactory_class, library_class;
	jmethodID init_id,initLua_id ;
	JNIEnv *env;

	vm = luaL_checkudata(L, 1, JAVAVM_METATABLE);

	if (!vm || !vm->vm) {
		luaL_error(L,"expects java vm");
	}

	int getEnvStat = (*vm->vm)->GetEnv(vm->vm,(void **)&env, JAVAVM_JNIVERSION);
	if (getEnvStat == JNI_EDETACHED) {
        if ((*vm->vm)->AttachCurrentThread(vm->vm,(void **) &env, NULL) != 0) {
			luaL_error(L, "Failed to attach");
        }
    } else if (getEnvStat == JNI_OK) {
        //
		if (!(luastateFactory_class = (*env)->FindClass(env, "org/keplerproject/luajava/LuaStateFactory"))
			|| !(initLua_id= (*env)->GetStaticMethodID(env, luastateFactory_class, "openLuajava","(J)V" ))) {
			return error(L, env, "unable to attach luastate");
		}
		(*env)->CallStaticVoidMethod(env, luastateFactory_class, initLua_id, (jlong) (uintptr_t) L);
		return 1;
    } else if (getEnvStat == JNI_EVERSION) {
		luaL_error(L, "GetEnv: version not supported");
    } else {
		luaL_error(L, "GetEnv: failed state=0x%x", getEnvStat);
	}
	return 0;
}


/*
 * Creates a VM.
 */
static int create_vm (lua_State *L) {
	vm_rec *vm;
	int i;
	const char *option;
	JavaVMInitArgs vm_args;
	int res;
	JNIEnv *env;
	/* Check for existing VM */
	lua_getfield(L, LUA_REGISTRYINDEX, JAVAVM_VM);
	if (!lua_isnil(L, -1)) {
		return luaL_error(L, "VM already created");
	}
	lua_pop(L, 1);
	
	/* Create VM */
	vm = lua_newuserdata(L, sizeof(vm_rec));
	memset(vm, 0, sizeof(vm_rec));
	luaL_getmetatable(L, JAVAVM_METATABLE);
	lua_setmetatable(L, -2);
	
	/* Process options */
	vm->num_options = lua_gettop(L) - 1;
	if (vm->num_options > JAVAVM_MAXOPTIONS) {
		return luaL_error(L, "%d options limit, got %d", JAVAVM_MAXOPTIONS, vm->num_options);
	}
	for (i = 1; i <= vm->num_options; i++) {
		option = luaL_checkstring(L, i);
		if (strcmp(option, "vfprintf") == 0
				|| strcmp(option, "exit") == 0
				|| strcmp(option, "abort") == 0) {
			return luaL_error(L, "unsupported option '%s'", option);
		}
		vm->options[i - 1].optionString = strdup(option);
		if (!vm->options[i - 1].optionString) {
			return luaL_error(L, "out of memory");
		}
	}
	
	/* Create Java VM */
	vm_args.version = JAVAVM_JNIVERSION;
	vm_args.options = vm->options;
	vm_args.nOptions = vm->num_options;
	vm_args.ignoreUnrecognized = JNI_TRUE;
	res = JNI_CreateJavaVM(&vm->vm, (void**) &env, &vm_args);
	if (res < 0) {
		return luaL_error(L, "error creating Java VM: %d", res);
	}
	
	/* Store VM */
	lua_pushvalue(L, -1);
	lua_setfield(L, LUA_REGISTRYINDEX, JAVAVM_VM);
	
	return 1;
}

/*
 * Destroys the VM.
 */
static int destroy_vm (lua_State *L) {
	/* Release VM, if any */
	lua_pushcfunction(L, release_vm);
	lua_getfield(L, LUA_REGISTRYINDEX, JAVAVM_VM);
	if (lua_isnil(L, -1)) {
		/* No VM to destroy */
		lua_pushboolean(L, 0);
		return 1;
	}
	lua_call(L, 1, 0);
	
	/* Clear VM */
	lua_pushnil(L);
	lua_setfield(L, LUA_REGISTRYINDEX, JAVAVM_VM);
	
	/* Success */
	lua_pushboolean(L, 1);
	return 1;
}

/*
 * Returns the VM, if any.
 */
static int get_vm (lua_State *L) {
	lua_getfield(L, LUA_REGISTRYINDEX, JAVAVM_VM);
	return 1;
}

/*
 * Java VM module functions.
 */
static const luaL_Reg functions[] = {
	{ "create", create_vm },
	{ "destroy", destroy_vm },
	{ "get", get_vm },
	{ NULL, NULL }
};

/*
 * Exported functions.
 */ 
LUALIB_API int luaopen_jvm(lua_State *L) {
	/* Create module */
	lua_newtable(L);
	luaL_register(L, NULL, functions);
	
	/* Create metatable */
	luaL_newmetatable(L, JAVAVM_METATABLE);
	lua_pushcfunction(L, release_vm);

	lua_setfield(L, -2, "__gc");
	lua_pushcfunction(L, tostring_vm);
	lua_setfield(L, -2, "__tostring");
	lua_newtable(L);	
	lua_pushcfunction(L, open_luajava);
	lua_setfield(L, -2, "luajava");
	lua_setfield(L,-2,"__index");
	lua_pop(L, 1);
	
	return 1;
}
