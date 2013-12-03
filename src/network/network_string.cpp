#include "network/network_string.hpp"

NetworkString operator+(NetworkString const& a, NetworkString const& b)
{
    NetworkString ns(a);
    ns += b;
    return ns;
}
