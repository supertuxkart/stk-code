#include "ge_spm.hpp"
#include "ge_spm_buffer.hpp"

namespace GE
{
GESPM::GESPM()
     : m_fps(0.0f), m_bind_frame(0), m_total_joints(0), m_joint_using(0),
       m_frame_count(0)
{
}   // GESPM

// ----------------------------------------------------------------------------
GESPM::~GESPM()
{
    for (unsigned i = 0; i < m_buffer.size(); i++)
        m_buffer[i]->drop();
}   // ~GESPM

// ----------------------------------------------------------------------------
IMeshBuffer* GESPM::getMeshBuffer(u32 nr) const
{
    if (nr < m_buffer.size())
        return m_buffer[nr];
    else
        return NULL;
}   // getMeshBuffer

// ----------------------------------------------------------------------------
IMeshBuffer* GESPM::getMeshBuffer(const video::SMaterial &material) const
{
    for (unsigned i = 0; i < m_buffer.size(); i++)
    {
        if (m_buffer[i]->getMaterial() == material)
            return m_buffer[i];
    }
    return NULL;
}   // getMeshBuffer

// ----------------------------------------------------------------------------
void GESPM::finalize()
{
    m_bounding_box.reset(0.0f, 0.0f, 0.0f);
    for (unsigned i = 0; i < m_buffer.size(); i++)
        m_bounding_box.addInternalBox(m_buffer[i]->getBoundingBox());
}   // finalize

}
