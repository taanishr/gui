namespace MTK_Extensions {
    extern "C" void* createMTKTextureLoader(void* devicePtr);
    extern "C" void* releaseMTKTextureLoader(void* loaderPtr);
    extern "C" void* createTexture(void* loaderPtr, const char* filePath);
};