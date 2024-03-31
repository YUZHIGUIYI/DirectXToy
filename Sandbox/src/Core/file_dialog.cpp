//
// Created by ZZK on 2023/6/25.
//

#include <Sandbox/Core/file_dialog.h>
#include <commdlg.h>

namespace toy::editor
{
    static std::string file_dialog(GLFWwindow* glfw_window, const char *title, const char *exts, bool open_to_load)
    {
        if(!glfw_window)
        {
            DX_WARN("Attempt to fall file dialog on null window");
            return std::string{};
        }
        HWND hwnd = glfwGetWin32Window(glfw_window);

        std::vector<char> extsfixed;
        for(size_t i = 0; i < strlen(exts); i++)
        {
            if(exts[i] == '|')
            {
                extsfixed.push_back(0);
            }
            else
            {
                extsfixed.push_back(exts[i]);
            }
        }
        extsfixed.push_back(0);
        extsfixed.push_back(0);

        OPENFILENAME ofn;           // common dialog box structure
        static char  szFile[1024];  // buffer for file name

        // Initialize OPENFILENAME
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner   = hwnd;
        ofn.lpstrFile   = szFile;
        // Set lpstrFile[0] to '\0' so that GetOpenFileName does not
        // use the contents of szFile to initialize itself.
        ofn.lpstrFile[0]    = '\0';
        ofn.nMaxFile        = sizeof(szFile);
        ofn.lpstrFilter     = extsfixed.data();
        ofn.nFilterIndex    = 1;
        ofn.lpstrFileTitle  = nullptr;
        ofn.nMaxFileTitle   = 0;
        ofn.lpstrInitialDir = nullptr;
        ofn.Flags           = OFN_PATHMUSTEXIST;
        ofn.lpstrTitle      = title;

        // Display the Open dialog box.

        if(open_to_load)
        {
            ofn.Flags |= OFN_FILEMUSTEXIST;
            if(GetOpenFileNameA(&ofn) == TRUE)
            {
                return ofn.lpstrFile;
            }
        } else
        {
            ofn.Flags |= OFN_OVERWRITEPROMPT;
            if(GetSaveFileNameA(&ofn) == TRUE)
            {
                return ofn.lpstrFile;
            }
        }

        return std::string{};
    }

    std::string FileDialog::window_open_file_dialog(GLFWwindow *glfw_window, const char *title, const char *exts)
    {
        return file_dialog(glfw_window, title, exts, true);
    }

    std::string FileDialog::window_save_file_dialog(GLFWwindow *glfw_window, const char *title, const char *exts)
    {
        return file_dialog(glfw_window, title, exts, false);
    }
}