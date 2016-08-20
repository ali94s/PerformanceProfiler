#include"PerformanceProfile.h"
#include<iostream>
//#include<time.h>
//#include<windows.h>
//using namespace std;


//全局变量  统计时间和调用次数
//PerformanceProfiler pro;

//void PPtest()
//{
//	PerformanceProfiler *pro = PerformanceProfiler::GetInstance();
//	PPSection* s1 = pro->CreateSection(__FILE__, __FUNCTION__, __LINE__, "数据库");
//	s1->Begin();
//	Sleep(500);
//	s1->End();
//
//	PPSection* s2 = pro->CreateSection(__FILE__, __FUNCTION__, __LINE__, "网络");
//	s2->Begin();
//	Sleep(1000);
//	s2->End();
//
//	pro->Output();
//}
LongType Fib(int n)
{
	LongType ret = 0;
	BEGIN(recursion, "网络");
	if (n < 2)
	{
		END(recursion);
		return n;
	}
	else
		ret = Fib(n - 1) + Fib(n - 2);
	END(recursion);
	return ret;
	
}
//
//
void TestFib()
{
	BEGIN(recursion, "网络");
	Fib(20);
	END(recursion);
}
void Pthread(int n)
{
	BEGIN(pthread, "网络");
	Sleep(n*1000);
	END(pthread);
}

void TestPthread()
{
	BEGIN(pthread, "网络");
	thread t1(Pthread, 1);
	thread t2(Pthread, 2);
	thread t3(Pthread, 3);

	t1.join();
	t2.join();
	t3.join();
	END(pthread);
}
int main()
{
	//TestFib();
	BEGIN(pthread,"网络");
	TestPthread();
	END(pthread);
	//PPtest();
	//cout << Fib(0) << endl;
	//cout << Fib(1) << endl;
	//cout << Fib(2) << endl;
	//cout << Fib(4) << endl;
	//system("pause");
	return 0;
}