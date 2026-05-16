//
//  SwiftObject.cpp
//  ModelIO_Bindings
//
//  Created by Taanish Reja on 1/8/25.
//

#include "swift_object.hpp"

void* SwiftObject::get()
{
    return m_obj;
}

const void* SwiftObject::get() const
{
    return m_obj;
}

    
bool SwiftObject::owns() const
{
    return m_owns;
}
    
void SwiftObject::retainOwnership()
{
    m_owns = true;
}
    
void SwiftObject::releaseOwnership()
{
    m_owns = false;
}
    
void SwiftObject::reset(void* obj, std::function<void(void*)> destructor, bool owns)
{
    release();
    m_obj = obj;
    m_destructor = destructor;
    m_owns = owns;
}
    
void SwiftObject::release()
{
    if ((m_obj && m_owns) && m_destructor) {
        m_destructor(m_obj);
    }
    m_obj = nullptr;
    m_owns = false;
}

SwiftObject::SwiftObject(SwiftObject&& other):
    m_obj{other.m_obj},
    m_owns{other.m_owns},
    m_destructor{std::move(other.m_destructor)}
{
    other.m_obj = nullptr;
    other.m_destructor = nullptr;
    other.m_owns = false;
}
    
SwiftObject& SwiftObject::operator=(SwiftObject&& other)
{
    if (this != &other) {
        release();
        m_obj = other.m_obj;
        m_owns = other.m_owns;
        m_destructor = std::move(other.m_destructor);
        other.m_obj = nullptr;
        other.m_destructor = nullptr;
        other.m_owns = false;
    }
    return *this;
}
    
SwiftObject::~SwiftObject()
{
    release();
}

