#pragma once

#include "CSVParser.h"

#define GETFIELD_STRING(pos) rowFields[pos]
#define GETFIELD_INT(pos) atoi(rowFields[pos])
#define GETFIELD_UINT(pos) strtol(rowFields[pos], nullptr, 10)
#define GETFIELD_UINT64(pos) strtoll(rowFields[pos], nullptr, 10)
#define GETFIELD_FLOAT(pos) (float)atof(rowFields[pos])
