// GridWorldState.cpp : Defines the entry point for the console application.
//
#include "grid_world.hpp"

#include <tcl/rll/lambda_method.hpp>

using namespace tcl::rll;

int main(int argc, char* argv[]) 
{
    CConfigPtr ptrConfig(new CConfig);
    ptrConfig->m_enableLog = false;
    ptrConfig->m_gamma = 1.0;

    CGridWorld gw;
    CLambdaTD m(&gw, ptrConfig);

    m.run(3000);
    gw.PrintValueFunc();
    return 0;
}

