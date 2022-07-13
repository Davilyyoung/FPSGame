// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "KismaetMultiFPSLibrary.generated.h"

/**
 * 
 */

//想扩充各种玩法 和 想有各种数据的时候可以卸载这里
//自定义结构体
//蓝图可调用
USTRUCT(BlueprintType)
struct FDeathShootPlayerData
{
	GENERATED_BODY()
	//蓝图里可写
	UPROPERTY(BlueprintReadWrite)
	FName PlayerName;
	UPROPERTY(BlueprintReadWrite)
	int PlayerScore;

	//初始化
	FDeathShootPlayerData()
	{
		PlayerName = TEXT(" ");//初始化无名字
		PlayerScore = 0;//初始化0分
	}
};


UCLASS()
class FPSGAME_API UKismaetMultiFPSLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable,Category="Sort")
	//排序
	//加static 是为了这个函数只作为一个方法 作为一个库调用 不需要作为一个类方法 类方法跟着对象来的 一个对象有一个方法
	//和对象没关系 有一个类就有一个这个方法
	//要是引用 要把传进来的值修改掉 对于结构体的数组:用UPARAM(ref)宏来告诉蓝图和C++这个参数传递采用引用来传递
	static void SortValues(UPARAM(ref)TArray<FDeathShootPlayerData>& Values); 

	//快速排序 递归方法
	//应为是递归方法 要让他随时把TArray<FDeathShootPlayerData>&返回
	static  TArray<FDeathShootPlayerData>& QuickSort(UPARAM(ref)TArray<FDeathShootPlayerData>& Values,int Start,int End);
	
};

 

