#ifndef NETWORK_STRING_HPP
#define NETWORK_STRING_HPP

#include "utils/types.hpp"

#include <string>
#include <stdarg.h>
#include <assert.h>

typedef unsigned char uchar;

class NetworkString
{
    union { 
    float f; 
    uint8_t i[4]; 
    } f_as_i; // float as integer
    union { 
    double d; 
    uint8_t i[8]; 
    } d_as_i; // double as integer
    public:
        NetworkString() { }
        NetworkString(const uint8_t& value) { m_string = (char)(value); }
        NetworkString(NetworkString const& copy) { m_string = copy.m_string; }
        NetworkString(std::string str) { m_string = str; }
        
        NetworkString& removeFront(int size)
        {
            m_string.erase(0, size);
            return *this;
        }
        NetworkString& remove(int pos, int size)
        {
            m_string.erase(pos, size);
            return *this;
        }
        
        uint8_t operator[](const int& pos)
        {
            return getUInt8(pos);
        }
    
        NetworkString& addUInt8(const uint8_t& value)
        {
            m_string += (char)(value);
            return *this;
        }
        inline NetworkString& ai8(const uint8_t& value) { return addUInt8(value); }
        NetworkString& addUInt16(const uint16_t& value)
        {
            m_string += (char)((value<<8)&0xff);
            m_string += (char)(value&0xff);
            return *this;
        }
        inline NetworkString& ai16(const uint16_t& value) { return addUInt16(value); }
        NetworkString& addUInt32(const uint32_t& value)
        {
            m_string += (char)((value<<24)&0xff);
            m_string += (char)((value<<16)&0xff);
            m_string += (char)((value<<8)&0xff);
            m_string += (char)(value&0xff);
            return *this;
        }
        inline NetworkString& ai32(const uint32_t& value) { return addUInt32(value); }
        NetworkString& addInt(const int& value)
        {
            m_string += (char)((value<<24)&0xff);
            m_string += (char)((value<<16)&0xff);
            m_string += (char)((value<<8)&0xff);
            m_string += (char)(value&0xff);
            return *this;
        }
        inline NetworkString& ai(const int& value) { return addInt(value); }
        NetworkString& addFloat(const float& value) //!< BEWARE OF PRECISION
        {
            assert(sizeof(float)==4);
            f_as_i.f = value;
            m_string += (char)(f_as_i.i[0]);
            m_string += (char)(f_as_i.i[1]);
            m_string += (char)(f_as_i.i[2]);
            m_string += (char)(f_as_i.i[3]);
            return *this;
        }
        inline NetworkString& af(const float& value) { return addFloat(value); }
        NetworkString& addDouble(const double& value) //!< BEWARE OF PRECISION
        {
            assert(sizeof(double)==8);
            d_as_i.d = value;
            m_string += (char)(d_as_i.i[0]);
            m_string += (char)(d_as_i.i[1]);
            m_string += (char)(d_as_i.i[2]);
            m_string += (char)(d_as_i.i[3]);
            m_string += (char)(d_as_i.i[4]);
            m_string += (char)(d_as_i.i[5]);
            m_string += (char)(d_as_i.i[6]);
            m_string += (char)(d_as_i.i[7]);
            return *this;
        }
        inline NetworkString& ad(const double& value) { return addDouble(value); }
        NetworkString& operator+=(NetworkString const& value)
        {
            m_string += value.m_string;
            return *this;
        }
        
        const char* c_str() const
        {
            return m_string.c_str();
        }
        int size() const
        {
            return m_string.size();
        }
        
        template<typename T, size_t n> 
        T get(int pos)
        {
            int a = n;
            T result = 0;
            while(a--)
            {
                result <<= 8; // offset one byte
                result += ((uint8_t)(m_string[pos+n-1-a]) & 0xff); // add the data
            }
            return result;
        }
    
        inline int          getInt(int pos = 0)     { return get<int,4>(pos);             }
        inline uint32_t     getUInt(int pos = 0)    { return get<uint32_t,4>(pos);        }
        inline uint32_t     getUInt32(int pos = 0)  { return get<uint32_t,4>(pos);        }
        inline uint16_t     getUInt16(int pos = 0)  { return get<uint16_t,2>(pos);        }
        inline uint8_t      getUInt8(int pos = 0)   { return get<uint8_t,1>(pos);         }
        inline char         getChar(int pos = 0)    { return get<char,1>(pos);            }
        inline unsigned char getUChar(int pos = 0)  { return get<unsigned char,1>(pos);   }
        
