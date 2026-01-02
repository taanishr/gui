namespace AppKit_Extensions {
    extern "C" void setTitleBarTransparent(void* windowPtr);
    extern "C" void setWindowTransparent(void* windowPtr);
    extern "C" void setMaximumDrawableCount(void* viewPtr, int count);
    extern "C" void setSyncEnabled(void* viewPtr, bool enabled);
}