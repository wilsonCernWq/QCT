#pragma once

#include <iostream>
#include <set>

namespace WarmT {
  namespace Error {
    void WarnAlways(std::string str)
    {
      std::cerr << "\033[1;33m"
		<< "[Warning] " << str << "\033[0m" << std::endl;
    }
    void WarnOnce(std::string str)
    {
      static std::set<std::string> warned;
      if (warned.find(str) == warned.end()) {
	WarnAlways(str);
	warned.insert(str);
      }
    }
    void ErrorNoExit(std::string str)
    {
      std::cerr << "\033[1;31m"
		<< "[Error] " << str << "\033[0m" << std::endl;
    }
    void ErrorFatal(std::string str)
    {
      ErrorNoExit(str);
      exit(EXIT_FAILURE);
    }
  };

};
