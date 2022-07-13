#include "KismaetMultiFPSLibrary.h"

//排序方法
void UKismaetMultiFPSLibrary::SortValues(TArray<FDeathShootPlayerData>& Values)
{
	//排序方法
	//UE自带的方法 传过去一个lamda表达式 把排序规则传进去
	//Values.Sort([](const FDeathShootPlayerData& a ,const FDeathShootPlayerData& b){return a.PlayerScore > b.PlayerScore;});
	//                                数组长度减1末尾位置
	QuickSort(Values,0,Values.Num()-1);
}

TArray<FDeathShootPlayerData>& UKismaetMultiFPSLibrary::QuickSort(TArray<FDeathShootPlayerData>& Values, int Start,
	int End)
{
	//一开始Start位置和End位置一样就return不用排了
	//终止条件
	if (Start >= End)
	{
		return Values;
	}
	//如果没过终止
	int i = Start;//位置随时要变
	int j = End;//位置随时要变

	FDeathShootPlayerData Temp = Values[Start];//存i(Start)位置的东西
	//1次递归
	while (i != j)//一直做
	{
		//第一次
		while (j > i && Values[j].PlayerScore <= Temp.PlayerScore)
		{
			j--; 
		}
		//出来就找到大于的了 
		Values[i] = Values[j];

		//第二次
		while (j > i && Values[i].PlayerScore >= Temp.PlayerScore)
		{
		 	i++; 
		}
		//出来就找到大于的了 然后
		Values[j] = Values[i];
		
	}
	Values[i] = Temp;
	QuickSort(Values,Start,i-1);
	QuickSort(Values,i+1,End);

	return Values;
	
}
