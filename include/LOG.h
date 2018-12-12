/* 
    Log function. 
    Right now it either prints to cerr or does nothing. 
    Takes stringstream operations as an argument, as opposed to a formatter syntax.
*/
#include <iostream> 

#define LOG std::cerr

//#define LOG(a) //(a)