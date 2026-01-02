//
//  SwiftObject.h
//  ModelIO_Bindings
//
//  Created by Taanish Reja on 1/8/25.
//

#pragma once

#include <functional>

class SwiftObject {
public:
    SwiftObject():
        m_obj{nullptr},
        m_destructor{nullptr},
        m_owns{false}
    {}
    
    SwiftObject(void* obj, std::function<void(void*)> destructor):
        m_obj{obj},
        m_destructor{destructor},
        m_owns{true}
    {}
    
    void* get();
    
    const void* get() const;
    
    bool owns() const;
    
    void retainOwnership();
    
    void releaseOwnership();
    
    void reset(void* obj, std::function<void(void*)> destructor, bool owns);
    
    void release();
    
    // delete copy constructors
    SwiftObject(const SwiftObject& other) = delete;
    
    SwiftObject& operator=(const SwiftObject& other) = delete;
    
    // enable move semantics
    SwiftObject(SwiftObject&& other);
    
    SwiftObject& operator=(SwiftObject&& other);
    
    ~SwiftObject();
private:
    void* m_obj;
    bool m_owns;
    std::function<void(void*)> m_destructor;
};
