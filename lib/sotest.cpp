// g++ -std=c++1y -fPIC -g -c sotest.cpp
// gcc -shared -o libsotest.so sotest.o -lc

#include <cmath>
#include "valuevar.hpp"
#include "env.hpp"


extern shared_ptr<BaseFrame> global;

val::Value _cos2(const vector<pair<string, val::Value>>& v) {
  return cos(cos(get<double>(v[0].second))); // just to check!
}

val::VBuiltinG g(global, "cos2", "function (x) NULL\n", _cos2);  
