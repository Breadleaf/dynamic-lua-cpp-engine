#include <filesystem>
#include <iostream>

#include "lua.hpp"

void registerModAPI(lua_State* L);

void loadMods(lua_State* L) {
	const std::string modsDir = "./mods/";

	for (const auto& entry : std::filesystem::directory_iterator(modsDir)) {
		if (entry.path().extension() == ".lua") {
			if (luaL_dofile(L, entry.path().c_str()) != LUA_OK) {
				std::cerr << "Error loading mod: " << lua_tostring(L, -1) << std::endl;
				lua_pop(L, 1);
			} else {
				std::cout << "Loaded mod: " << entry.path().filename() << std::endl;
			}
		}
	}
}

void printMods(lua_State* L) {
	lua_getglobal(L, "registeredMods");
	if (lua_isnil(L, -1)) {
		std::cout << "No registered mods found" << std::endl;
		lua_pop(L, 1);
		return;
	}

	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		std::cout << "Mod: " << lua_tostring(L, -2) << ":" << std::endl;

		if (lua_istable(L, -1)) {
			lua_pushstring(L, "version");
			lua_gettable(L, -2);
			std::cout << "- Version: " << lua_tostring(L, -1) << std::endl;
			lua_pop(L, 1);

			lua_pushstring(L, "author");
			lua_gettable(L, -2);
			std::cout << "- Author: " << lua_tostring(L, -1) << std::endl;
			lua_pop(L, 1);
		}

		lua_pop(L, 1);
	}

	lua_pop(L, 1);
}

int main() {
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	registerModAPI(L);

	loadMods(L);

	printMods(L);

	lua_close(L);
	return 0;
}

int lua_createMod(lua_State* L) {
	const char* name = luaL_checkstring(L, 1);
	const char* version = luaL_checkstring(L, 2);
	const char* author = luaL_checkstring(L, 3);

	lua_newtable(L); // Create the table for the mod

	// Set the fields of the mod table
	lua_pushstring(L, name);
	lua_setfield(L, -2, "name");
	lua_pushstring(L, version);
	lua_setfield(L, -2, "version");
	lua_pushstring(L, author);
	lua_setfield(L, -2, "author");

	// Add to the global "registeredMods" table
	lua_getglobal(L, "registeredMods");
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		lua_newtable(L);
		lua_pushvalue(L, -1);
		lua_setglobal(L, "registeredMods");
	}

	lua_pushvalue(L, -2);
	lua_setfield(L, -2, name);
	lua_pop(L, 1);

	return 1;
}

void registerModAPI(lua_State* L) {
	lua_newtable(L);

	lua_pushcfunction(L, lua_createMod);
	lua_setfield(L, -2, "create");

	lua_setglobal(L, "mod");
}
