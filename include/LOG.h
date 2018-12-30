/* 
    Log function. 
    Right now it either prints to cerr or does nothing. 
    Takes stringstream operations as an argument, as opposed to a formatter syntax.
*/
#include <iostream> 

// This logger is very stupid. I'll change this eventually.
#define LOG std::cerr

//#define LOG(a) //(a)