#include "script.hpp"

extern "C"
{
#include <stdio.h>
#include <stdlib.h>
}

namespace Game
{
	ScriptEngine::ScriptEngine(void)
	{
		mHeap = heap_new();
	}

	ScriptEngine::~ScriptEngine(void)
	{
		heap_free(mHeap);
	}

	struct simple_stream
	{
		FILE *file;
		int   buf;
	};

#define BUF_EMPTY (-2)

	int simple_stream_in(struct simple_stream *stream, int advance)
	{
		int r;
		if (advance)
		{
			if (stream->buf == BUF_EMPTY)
				r = fgetc(stream->file);
			else
			{
				r = stream->buf;
				stream->buf = BUF_EMPTY;
			}
		}
		else
		{
			if (stream->buf == BUF_EMPTY)
			{
				stream->buf = fgetc(stream->file);
				if (stream->buf < 0) stream->buf = -1;
			}
		
			r = stream->buf;
		}
		return r;
	}
	
	void
	ScriptEngine::LoadScript(const char *name)
	{
		struct simple_stream s;
		s.file = fopen(name, "r");
		s.buf  = BUF_EMPTY;
		
		ast_node_t n = ast_simple_parse_char_stream((stream_in_f)simple_stream_in, &s);

		fclose(s.file);
		
		n = ast_syntax_parse(n, 0);
		sematic_symref_analyse(n);

		expression_t e = expression_from_ast(mHeap, n);
		mProg = continuation_from_expression(mHeap, e);
		mEx = NULL;
	}

	int
	ScriptEngine::Execute(object_t value, std::vector<object_t> *excall)
	{
		if (mEx)
			mEx->value = value;
		int r = vm_apply(mHeap, mProg, 0, NULL, &mRet, &mEx, &mExFunc, &mExArgc, &mExArgs);
		mProg = NULL;
		
		excall->clear();
		excall->push_back(mExFunc);
		int i;
		for (i = 0; i < mExArgc; ++ i) excall->push_back(mExArgs[i]);

		return r;
	}

	object_t
	ScriptEngine::ObjectNew(void)
	{
		return heap_object_new(mHeap);
	}
	
	void
	ScriptEngine::ObjectProtect(object_t object)
	{
		return heap_protect_from_gc(mHeap, object);
	}
	
	void
	ScriptEngine::ObjectUnprotect(object_t object)
	{
		return heap_unprotect(mHeap, object);
	}

}
