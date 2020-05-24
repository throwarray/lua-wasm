#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <emscripten/emscripten.h>
#include <stdlib.h>

lua_State* lscript_context = 0;

int lscript_table = 0;

EM_JS(const char*, call_js_post_message, (const char* message), {
  const str = self.onMessage(UTF8ToString(message) || "") || "";
  const lengthBytes = lengthBytesUTF8(str)+1;
  const mem = _malloc(lengthBytes);

  stringToUTF8(str, mem, lengthBytes);
  
  return mem;
});

// LUA C methods
int handle_lua_print(lua_State *L) {
	size_t len = 0;
	
	const char* value = lua_tolstring(L, -1, &len);
	
	printf("%s\n", value);

	return 0;
}

int handle_lua_message (lua_State *L) {
	size_t len = 0;

	const char* message = lua_tolstring(L, lua_gettop(L), &len);

 	const char* str = call_js_post_message(message);

	lua_pushstring(L, str);
	
	// free(str);
	
	return 1;
}

// API methods
const char* EMSCRIPTEN_KEEPALIVE runCode(const char* value) {
	lua_State* lua = lscript_context;

	int res = luaL_dostring(lua, value);

	size_t len = 0;

	const char* output = lua_tolstring(lua, lua_gettop(lua), &len);

	return output;
}

// invoke lua _G.onmessage
const char* EMSCRIPTEN_KEEPALIVE postMessage(const char* value) {
	lua_State* L = lscript_context;
	lua_getglobal(L, "onmessage");
	lua_pushstring(L, value);
	lua_call(L, 1, 1);
	size_t len = 0;
	const char* output = lua_tolstring(L, lua_gettop(L), &len);
	return output;
}

int EMSCRIPTEN_KEEPALIVE destroyContext() {
	lua_State* prev = lscript_context;
	lua_close(prev);
	free(prev);

	lscript_context = 0;
	lscript_table = 0;

	return 0;
}


int EMSCRIPTEN_KEEPALIVE initContext() {
	if (lscript_context != 0) destroyContext();

	lua_State* L = luaL_newstate();
	lua_register(L, "postMessage", handle_lua_message);	
	lua_register(L, "print", handle_lua_print);
	lua_newtable(L);

	int tab_idx = luaL_ref(L,LUA_REGISTRYINDEX);
	lua_pop(L,1);

	lscript_context = L;
	lscript_table = tab_idx;
	
	return 0;
}
