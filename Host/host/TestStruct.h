#pragma once
// dll export 함수의 인자 및 반환형으로 사용될 구조체
struct Hello
{
	double numD;
	int numI;
	Hello(double numD, int numI)
		: numD(numD), numI(numI)
	{
	}
};