// GridWorldAction.cpp : Defines the entry point for the console application.
//

#include "grid_world.hpp"

#include <tcl/rll/lambda_method.hpp>
#include <tcl/rll/config.hpp>

using namespace tcl::rll;

int main(int argc, char* argv[]) 
{
    CConfigPtr ptrConfig(new CConfig);
    ptrConfig->m_enableLog = false;
    ptrConfig->m_gamma = 1.0;
    ptrConfig->m_accumulating = false;

    CGridWorld gw;
    CLambdaSarsa m(&gw, ptrConfig);

    m.run(2000);
    return 0;
}
