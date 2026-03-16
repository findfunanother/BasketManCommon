#pragma once


#define BALLDATALISTSIZE 300

class CBallCurveInfo
{
public:
    unsigned int     ownerID;
    unsigned int     targetID;
    unsigned int     ballNumber;
    int      indexFirstBound = 0;
    bool     ready;
};

