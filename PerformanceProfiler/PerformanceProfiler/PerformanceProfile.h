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
typedef long long LongType;


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

	bool operator<(PPNode node)const
	{
		return  _line < node._line || _function < node._function ||_filename < node._filename ||_description < node._description;
	}
};


struct PPSection
{
	LongType _begintime;
	LongType _costtime;
	LongType _calltime;

	//如果函数是递归调用 统计开始时间存在问题  采用引用计数的方法
	LongType _reccount;

	PPSection()
		:_begintime(0)
		, _costtime(0)
		, _calltime(0)
		, _reccount(0)
	{}

	void Begin()
	{
		if (_reccount++ == 0)
		{
			_begintime = clock();
		}
		++_calltime;
	}
	void End()
	{
		if (--_reccount==0)
			_costtime = clock() - _begintime;
	}
	map<int, LongType> _begintimeMap;
	map<int, LongType> _costtimeMap;
	map<int, LongType> _calltimeMap;
	map<int, LongType> _reccountMap;
};

//保存适配器
class SaveAdapter
{
public:
	virtual void Save(const char* format, ...) = 0;
};

class ConsoleSaveAdapter :public SaveAdapter
{
public:
	virtual void Save(const char* format,...)
	{
		va_list args;
		va_start(args, format);
		vfprintf(stdout, format, args);
		va_end(args);
	}
};

class FileSaveAdapter:public SaveAdapter
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

	virtual void Save(const char* format,...)
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
//设计单例基类
//懒汉模式的单例
template<class T>
class Singleton
{
public:
	static T* GetInstance()const 
	{
		mutex mtx;
		if (_Instance == NULL)
		{
			lock_guard<mutex> lck(mtx);
			if (_Instance == NULL)
			{
				//s_instance = new S_Instance();
				_Instance* tmp = new _Instance();
				//内存栅栏
				MemoryBarrier();
				s_instance = tmp;
			}
		}
		return _Instance;
	}
protected:
	static T* _Instance;
};
template<class T>
T* Singleton<T>::_Instance = NULL;
class PerformanceProfiler:public Singleton<PerformanceProfiler>
{
public:
	PPSection* CreateSection(const char* filename, const char* function, const LongType line, const char* description);
	void Output();
protected:
	//多态的应用
	void _Output(SaveAdapter &sa);
private:
	map<PPNode, PPSection*> _ppMap;
};

#define BEGIN(sign,desc) PPSection* sign##section=PerformanceProfiler::GetInstance()->CreateSection(__FILE__,__FUNCTION__,__LINE__,desc);sign##section->Begin()
#define END(sign) sign##section->End()

struct Print
{
	~Print()
	{
		PerformanceProfiler::GetInstance()->Output();
	}
};