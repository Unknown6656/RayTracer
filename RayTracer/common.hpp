#pragma once

#include <cmath>
#include <sstream>
#include <string>
#include <ostream>
#include <algorithm>
#include <vector>
#include <tuple>


#define EPSILON 1e-6
#define INFINITY 1e7

#define OSTREAM_OPERATOR(x) friend inline std::ostream& operator<<(std::ostream& os, const x& value) { os << value.to_string(); return os; }
