#ifndef HEADER_MAC_VULKAN_HPP
#define HEADER_MAC_VULKAN_HPP

#ifdef DLOPEN_MOLTENVK
namespace irr
{
class MoltenVK
{
private:
    bool m_loaded;
    void* m_handle;
public:
    // ------------------------------------------------------------------------
    MoltenVK();
    // ------------------------------------------------------------------------
    ~MoltenVK();
    // ------------------------------------------------------------------------
    bool loaded() const                                    { return m_loaded; }
};
}

#endif

#endif
