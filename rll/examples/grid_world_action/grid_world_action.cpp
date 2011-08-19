// GridWorldAction.cpp : Defines the entry point for the console application.
//

#include "grid_world.hpp"

#include <tcl/rll/lambda_method.hpp>
#include <tcl/rll/config.hpp>

using namespace tcl::rll;

int main(int argc, char* argv[]) {
  CConfigPtr ptrConfig(new CConfig);
  ptrConfig->m_enableLog = false;

  CGridWorld gw;
  CLambdaWatkins m(&gw, ptrConfig);

  m.Run(2000);
	return 0;
}
