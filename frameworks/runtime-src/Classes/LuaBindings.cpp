#include "LuaBindings.h"
#include "cocos2d.h"
#include "base/ccUtils.h"
#include "2d/CCDynamicAtlasManager.h"

#include "spine/spine-cocos2dx.h"
#include "scripting/lua-bindings/manual/tolua_fix.h"
#include "ui/WidgetExport.h"

using namespace cocos2d;
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
#include "zlib.h"
#else
#include "zlib.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif
#include "tolua++.h"

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
extern "C" {
#endif

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#define luaL_newlib(L ,reg) luaL_register(L,"projectx",reg)

	//c call lua functions




static int executeFunction(lua_State* L,int numArgs)
	{
		int functionIndex = -(numArgs + 1);
		if (!lua_isfunction(L, functionIndex))
		{
			CCLOG("value at stack [%d] is not function", functionIndex);
			lua_pop(L, numArgs + 1); // remove function and arguments
			return 0;
		}

		int traceback = 0;
		lua_getglobal(L, "__G__TRACKBACK__");                         /* L: ... func arg1 arg2 ... G */
		if (!lua_isfunction(L, -1))
		{
			lua_pop(L, 1);                                            /* L: ... func arg1 arg2 ... */
		}
		else
		{
			lua_insert(L, functionIndex - 1);                         /* L: ... G func arg1 arg2 ... */
			traceback = functionIndex - 1;
		}

		int error = 0;
	
		error = lua_pcall(L, numArgs, 1, traceback);                  /* L: ... [G] ret */
		
		if (error)
		{
			if (traceback == 0)
			{
				CCLOG("[LUA ERROR] %s", lua_tostring(L, -1));        /* L: ... error */
				lua_pop(L, 1); // remove error message from stack
			}
			else                                                            /* L: ... G error */
			{
				lua_pop(L, 2); // remove __G__TRACKBACK__ and error message from stack
			}
			return 0;
		}

		// get return value
		int ret = 0;
		if (lua_isnumber(L, -1))
		{
			ret = (int)lua_tointeger(L, -1);
		}
		else if (lua_isboolean(L, -1))
		{
			ret = (int)lua_toboolean(L, -1);
		}
		// remove return value from stack
		lua_pop(L, 1);                                                /* L: ... [G] */

		if (traceback)
		{
			lua_pop(L, 1); // remove __G__TRACKBACK__ from stack      /* L: ... */
		}

		return ret;
	}




	void ccl_appPause(lua_State* L)
	{
		lua_getglobal(L, "NativeCall");

		if (!lua_istable(L, -1))
		{
			CCLOG("[LUA ERROR] name '%s' does not represent a Lua table", "NativeCall");
			lua_pop(L, 1);
			return ;
		}
		lua_getfield(L, -1, "ccl_appPause");

		executeFunction(L,0);
	}

	void ccl_appResume(lua_State* L)
	{

		lua_getglobal(L, "NativeCall");

		if (!lua_istable(L, -1))
		{
			CCLOG("[LUA ERROR] name '%s' does not represent a Lua table", "NativeCall");
			lua_pop(L, 1);
			return;
		}
		lua_getfield(L, -1, "ccl_appResume");

		executeFunction(L, 0);
	}






	void ccl_socketEvent(lua_State *L, int nNetSocketId,int eventId, const char * pStr)
	{		
		CCLOG("ccl_socketEvent %d %d", nNetSocketId, eventId);
		lua_getglobal(L, "NativeCall");

		if (!lua_istable(L, -1))
		{
			CCLOG("[LUA ERROR] name '%s' does not represent a Lua table", "NativeCall");
			lua_pop(L, 1);
			return;
		}
		lua_getfield(L, -1, "ccl_socketEvent");


		
		lua_pushinteger(L, nNetSocketId);

		lua_pushinteger(L, eventId);

		lua_pushstring(L, pStr);

		executeFunction(L, 3);
	}



	void ccl_recvMsgFromServer(lua_State *L, int nNetSocketId, int msgType, int msgSize,const char * pStr)
	{
		CCLOG("lcc_recvMsgFromServer");
		lua_getglobal(L, "NativeCall");

		if (!lua_istable(L, -1))
		{
		}
		lua_getfield(L, -1, "ccl_recvMsgFromServer");



		lua_pushinteger(L, nNetSocketId);

		lua_pushinteger(L, msgType);

		lua_pushinteger(L, msgSize);

		lua_pushlstring(L, pStr, msgSize);

		executeFunction(L, 4);
	}



	void ccl_systemCallLuaEvent(lua_State* L, int nKey, const char* sJsonData)
	{


		CCLOG("ccl_systemCallLuaEvent");
		lua_getglobal(L, "NativeCall");

		if (!lua_istable(L, -1))
		{
		}
		lua_getfield(L, -1, "ccl_systemCallLuaEvent");



		lua_pushinteger(L, nKey);

		lua_pushstring(L, sJsonData);


		executeFunction(L, 2);
	}



	

	static int lcc_getmillisecond(lua_State *L) {


		auto millisecond = cocos2d::utils::getTimeInMilliseconds();
		lua_pushinteger(L, millisecond);

		return 1;
	}


	static int lcc_addNodeToDynamicAtlas(lua_State *L) {

		int argc = lua_gettop(L);
		Node* pNode = (Node*)tolua_tousertype(L, 1, 0);
		DynamicAtlasManager::getInstance()->addNodeToDynamicAtlas(pNode);

		return 0;
	}



	int luaopen_projectx_c(lua_State *L)
	{
		luaL_Reg reg[] = {
		{ "lcc_getmillisecond" , lcc_getmillisecond },
		{ "lcc_addNodeToDynamicAtlas" , lcc_addNodeToDynamicAtlas },
		{ NULL,NULL },
		};

		//luaL_checkversion(L);
		luaL_newlib(L, reg);

		return 1;
	}



#ifdef __cplusplus
}
#endif