        inline int          gi(int pos = 0)         { return get<int,4>(pos);             }
        inline uint32_t     gui(int pos = 0)        { return get<uint32_t,4>(pos);        }
        inline uint32_t     gui32(int pos = 0)      { return get<uint32_t,4>(pos);        }
        inline uint16_t     gui16(int pos = 0)      { return get<uint16_t,2>(pos);        }
        inline uint8_t      gui8(int pos = 0)       { return get<uint8_t,1>(pos);         }
        inline char         gc(int pos = 0)         { return get<char,1>(pos);            }
        inline unsigned char guc(int pos = 0)       { return get<unsigned char,1>(pos);   }
        
        double getDouble(int pos = 0) //!< BEWARE OF PRECISION
        {
            for (int i = 0; i < 8; i++)
                d_as_i.i[i] = m_string[pos+i];
            return d_as_i.d;
        }
        float getFloat(int pos = 0) //!< BEWARE OF PRECISION
        {
            for (int i = 0; i < 4; i++)
                f_as_i.i[i] = m_string[pos+i];
            return f_as_i.f;
        }
        
        //! Functions to get while removing
        template<typename T, size_t n> 
        T getAndRemove(int pos)
        {
            int a = n;
            T result = 0;
            while(a--)
            {
                result <<= 8; // offset one byte
                result += ((uint8_t)(m_string[pos+n-1-a]) & 0xff); // add the data
            }
            remove(pos,n);
            return result;
        }
        
        inline int          getAndRemoveInt(int pos = 0)     { return getAndRemove<int,4>(pos);             }
        inline uint32_t     getAndRemoveUInt(int pos = 0)    { return getAndRemove<uint32_t,4>(pos);        }
        inline uint32_t     getAndRemoveUInt32(int pos = 0)  { return getAndRemove<uint32_t,4>(pos);        }
        inline uint16_t     getAndRemoveUInt16(int pos = 0)  { return getAndRemove<uint16_t,2>(pos);        }
        inline uint8_t      getAndRemoveUInt8(int pos = 0)   { return getAndRemove<uint8_t,1>(pos);         }
        inline char         getAndRemoveChar(int pos = 0)    { return getAndRemove<char,1>(pos);            }
        inline unsigned char getAndRemoveUChar(int pos = 0)  { return getAndRemove<unsigned char,1>(pos);   }
        double getAndRemoveDouble(int pos = 0) //!< BEWARE OF PRECISION
        {
            for (int i = 0; i < 8; i++)
                d_as_i.i[i] = m_string[pos+i];
            return d_as_i.d;
            remove(pos, 8);
        }
        float getAndRemoveFloat(int pos = 0) //!< BEWARE OF PRECISION
        {
            for (int i = 0; i < 4; i++)
                f_as_i.i[i] = m_string[pos+i];
            return f_as_i.f;
            remove(pos, 4);
        }
        
        inline NetworkString& gui8(uint8_t* dst)   { *dst = getAndRemoveUInt8(0);  return *this; }
        inline NetworkString& gui16(uint16_t* dst) { *dst = getAndRemoveUInt16(0); return *this; }
        inline NetworkString& gui32(uint32_t* dst) { *dst = getAndRemoveUInt32(0); return *this; }
        inline NetworkString& gui(uint32_t* dst)   { *dst = getAndRemoveUInt32(0); return *this; }
        inline NetworkString& gi(int* dst)         { *dst = getAndRemoveInt(0);    return *this; }
        inline NetworkString& gc(char* dst)        { *dst = getAndRemoveChar(0);   return *this; }
        inline NetworkString& guc(uchar* dst)      { *dst = getAndRemoveUChar(0);  return *this; }
        inline NetworkString& gd(double* dst)      { *dst = getAndRemoveDouble(0); return *this; }
        inline NetworkString& gf(float* dst)       { *dst = getAndRemoveFloat(0);  return *this; }
    
    protected:
        std::string m_string;
};

NetworkString operator+(NetworkString const& a, NetworkString const& b);

#endif // NETWORK_STRING_HPP
