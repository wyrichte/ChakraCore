
bool IsOs_OneCoreUAP()
{
    return false;
}

extern "C"
{
#ifndef LANGUAGE_SERVICE
    bool IsMessageBoxWPresent()
    {
        return true;
    }
#endif
}