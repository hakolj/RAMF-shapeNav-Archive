#include <stdexcept>
#include <string.h>
template <typename T>
void throwException(std::string errmsg) {
    std::cout << errmsg << std::endl;
    throw T(errmsg);
}