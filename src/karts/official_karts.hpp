#ifndef HEADER_OFFICIAL_KARTS_HPP
#define HEADER_OFFICIAL_KARTS_HPP

#include <string>
#include <set>

class KartProperties;
class Vec3;

namespace OfficialKarts
{
void dumpOfficialKarts();
void load();
std::set<std::string> getOfficialKarts();
const KartProperties* getKartByIdent(const std::string& ident,
                                     float* width, float* height,
                                     float* length, Vec3* gravity_shift);
}

#endif
