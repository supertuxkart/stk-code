#ifndef HEADER_GE_COMPRESSOR_S3TC_BC3_HPP
#define HEADER_GE_COMPRESSOR_S3TC_BC3_HPP

#include "ge_mipmap_generator.hpp"

namespace GE
{
class GECompressorS3TCBC3 : public GEMipmap
{
private:
    uint8_t* m_compressed_data;
public:
    // ------------------------------------------------------------------------
    GECompressorS3TCBC3(GEMipmap *mipmap);
    // ------------------------------------------------------------------------
    ~GECompressorS3TCBC3()                     { delete [] m_compressed_data; }
    // ------------------------------------------------------------------------
    bool isRawData()                                          { return false; }
};   // GECompressorS3TCBC3

}

#endif
