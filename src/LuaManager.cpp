#include "Global.h"
#include "GameState.h"

#include "LuaManager.h"

int LuaPanic(lua_State* State)
{
	if (lua_isstring(State, -1))
		wprintf(L"LUA ERROR: %ls\n", Utility::Widen(lua_tostring(State, -1)).c_str());
	Utility::DebugBreak();
	return 0;
}

int Break(lua_State *S)
{
	Utility::DebugBreak();
	return 0;
}

int DoGameScript(lua_State *S)
{
	LuaManager* Lua = GetObjectFromState<LuaManager>(S, "Luaman");
	GString File = luaL_checkstring(S, 1);
	lua_pushnumber(S, Lua->RunScript(GameState::GetInstance().GetScriptsDirectory() + File));
	return 1;
}

LuaManager::LuaManager()
{
	State = luaL_newstate();
	if (State)
	{
		// luaL_openlibs(State);
		RegisterStruct("Luaman", (void*)this);
		Register(Break, "DEBUGBREAK");
		Register(DoGameScript, "game_require");
		luaL_openlibs(State);
		lua_atpanic(State, &LuaPanic);
	}
	// If we couldn't open lua, can we throw an exception?
}

LuaManager::~LuaManager()
{
	if (State)
		lua_close(State);
}

void LuaManager::GetGlobal(GString VarName)
{
	lua_getglobal(State, VarName.c_str());
}

bool LuaManager::RunScript(Directory file)
{
	return RunScript(file.path());
}

bool LuaManager::RunScript(GString Filename)
{
	int errload = 0, errcall = 0;

	if (!Filename.length())
		return false;

	if( (errload = luaL_loadfile(State, Filename.c_str())) || (errcall = lua_pcall(State, 0, LUA_MULTRET, 0)))
	{
		std::wstring reason = Utility::Widen(lua_tostring(State, -1));

		wprintf(L"Lua error: %s\n", reason.c_str());
		return false;
	}
	return true;
}

bool LuaManager::RunGString(GString string)
{
	int errload = 0, errcall = 0;
	
	if ( (errload = luaL_loadstring(State, string.c_str())) || (errcall = lua_pcall(State, 0, LUA_MULTRET, 0)) )
	{
		GString reason = lua_tostring(State, -1);
		return false;
	}
	return true;
}

bool LuaManager::IsValid()
{
	return State != NULL;
}

bool LuaManager::Register(lua_CFunction Function, GString FunctionName)
{
	if (!Function || FunctionName.empty())
		return false;
	lua_register(State, FunctionName.c_str(), Function);
	return true;
}

int LuaManager::GetGlobalI(GString VariableName, int Default)
{
	int rval = Default;
	
	GetGlobal(VariableName);
	
	if (lua_isnumber(State, -1))
	{
		rval = lua_tonumber(State, -1);
	}else
	{ 
		// Pop();
		// throw LuaTypeException(VariableName, "int");
	}

	Pop();
	return rval;
}

GString LuaManager::GetGlobalS(GString VariableName, GString Default)
{
	GString rval = Default;
	
	GetGlobal(VariableName);
	
	if (!lua_isnil(State, -1) && lua_isstring(State, -1))
	{
		const char* s = lua_tostring(State, -1);
		rval = s ? s : "";
	}else
	{
		// Pop();
		// throw LuaTypeException(VariableName, "GString");
	}

	Pop();
	return rval;
}

double LuaManager::GetGlobalD(GString VariableName, double Default)
{
	double rval = Default;
	GetGlobal(VariableName);
	if (lua_isnumber(State, -1))
	{
		rval = lua_tonumber(State, -1);
	}else
	{ 
		// Pop();
		// throw LuaTypeException(VariableName, "double");
	}

	Pop();
	return rval;
}

void LuaManager::SetGlobal(GString VariableName, GString Value)
{
	lua_pushstring(State, Value.c_str());
	lua_setglobal(State, VariableName.c_str());
}

void LuaManager::SetGlobal(GString VariableName, double Value)
{
	lua_pushnumber(State, Value);
	lua_setglobal(State, VariableName.c_str());
}

bool LuaManager::RegisterStruct(GString Key, void* data, GString MetatableName)
{
	if (!data) return false;
	if (Key.length() < 1) return false;

	lua_pushstring(State, Key.c_str());
	lua_pushlightuserdata(State, data);

	if (MetatableName.length())
	{
		luaL_getmetatable(State, MetatableName.c_str());
		lua_setmetatable(State, -2);
	}

	lua_settable(State, LUA_REGISTRYINDEX);
	return true;
}

