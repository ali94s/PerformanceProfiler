#include"PerformanceProfile.h"

Print output; 
mutex mtx;
PPSection* PerformanceProfiler::CreateSection(const char* filename, const char* function, const LongType line, const char* description)
{
	lock_guard<mutex> lck(mtx);
	PPNode node = PPNode(filename,function,line,description);
	PPSection* sec = NULL;
	map<PPNode, PPSection*>::iterator it = _ppMap.find(node);
	if (it != _ppMap.end())
	{
		//ÒÑ¾­´æÔÚ
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
	ConsoleSaveAdapter csa;
	_Output(csa);

	FileSaveAdapter fsa("log.txt");
	_Output(fsa);
}

void PerformanceProfiler::_Output(SaveAdapter &sa)
{
	int num = 1;
	map<PPNode, PPSection*>::iterator it = _ppMap.begin();
	while (it != _ppMap.end())
	{
		sa.Save("NO%d, Desc:%s\n", num++, it->first._description.c_str());
		sa.Save("filename:%s, function:%s, line:%ld\n", it->first._filename.c_str(), it->first._function.c_str(), it->first._line);
		sa.Save("calltime:%lld, costtime:%lld\n", it->second->_calltime,it->second->_costtime);

		it++;
	}
}