#ifndef __SCRIPT_HPP__
#define __SCRIPT_HPP__

#include <vector>

extern "C" {
#include <see/object.h>
#include <see/as/simple_parse.h>
#include <see/as/syntax_parse.h>
#include <see/vm/symref.h>
#include <see/vm/io.h>
#include <see/vm/vm.h>
}

namespace Game
{
	class ScriptEngine
	{
		heap_t mHeap;
		
		execution_t mEx;
		object_t  mProg;
		object_t  mExFunc;
		int       mExArgc;
		object_t *mExArgs;
		object_t  mRet;
		
	public:
		ScriptEngine(void);
		void LoadScript(const char *name);
		int Execute(object_t value, std::vector<object_t> *excall);
		
		object_t ObjectNew(void);
		void     ObjectProtect(object_t object);
		void     ObjectUnprotect(object_t object);

		~ScriptEngine(void);
	};
}

#endif
