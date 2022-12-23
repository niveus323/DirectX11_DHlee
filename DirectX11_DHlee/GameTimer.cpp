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

//��ü ��� �ð� ����.
float GameTimer::GameTime() const
{
	//�ߴ� ���¿� �ִٸ� �ߴܵ� �ð��� ����x.
	//BaseTime ~ StopTime������ �ð����� ���� �ߴ� �ð��� PausedTime�� ����.
	if (mStopped)
	{
		//                     |<--paused time-->|
		// ----*---------------*-----------------*------------*------------*------> time
		//  mBaseTime       mStopTime'        startTime'    mStopTime    mCurrTime
		return (float)(((mStopTime - mPausedTime) - mBaseTime) * mSecondsPerCount);
	}
	//Ÿ�̸Ӱ� ���ư��� �ִٸ� CurrTime�� �������� ����Ѵ�.
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

//Timer ����.
void GameTimer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	mBaseTime = currTime;
	mPrevTime = currTime;    //ù �������� ���� �������� �������� �ʴ´�. PrevTime�� currTime���� �����Ͽ� DeltaTime�� Reset�� ������ �������κ��� ����ϵ��� �Ѵ�.
	mStopTime = 0;
	mStopped = false;
}

//Timer ���ۤ��ߴܵ� Timer �簳
void GameTimer::Start()
{
	//���۽ð� ����.
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	//�ߴܵ� Ÿ�̸Ӹ� �簳�ϴ� ���.
	if (mStopped)
	{
		//������ �ð��� ����.
		mPausedTime += (startTime - mStopTime);
		//PrevTime�� ����ð����� �缳��.
		mPrevTime = startTime;
		//�ߴܻ��¿� ���õ� ���� �缳��.
		mStopTime = 0;
		mStopped = false;
	}
}

//Timer �ߴ�.
void GameTimer::Stop()
{
	//�̹� �ߴܵ� �������� üũ.
	if (!mStopped)
	{
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		//�ߴܽð��� ���� �� �÷��� ����.
		mStopTime = currTime;
		mStopped = true;
	}
}

//Timer ����.
void GameTimer::Tick()
{
	if (mStopped)
	{
		mDeltaTime = 0.0;
		return;
	}
	//���� �������� �ð� �ޱ�.
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	mCurrTime = currTime;

	//���� ������ �ð����� ���� ���.
	mDeltaTime = (mCurrTime - mPrevTime) * mSecondsPerCount;

	//���� �����ӿ� ����� �ð� ���� ����.
	mPrevTime = mCurrTime;

	//���μ����� �������� ���ų� �ٸ� ���μ����� ��Ű�� ��� �������� �� �� �ִ�. - ���� ó��.
	if (mDeltaTime < 0.0)
	{
		mDeltaTime = 0.0;
	}
}