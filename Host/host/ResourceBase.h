#pragma once

class CResourceBase
{
public:
	virtual bool Initialize() = 0;
	virtual bool Finalize() = 0;
	virtual bool Verify() = 0;
};