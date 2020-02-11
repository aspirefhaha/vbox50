#ifdef RT_OS_WINDOWS
#include <Windows.h>
#endif
#include <iprt/log.h>
typedef int (*PFUNC)();
int minimizedSelf()
{
#ifdef RT_OS_WINDOWS
    HMODULE hm = LoadLibraryA("InEnvDirPlugin.dll"); //假设DLL路径是c:\test\test.dll
    LogRel(("LoadLibraryA %x\n"),hm);
    PFUNC pFunc = (PFUNC)GetProcAddress((HINSTANCE)hm, "InEnvMinimizeWin"); //Find a function and use it
	LogRel(("GetProcAddress %x"),pFunc);
    if (pFunc != NULL)
    {
        LogRel(("Want call pFunc %p (InEnvMinimizeWin)\n"),hm);
        int n = pFunc();      //typedef int (*PFUNC)(void);
        LogRel(("Call InEnvMinimizeWin ENd"));

    }
    FreeLibrary((HMODULE)hm);   //Finally, don't forget to free it.
#endif 
    return 1;

}