#include "GameTimer.h"
#include <Windows.h>

GameTimer::GameTimer() : 
	mSecondsPerCount(0.0), mDeltaTime(-1.0), mBaseTime(0), mStopTime(0),
	mPausedTime(0), mPrevTime(0), mCurrTime(0), mStopped(false)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	mSecondsPerCount = 1.0 / (double)countsPerSec;
}

//전체 경과 시간 리턴.
float GameTimer::GameTime() const
{
	//중단 상태에 있다면 중단된 시간을 포함x.
	//BaseTime ~ StopTime까지의 시간에서 이전 중단 시간인 PausedTime을 제외.
	if (mStopped)
	{
		//                     |<--paused time-->|
		// ----*---------------*-----------------*------------*------------*------> time
		//  mBaseTime       mStopTime'        startTime'    mStopTime    mCurrTime
		return (float)(((mStopTime - mPausedTime) - mBaseTime) * mSecondsPerCount);
	}
	//타이머가 돌아가고 있다면 CurrTime을 기준으로 계산한다.
	else
	{
		//  (mCurrTime - mPausedTime) - mBaseTime 
		//
		//                     |<--paused time-->|
		// ----*---------------*-----------------*------------*------> time
		//  mBaseTime       mStopTime        startTime     mCurrTime
		return (float)(((mCurrTime - mPausedTime) - mBaseTime) * mSecondsPerCount);
	}
}

float GameTimer::DeltaTime() const
{
	return (float)mDeltaTime;
}

//Timer 리셋.
void GameTimer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	mBaseTime = currTime;
	mPrevTime = currTime;    //첫 프레임은 이전 프레임이 존재하지 않는다. PrevTime을 currTime으로 설정하여 DeltaTime을 Reset을 시작한 시점으로부터 계산하도록 한다.
	mStopTime = 0;
	mStopped = false;
}

//Timer 시작ㆍ중단된 Timer 재개
void GameTimer::Start()
{
	//시작시간 저장.
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	//중단된 타이머를 재개하는 경우.
	if (mStopped)
	{
		//정지한 시간을 누적.
		mPausedTime += (startTime - mStopTime);
		//PrevTime을 현재시간으로 재설정.
		mPrevTime = startTime;
		//중단상태와 관련된 변수 재설정.
		mStopTime = 0;
		mStopped = false;
	}
}

//Timer 중단.
void GameTimer::Stop()
{
	//이미 중단된 상태인지 체크.
	if (!mStopped)
	{
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		//중단시간을 저장 후 플래그 설정.
		mStopTime = currTime;
		mStopped = true;
	}
}

//Timer 진행.
void GameTimer::Tick()
{
	if (mStopped)
	{
		mDeltaTime = 0.0;
		return;
	}
	//현재 프레임의 시간 받기.
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	mCurrTime = currTime;

	//이전 프레임 시간과의 차이 계산.
	mDeltaTime = (mCurrTime - mPrevTime) * mSecondsPerCount;

	//다음 프레임에 사용할 시간 정보 저장.
	mPrevTime = mCurrTime;

	//프로세서가 절전모드로 들어가거나 다른 프로세서와 엉키는 경우 음수값이 될 수 있다. - 예외 처리.
	if (mDeltaTime < 0.0)
	{
		mDeltaTime = 0.0;
	}
}