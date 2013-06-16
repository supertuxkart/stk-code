#ifndef HTTP_FUNCTIONS_HPP
#define HTTP_FUNCTIONS_HPP

#include <string>

namespace HTTP
{

void init();
void shutdown();

std::string getPage(std::string url);


}

#endif // HTTP_FUNCTIONS_HPP