void* LuaManager::GetStruct(GString Key)
{
	lua_pushstring(State, Key.c_str());
	lua_gettable(State, LUA_REGISTRYINDEX);
	return lua_touserdata(State, -1); // returns null if does not exist
}

void LuaManager::NewArray()
{
	lua_newtable(State);
}

bool LuaManager::UseArray(GString VariableName)
{
	GetGlobal(VariableName);
	return lua_istable(State, -1);
}

void LuaManager::SetFieldI(int index, int Value)
{
	lua_pushnumber(State, Value);
	lua_rawseti(State, -2, index);
}

void LuaManager::SetFieldS(int index, GString Value)
{
	lua_pushstring(State, Value.c_str());
	lua_rawseti(State, -2, index);
}

void LuaManager::SetFieldD(int index, double Value)
{
	lua_pushnumber(State, Value);
	lua_rawseti(State, -2, index);
}

int LuaManager::GetFieldI(GString Key, int Default)
{
	int R = Default;
	lua_pushstring(State, Key.c_str());
	lua_gettable(State, -2);

	if (lua_isnumber(State, -1))
	{
		R = lua_tonumber(State, -1);
	}// else Error
	
	Pop();
	return R;
}

double LuaManager::GetFieldD(GString Key, double Default)
{
	double R = Default;

	if (lua_istable(State, -1))
	{
		lua_pushstring(State, Key.c_str());
		lua_gettable(State, -2);

		if (lua_isnumber(State, -1))
		{
			R = lua_tonumber(State, -1);
		}// else Error

		Pop();
		return R;
	}else
		return R;
}

GString LuaManager::GetFieldS(GString Key, GString Default)
{
	GString R = Default;

	lua_pushstring(State, Key.c_str());
	lua_gettable(State, -2);

	if (lua_isstring(State, -1))
	{
		R = lua_tostring(State, -1);
	}// else Error

	Pop();
	return R;
}

void LuaManager::Pop()
{
	lua_pop(State, 1);
}

void LuaManager::FinalizeArray(GString ArrayName)
{
	lua_setglobal(State, ArrayName.c_str());
}

void LuaManager::PushArgument(int Value)
{
	if(func_input)
		lua_pushnumber(State, Value);
}

void LuaManager::PushArgument(double Value)
{
	if(func_input)
		lua_pushnumber(State, Value);
}

void LuaManager::PushArgument(GString Value)
{
	if(func_input)
		lua_pushstring(State, Value.c_str());
}

bool LuaManager::CallFunction(const char* Name, int Arguments, int Results)
{
	bool IsFunc;

	func_args = Arguments;
	func_results = Results;
	lua_getglobal(State, Name);

	IsFunc = lua_isfunction(State, -1);

	if (IsFunc)
		func_input = true;
	else
		Pop();

	return IsFunc;
}

bool LuaManager::RunFunction()
{
	func_input = false;
	int errc = lua_pcall(State, func_args, func_results, 0);

	if (errc)
	{
		GString reason = lua_tostring(State, -1);

#ifndef WIN32
		printf("lua call error: %s\n", reason.c_str());
#else
		wprintf(L"lua call error: %s\n", Utility::Widen(reason).c_str());
#endif
		func_err = true;
		return false;
	}

	func_err = false;
	return true;
}

int LuaManager::GetFunctionResult(int StackPos)
{
	return GetFunctionResultF(StackPos);
}

float LuaManager::GetFunctionResultF(int StackPos)
{
	float Value = -1;
	
	if (func_err) return 0;

	if (lua_isnumber(State, -StackPos))
	{
		Value = lua_tonumber(State, -StackPos);

		// Make sure you're getting what you want.
		Pop();
	}

	return Value;
}

void LuaManager::NewMetatable(GString MtName)
{
	luaL_newmetatable(State, MtName.c_str());
}

void LuaManager::RegisterLibrary(GString Libname, const luaL_Reg *Reg)
{
	luaL_newlib(State, Reg);
	lua_setglobal(State, Libname.c_str());
}

lua_State* LuaManager::GetState()
{
	return State;
}

void LuaManager::StartIteration ()
{
	lua_pushnil(State);
}

bool LuaManager::IterateNext()
{
	return lua_next(State, -2) != 0;
}

int LuaManager::NextInt()
{
	return lua_tonumber(State, -1);
}

double LuaManager::NextDouble()
{
	return lua_tonumber(State, -1);
}

GString LuaManager::NextGString()
{
	return lua_tostring(State, -1);
}
