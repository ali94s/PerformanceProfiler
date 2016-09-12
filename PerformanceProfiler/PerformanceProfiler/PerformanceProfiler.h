#pragma once
#include<iostream>
using namespace std;
#include<map>
#include<windows.h>
#include<time.h>
#include<assert.h>
#include<stdlib.h>
#include<stdio.h>
#include<thread>
#include <mutex> 
#include<vector>
#include <algorithm>
typedef long long LongType;





//保存适配器
class SaveAdapter
{
public:
	virtual void Save(const char* format, ...) = 0;
};

class ConsoleSaveAdapter :public SaveAdapter
{
public:
	virtual void Save(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		vfprintf(stdout, format, args);
		va_end(args);
	}
};

class FileSaveAdapter :public SaveAdapter
{
public:
	FileSaveAdapter(const char* filename)
	{
		_fout = fopen(filename, "w");
		assert(_fout);
	}

	~FileSaveAdapter()
	{
		if (_fout)
			fclose(_fout);
	}

	virtual void Save(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		vfprintf(_fout, format, args);
		va_end(args);
	}
protected:
	FileSaveAdapter(const FileSaveAdapter&);
	FileSaveAdapter& operator=(const FileSaveAdapter&);
protected:
	FILE* _fout;
};

static int GetThreadId()
{
#ifdef _WIN32
	return ::GetCurrentThreadId();
#else
	return ::thread_self();
#endif
}



struct PPNode
{
	string _filename;
	string _function;
	LongType _line;
	string _description;

	PPNode(const string filename, const string function, const LongType line, const string description)
		:_filename(filename)
		,_function(function)
		, _line(line)
		, _description(description)
	{}

	bool operator<(PPNode p)const
	{
		if (_line > p._line)
			return false;

		if (_line < p._line)
			return true;

		if (_filename > p._filename)
			return false;

		if (_filename < p._filename)
			return true;

		if (_function > p._function)
			return false;

		if (_function < p._function)
			return true;

		return false;
	}
	void Output(SaveAdapter& sa)const;
};


struct PPSection
{
	LongType _Tottime;
	LongType _Totcount;
	LongType _Totrec;


	PPSection()
		:_Tottime(0)
		, _Totcount(0)
		, _Totrec(0)
	{}
	void Begin(int threadId);
	void End(int threadId);
	void Output(SaveAdapter& sa)const;
	map<int, LongType> _begintimeMap;
	map<int, LongType> _costtimeMap;
	map<int, LongType> _calltimeMap;
	map<int, LongType> _reccountMap;
};




//设计单例基类
//懒汉模式的单例
template<class T>
class Singleton
{
public:
	static T* GetInstance()
	{
		
		if (_Instance == NULL)
		{
			lock_guard<mutex> lck(_mutex);
			if (_Instance == NULL)
			{
				//s_instance = new S_Instance();
				T* tmp = new T;
				//内存栅栏
				MemoryBarrier();
				_Instance = tmp;
			}
		}
		return _Instance;
	}

protected:
	static T* _Instance;
	static mutex _mutex;
};
template<class T>
T* Singleton<T>::_Instance = NULL;

template<class T>
mutex Singleton<T>::_mutex;

class PerformanceProfiler:public Singleton<PerformanceProfiler>
{
public:
	PPSection* CreateSection(const char* filename, const char* function, const LongType line, const char* description);
	void Output();
	static bool CompareByCallCount(map<PPNode, PPSection*>::iterator lhs,map<PPNode, PPSection*>::iterator rhs);
	static bool CompareByCostTime(map<PPNode, PPSection*>::iterator lhs,map<PPNode, PPSection*>::iterator rhs);
protected:
	//多态的应用
	void _Output(SaveAdapter &sa);
private:
	map<PPNode, PPSection*> _ppMap;
};




enum PP_CONFIG_OPTION
{
	PPCO_NONE = 0,					// 不做剖析
	PPCO_PROFILER = 2,				// 开启剖析
	PPCO_SAVE_TO_CONSOLE = 4,		// 保存到控制台
	PPCO_SAVE_TO_FILE = 8,			// 保存到文件
	PPCO_SAVE_BY_CALL_COUNT = 16,	// 按调用次数降序保存
	PPCO_SAVE_BY_COST_TIME = 32,	// 按调用花费时间降序保存
};


//配置管理
class ConfigManager :public Singleton<ConfigManager>
{
public:
	ConfigManager() :_flag(PPCO_NONE)
	{}
	void Setoption(int flag)
	{
		_flag |= flag;
	}
	int Getoption()
	{
		return _flag;
	}
private:
	int _flag;
};



//RAII机制
struct Print
{
	~Print()
	{
		PerformanceProfiler::GetInstance()->Output();
	}
};

#define BEGIN(sign,desc) PPSection* sign##section=PerformanceProfiler::GetInstance()->CreateSection(__FILE__,__FUNCTION__,__LINE__,desc);\
	sign##section->Begin(GetThreadId())
#define END(sign) sign##section->End(GetThreadId())

#define SET_PERFORMANCE_PROFILER_OPTIONS(flag)		\
	ConfigManager::GetInstance()->Setoption(flag)
