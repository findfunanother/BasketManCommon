// // dllmain.cpp : DLL 애플리케이션의 진입점을 정의합니다.

#include "pch.h"
// #include "AnimationInformation_generated.h"
// #include <iostream> // C++ header file for printing
// #include <fstream>
// #include <string>
// #include <windows.h>
// #include <map>
// #define MAX_PATH 256

// using namespace TB;
// using namespace std;
// bool ReadFile(std::string filePath, unsigned char** _data, int* datalen)
// {
	// std::ifstream is(filePath, std::ifstream::binary);
	// if (is) {
		// // seekg를 이용한 파일 크기 추출
		// is.seekg(0, is.end);
		// int length = (int)is.tellg();
		// is.seekg(0, is.beg);

		// // malloc으로 메모리 할당
		// unsigned char* buffer = (unsigned char*)malloc(length);

		// // read data as a block:
		// is.read((char*)buffer, length);
		// is.close();
		// *_data = buffer;
		// *datalen = length;
	// }

	// return true;
// }
// std::string GetCurrentDirectoryStr()
// {
	// char buffer[MAX_PATH];
	// GetModuleFileNameA(NULL, buffer, MAX_PATH);
	// std::string::size_type pos = std::string(buffer).find_last_of("\\/");

	// return std::string(buffer).substr(0, pos);
// }
// void main()
// {
	// unsigned char* _data = NULL;
	// int datalen = 0;
	// ReadFile(GetCurrentDirectoryStr()+"\\AnimData.bin", &_data , &datalen);
	
	// auto monster = flatbuffers::GetRoot<TB::TBAnimationInformationList>(_data);
	// auto datas = monster->datas();
	// std::map<string,const TB::TBAnimationInformation*> animinformation;
	
	// for (size_t i = 0; i < datas->Length(); i++)
	// {
		// auto data = datas->Get(i);
		// animinformation.emplace(pair<string,const TB::TBAnimationInformation*>(data->name()->str(), data));
		
	// }
	
// }