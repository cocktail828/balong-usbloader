#ifndef __TYPES_H__
#define __TYPES_H__

#include <iostream>
#include <vector>
#include <string>
#include <string.h>

#include "log.h"

typedef uint32_t be32_t;
typedef uint16_t be16_t;
typedef uint32_t le32_t;

using std::vector;
using std::string;
using std::ifstream;
using std::ios;
using std::istringstream;
using std::ostringstream;
using std::stringstream;
using std::hex;
using std::dec;
using std::cout;
using std::endl;
using LOGGER::LOG;
using LOGGER::Severity;

#endif //__TYPES_H__
