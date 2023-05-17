//
// Created by ZHIKANG on 2023/5/17.
//

#include <Sandbox/game_app.h>


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE prevInstance,
                    _In_ LPSTR cmdLine, _In_ int showCmd)
{
    // never use
    UNREFERENCED_PARAMETER(prevInstance);
    UNREFERENCED_PARAMETER(cmdLine);
    UNREFERENCED_PARAMETER(showCmd);
    // Allow memory allocate and memory deduce while running
#if defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    toy::game_app_c app(hInstance, "DirectX11 Initialization", 1280, 720);
    app.init();
    return app.run();
}




