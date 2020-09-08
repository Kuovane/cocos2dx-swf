/****************************************************************************
 Copyright (c) 2017 Chukong Technologies Inc.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
#include "lua_swf_manual.hpp"

#include "scripting/lua-bindings/manual/tolua_fix.h"
#include "scripting/lua-bindings/manual/LuaBasicConversions.h"
#include "scripting/lua-bindings/manual/cocos2d/LuaScriptHandlerMgr.h"
#include "scripting/lua-bindings/manual/CCLuaValue.h"
#include "scripting/lua-bindings/manual/CCLuaEngine.h"
#include "base/CCEventListenerFocus.h"
#include "SwfManager.h"





static int lua_SwfManager_getInstance(lua_State* tolua_S)
{
	int argc = 0;
	bool ok = true;

#if COCOS2D_DEBUG >= 1
	tolua_Error tolua_err;
#endif

#if COCOS2D_DEBUG >= 1
	if (!tolua_isusertable(tolua_S, 1, "SwfManager", 0, &tolua_err)) goto tolua_lerror;
#endif

	argc = lua_gettop(tolua_S) - 1;

	if (argc == 0)
	{

		SwfManager* ret = SwfManager::getInstance();
		object_to_luaval<SwfManager>(tolua_S, "SwfManager", (SwfManager*)ret);
		return 1;
	}
	luaL_error(tolua_S, "%s has wrong number of arguments: %d, was expecting %d\n ", "SwfManager:getInstance", argc, 0);
	return 0;
#if COCOS2D_DEBUG >= 1
	tolua_lerror:
				tolua_error(tolua_S, "#ferror in function 'lua_SwfManager_getInstance'.", &tolua_err);
#endif
				return 0;
}



int lua_SwfManager_genAniData(lua_State* tolua_S)
{
	int argc = 0;
	SwfManager* cobj = nullptr;
	bool ok = true;

#if COCOS2D_DEBUG >= 1
	tolua_Error tolua_err;
#endif


#if COCOS2D_DEBUG >= 1
	if (!tolua_isusertype(tolua_S, 1, "SwfManager", 0, &tolua_err)) goto tolua_lerror;
#endif

	cobj = (SwfManager*)tolua_tousertype(tolua_S, 1, 0);

#if COCOS2D_DEBUG >= 1
	if (!cobj)
	{
		tolua_error(tolua_S, "invalid 'cobj' in function 'lua_SwfManager_genAniData'", nullptr);
		return 0;
	}
#endif
	argc = lua_gettop(tolua_S) - 1;
	if (argc == 2)
	{
		std::string arg0;
		ok &= luaval_to_std_string(tolua_S, 2, &arg0, "SwfManager:genAniData");
		if (!ok)
		{
			tolua_error(tolua_S, "invalid arguments in function 'lua_SwfManager_genAniData'", nullptr);
			return 0;
		}


		LUA_FUNCTION handler = (toluafix_ref_function(tolua_S, 3, 0));


		auto func = [=](bool bSuccess)
		{
			auto stack = cocos2d::LuaEngine::getInstance()->getLuaStack();

			stack->pushBoolean(bSuccess);
			//stack->pushInt(static_cast<int>(type));
			//stack->pushObject(collider1, "creator.Collider");
			//stack->pushObject(collider2, "creator.Collider");
			stack->executeFunctionByHandler(handler, 1);
			stack->clean();
		};

		ScriptHandlerMgr::getInstance()->addCustomHandler((void*)cobj, handler);

		cobj->genAniData(arg0, func);

		lua_settop(tolua_S, 1);
		return 1;

	}
	
	luaL_error(tolua_S, "%s has wrong number of arguments: %d, was expecting %d \n", "SwfManager:genAniData", argc, 2);
	return 0;

#if COCOS2D_DEBUG >= 1
	tolua_lerror:
				tolua_error(tolua_S, "#ferror in function 'lua_SwfManager_genAniData'.", &tolua_err);
#endif

				return 0;
}

int lua_SwfManager_setCachePath(lua_State* tolua_S)
{
	int argc = 0;
	SwfManager* cobj = nullptr;
	bool ok = true;



	cobj = (SwfManager*)tolua_tousertype(tolua_S, 1, 0);

	argc = lua_gettop(tolua_S) - 1;
	if (argc == 1)
	{
		std::string arg0;
		ok &= luaval_to_std_string(tolua_S, 2, &arg0, "SwfManager:setCachePath");
		if (!ok)
		{
			tolua_error(tolua_S, "invalid arguments in function 'lua_SwfManager_setCachePath'", nullptr);
			return 0;
		}

	
		cobj->setCachePath(arg0 );

		lua_settop(tolua_S, 1);
		return 1;

	}

	luaL_error(tolua_S, "%s has wrong number of arguments: %d, was expecting %d \n", "SwfManager:genAniData", argc, 2);
	return 0;

}

int lua_SwfManager_getFileName(lua_State* tolua_S)
{
	int argc = 0;
	SwfManager* cobj = nullptr;
	bool ok = true;



	cobj = (SwfManager*)tolua_tousertype(tolua_S, 1, 0);

	argc = lua_gettop(tolua_S) - 1;
	if (argc == 1)
	{
		std::string arg0;
		ok &= luaval_to_std_string(tolua_S, 2, &arg0, "SwfManager:setCachePath");
		if (!ok)
		{
			tolua_error(tolua_S, "invalid arguments in function 'lua_SwfManager_setCachePath'", nullptr);
			return 0;
		}


		auto name = cobj->getFileName(arg0);
		lua_pushstring(tolua_S, name.c_str());
		//lua_settop(tolua_S, 1);
		return 1;

	}

	luaL_error(tolua_S, "%s has wrong number of arguments: %d, was expecting %d \n", "SwfManager:genAniData", argc, 2);
	return 0;

}

static int lua_register_SwfManager(lua_State* tolua_S)
{
	tolua_usertype(tolua_S, "SwfManager");
	tolua_cclass(tolua_S, "SwfManager", "SwfManager", "cc.Node", nullptr);

	tolua_beginmodule(tolua_S, "SwfManager");
		tolua_function(tolua_S, "getInstance", lua_SwfManager_getInstance);
		tolua_function(tolua_S, "genAniData", lua_SwfManager_genAniData);
		tolua_function(tolua_S, "setCachePath", lua_SwfManager_setCachePath);
		tolua_function(tolua_S, "getFileName", lua_SwfManager_getFileName);
		
	tolua_endmodule(tolua_S);
	std::string typeName = typeid(SwfManager).name();
	g_luaType[typeName] = "SwfManager";
	g_typeCast["SwfManager"] = "SwfManager";
	return 1;
}






int register_all_swf_manual(lua_State* L)
{


	lua_register_SwfManager(L);


    return 0;
}
