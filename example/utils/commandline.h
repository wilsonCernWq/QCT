// ======================================================================== //
// Copyright SCI Institute, University of Utah, 2018
// ======================================================================== //

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <type_traits>

namespace CommandLine 
{
  template <class T1, class T2> T1 lexical_cast(const T2& t2) {
    std::stringstream s; s << t2; T1 t1;
    if (s >> t1 && s.eof()) { return t1; }
    else {
      throw std::runtime_error("bad conversion " + s.str());
      return T1();
    }
  }
  template<int N, typename T> T Parse(const int ac, const char** av, int &i, T& v) {
    const int init = i;
    if (init + N < ac) {
      if constexpr (N == 1) {
	  v = lexical_cast<double, const char*>(av[++i]);
      } else {
	for (int k = 0; k < N; ++k) {
	  v[k] = lexical_cast<double, const char*>(av[++i]);
	}	
      }
    } else {
      throw std::runtime_error(std::to_string(N) + " values required for " + av[init]);
    }
  }
};
