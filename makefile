all: main.o

main.o: main.c lua
	emcc -I lua-5.3.5/src main.c lua-5.3.5/src/liblua.a -s WASM=1 -O2 -o ./dist/lua.html -o ./dist/lua.mjs -s EXPORTED_FUNCTIONS="['_initContext','_runCode', '_destroyContext', '_postMessage']" -s EXTRA_EXPORTED_RUNTIME_METHODS="['ccall', 'cwrap']"

lua:
	cd lua-5.3.5/src && make generic CC='emcc -s WASM=1'