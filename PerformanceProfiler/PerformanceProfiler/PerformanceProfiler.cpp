#include"PerformanceProfiler.h"

Print output; 




bool PerformanceProfiler::CompareByCallCount(map<PPNode, PPSection*>::iterator lhs, map<PPNode, PPSection*>::iterator rhs)
{
	return lhs->second->_Totcount > rhs->second->_Totcount;
}
bool PerformanceProfiler::CompareByCostTime(map<PPNode, PPSection*>::iterator lhs, map<PPNode, PPSection*>::iterator rhs)
{
	return lhs->second->_Tottime > rhs->second->_Tottime;
}
PPSection* PerformanceProfiler::CreateSection(const char* filename, const char* function, const LongType line, const char* description)
{
	mutex mtx;
	lock_guard<mutex> lck(mtx);
	PPNode node = PPNode(filename,function,line,description);
	PPSection* sec = NULL;
	map<PPNode, PPSection*>::iterator it = _ppMap.find(node);
	if (it != _ppMap.end())
	{
		//已经存在
		sec = it->second;
	}
	else
	{
		sec = new PPSection();
		_ppMap.insert(pair<PPNode, PPSection*>(node, sec));
	}
	return sec;
}

void PerformanceProfiler::Output()
{
	int flag = ConfigManager::GetInstance()->Getoption();
	if (flag&PPCO_SAVE_TO_CONSOLE)
	{
		ConsoleSaveAdapter csa;
		_Output(csa);
	}
	if (flag&PPCO_SAVE_TO_FILE)
	{
		FileSaveAdapter fsa("log.txt");
		_Output(fsa);
	}
}

void PerformanceProfiler::_Output(SaveAdapter &sa)
{
	int num = 1;
	vector<map<PPNode, PPSection*>::iterator> vInfos;
	map<PPNode, PPSection*>::iterator it = _ppMap.begin();

	for (; it != _ppMap.end(); it++)
	{
		vInfos.push_back(it);
	}

	int flag = ConfigManager::GetInstance()->Getoption();
	if (flag & PPCO_SAVE_BY_COST_TIME)
		sort(vInfos.begin(), vInfos.end(), CompareByCostTime);
	else if (flag & PPCO_SAVE_BY_CALL_COUNT)
		sort(vInfos.begin(), vInfos.end(), CompareByCallCount);
	for (int index = 0; index < vInfos.size(); index++)
	{
		sa.Save("NO%d. Description:%s\n", index + 1, vInfos[index]->first._description.c_str());
		vInfos[index]->first.Output(sa);
		vInfos[index]->second->Output(sa);
		sa.Save("\n");
	}
}

void PPSection::Begin(int threadId)
{
	mutex mtx;
	lock_guard<mutex> lck(mtx);
	++_calltimeMap[threadId];
	if (_reccountMap[threadId]++==0)
	{
		_begintimeMap[threadId] = clock();
	}
	_Totcount++;
	_Totrec++;
}
void PPSection::End(int threadId)
{
	mutex mtx;
	lock_guard<mutex> lck(mtx);
	auto rec = _reccountMap[threadId];
	rec--;
	_Totrec--;
	if (rec==0)
	{
		_costtimeMap[threadId] = clock() - _begintimeMap[threadId];
		_Tottime += _costtimeMap[threadId];
	}
		
}

void PPSection::Output(SaveAdapter& sa)const
{
	if (_Totrec)
		sa.Save("Performance Profiler Not Match!\n");
	auto costtimeIt = _costtimeMap.begin();
	auto calltimeIt = _calltimeMap.begin();
	for(; costtimeIt != _costtimeMap.end(); costtimeIt++)
	{
		//auto callcount = _calltimeMap[costtimeIt->first];
		sa.Save("ThreadId:%d,Cost Time:%.2f,Call Count:%d\n",
			costtimeIt->first
			, (double)costtimeIt->second / 1000
			, calltimeIt->second);
	}
	sa.Save("Total Cost Time:%.2f, Total Call Count:%d\n",
		(double)_Tottime / 1000, _Totcount);
}

//获取文件名
static string GetFileName(const string& path)
{
	char ch = '/';

#ifdef _WIN32
	ch = '\\';
#endif

	size_t pos = path.rfind(ch);
	if (pos == string::npos)
	{
		return path;
	}
	else
	{
		return path.substr(pos + 1);
	}
}
void PPNode::Output(SaveAdapter& sa) const
{
	sa.Save("FileName:%s, Fuction:%s, Line:%d\n",
		GetFileName(_filename).c_str(), _function.c_str(), _line);
}