#include "stdafx.h"
#include "window_select_common.h"
#include "ignore_hwnd_cache.h"
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#include <strsafe.h>
#include <sddl.h>
//#include "version.h"

class scoped_handle
{
public:
    scoped_handle() = default;
    scoped_handle(HANDLE handle)
        : handle_(handle)
    {
    }

    ~scoped_handle()
    {
        CloseHandle(handle_);
    }

    //HANDLE* HANDLE() noexcept
    //{
    //    return &handle_;
    //}
private:
    HANDLE handle_{nullptr};
};

std::optional<RECT> get_button_pos(HWND hwnd)
{
    RECT button;
    if (DwmGetWindowAttribute(hwnd, DWMWA_CAPTION_BUTTON_BOUNDS, &button, sizeof(button)) == S_OK)
    {
        return button;
    }
    else
    {
        return {};
    }
}

std::optional<RECT> get_window_pos(HWND hwnd)
{
    RECT window;
    if (DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &window, sizeof(window)) == S_OK)
    {
        return window;
    }
    else
    {
        return {};
    }
}

std::optional<POINT> get_mouse_pos()
{
    POINT point;
    if (GetCursorPos(&point) == 0)
    {
        return {};
    }
    else
    {
        return point;
    }
}

// WindowAndProcPath get_filtered_base_window_and_path(HWND window)
//{
//    return hwnd_cache.get_window_and_path(window);
//}
//
// HWND get_filtered_active_window()
//{
//    return hwnd_cache.get_window(GetForegroundWindow());
//}

int width(const RECT &rect)
{
    return rect.right - rect.left;
}

int height(const RECT &rect)
{
    return rect.bottom - rect.top;
}

bool operator<(const RECT &lhs, const RECT &rhs)
{
    auto lhs_tuple = std::make_tuple(lhs.left, lhs.right, lhs.top, lhs.bottom);
    auto rhs_tuple = std::make_tuple(rhs.left, rhs.right, rhs.top, rhs.bottom);
    return lhs_tuple < rhs_tuple;
}

RECT keep_rect_inside_rect(const RECT &small_rect, const RECT &big_rect)
{
    RECT result = small_rect;
    if ((result.right - result.left) > (big_rect.right - big_rect.left))
    {
        // small_rect is too big horizontally. resize it.
        result.right = big_rect.right;
        result.left = big_rect.left;
    }
    else
    {
        if (result.right > big_rect.right)
        {
            // move the rect left.
            result.left -= result.right - big_rect.right;
            result.right -= result.right - big_rect.right;
        }

        if (result.left < big_rect.left)
        {
            // move the rect right.
            result.right += big_rect.left - result.left;
            result.left += big_rect.left - result.left;
        }
    }

    if ((result.bottom - result.top) > (big_rect.bottom - big_rect.top))
    {
        // small_rect is too big vertically. resize it.
        result.bottom = big_rect.bottom;
        result.top = big_rect.top;
    }
    else
    {
        if (result.bottom > big_rect.bottom)
        {
            // move the rect up.
            result.top -= result.bottom - big_rect.bottom;
            result.bottom -= result.bottom - big_rect.bottom;
        }

        if (result.top < big_rect.top)
        {
            // move the rect down.
            result.bottom += big_rect.top - result.top;
            result.top += big_rect.top - result.top;
        }
    }
    return result;
}

int run_message_loop()
{
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return static_cast<int>(msg.wParam);
}

void show_last_error_message(LPCWSTR lpszFunction, DWORD dw)
{
    // Retrieve the system error message for the error code
    LPWSTR lpMsgBuf = nullptr;
    if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), lpMsgBuf, 0, nullptr) > 0)
    {
        // Display the error message and exit the process
        const auto displayBufSize = (static_cast<long long>(lstrlenW(lpMsgBuf)) + lstrlenW(lpszFunction) + 40) * sizeof(WCHAR);
        if (displayBufSize == 0)
            return;
        auto lpDisplayBuf = reinterpret_cast<LPWSTR>(LocalAlloc(LMEM_ZEROINIT, displayBufSize));
        if (lpDisplayBuf != nullptr)
        {
            StringCchPrintfW(lpDisplayBuf, LocalSize(lpDisplayBuf) / sizeof(WCHAR), L"%s failed with error %d: %s",
                             lpszFunction, dw, lpMsgBuf);
            MessageBoxW(nullptr, lpDisplayBuf, L"Error", MB_OK);
            LocalFree(lpDisplayBuf);
        }
        LocalFree(lpMsgBuf);
    }
}

WindowState get_window_state(HWND hwnd)
{
    WINDOWPLACEMENT placement;
    placement.length = sizeof(WINDOWPLACEMENT);

    if (GetWindowPlacement(hwnd, &placement) == 0)
        return WindowState::UNKNOWN;

    if (placement.showCmd == SW_MINIMIZE || placement.showCmd == SW_SHOWMINIMIZED || IsIconic(hwnd))
        return WindowState::MINIMIZED;

    if (placement.showCmd == SW_MAXIMIZE)
        return WindowState::MAXIMIZED;

    auto rectp = get_window_pos(hwnd);
    if (!rectp)
        return WindowState::UNKNOWN;

    auto rect = *rectp;
    MONITORINFO monitor;
    monitor.cbSize = sizeof(MONITORINFO);
    auto h_monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    GetMonitorInfo(h_monitor, &monitor);
    bool top_left = monitor.rcWork.top == rect.top && monitor.rcWork.left == rect.left;
    bool bottom_left = monitor.rcWork.bottom == rect.bottom && monitor.rcWork.left == rect.left;
    bool top_right = monitor.rcWork.top == rect.top && monitor.rcWork.right == rect.right;
    bool bottom_right = monitor.rcWork.bottom == rect.bottom && monitor.rcWork.right == rect.right;

    if (top_left && bottom_left)
        return WindowState::SNAPED_LEFT;
    if (top_left)
        return WindowState::SNAPED_TOP_LEFT;
    if (bottom_left)
        return WindowState::SNAPED_BOTTOM_LEFT;
    if (top_right && bottom_right)
        return WindowState::SNAPED_RIGHT;
    if (top_right)
        return WindowState::SNAPED_TOP_RIGHT;
    if (bottom_right)
        return WindowState::SNAPED_BOTTOM_RIGHT;

    return WindowState::RESTORED;
}

bool is_process_elevated()
{
    HANDLE token = nullptr;
    bool elevated = false;

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
    {
        TOKEN_ELEVATION elevation;
        DWORD size;
        if (GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &size))
        {
            elevated = (elevation.TokenIsElevated != 0);
        }
    }

    if (token)
    {
        CloseHandle(token);
    }

    return elevated;
}

#if 0
bool drop_elevated_privileges()
{
    HANDLE token = nullptr;
    LPCTSTR lpszPrivilege = SE_SECURITY_NAME;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_DEFAULT | WRITE_OWNER, &token))
    {
        return false;
    }

    PSID medium_sid = NULL;
    if (!::ConvertStringSidToSid(SDDL_ML_MEDIUM, &medium_sid))
    {
        return false;
    }

    TOKEN_MANDATORY_LABEL label = {0};
    label.Label.Attributes = SE_GROUP_INTEGRITY;
    label.Label.Sid = medium_sid;
    DWORD size = (DWORD)sizeof(TOKEN_MANDATORY_LABEL) + ::GetLengthSid(medium_sid);

    BOOL result = SetTokenInformation(token, TokenIntegrityLevel, &label, size);
    LocalFree(medium_sid);
    CloseHandle(token);

    return result;
}
#endif

std::wstring get_process_path(DWORD pid) noexcept
{
    auto process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, TRUE, pid);
    std::wstring name;
    if (process != INVALID_HANDLE_VALUE)
    {
        name.resize(MAX_PATH);
        DWORD name_length = static_cast<DWORD>(name.length());
        if (QueryFullProcessImageNameW(process, 0, (LPWSTR)name.data(), &name_length) == 0)
        {
            name_length = 0;
        }
        name.resize(name_length);
        CloseHandle(process);
    }
    return name;
}

bool run_elevated(const std::wstring &file, const std::wstring &params)
{
    SHELLEXECUTEINFOW exec_info = {0};
    exec_info.cbSize = sizeof(SHELLEXECUTEINFOW);
    exec_info.lpVerb = L"runas";
    exec_info.lpFile = file.c_str();
    exec_info.lpParameters = params.c_str();
    exec_info.hwnd = 0;
    exec_info.fMask = SEE_MASK_NOCLOSEPROCESS;
    exec_info.lpDirectory = 0;
    exec_info.hInstApp = 0;

    if (ShellExecuteExW(&exec_info))
    {
        return exec_info.hProcess != nullptr;
    }
    else
    {
        return false;
    }
}

#if 0
bool run_non_elevated(const std::wstring &file, const std::wstring &params)
{
    auto executable_args = file;
    if (!params.empty())
    {
        executable_args += L" " + params;
    }

    HWND hwnd = GetShellWindow();
    if (!hwnd)
    {
        return false;
    }
    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);

    winrt::handle process{OpenProcess(PROCESS_CREATE_PROCESS, FALSE, pid)};
    if (!process)
    {
        return false;
    }

    SIZE_T size = 0;

    InitializeProcThreadAttributeList(nullptr, 1, 0, &size);
    auto pproc_buffer = std::make_unique<char[]>(size);
    auto pptal = reinterpret_cast<PPROC_THREAD_ATTRIBUTE_LIST>(pproc_buffer.get());

    if (!InitializeProcThreadAttributeList(pptal, 1, 0, &size))
    {
        return false;
    }

    HANDLE process_handle = process.get();
    if (!pptal || !UpdateProcThreadAttribute(pptal, 0, PROC_THREAD_ATTRIBUTE_PARENT_PROCESS, &process_handle,
                                             sizeof(process_handle), nullptr, nullptr))
    {
        return false;
    }

    STARTUPINFOEX siex = {0};
    siex.lpAttributeList = pptal;
    siex.StartupInfo.cb = sizeof(siex);

    PROCESS_INFORMATION process_info = {0};
    auto succedded = CreateProcessW(file.c_str(), const_cast<LPWSTR>(executable_args.c_str()), nullptr, nullptr, FALSE,
                                    EXTENDED_STARTUPINFO_PRESENT, nullptr, nullptr, &siex.StartupInfo, &process_info);
    if (process_info.hProcess)
    {
        CloseHandle(process_info.hProcess);
    }
    if (process_info.hThread)
    {
        CloseHandle(process_info.hThread);
    }
    return succedded;
}
#endif

bool run_same_elevation(const std::wstring &file, const std::wstring &params)
{
    auto executable_args = file;
    if (!params.empty())
    {
        executable_args += L" " + params;
    }
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};
    auto succedded = CreateProcessW(file.c_str(), const_cast<LPWSTR>(executable_args.c_str()), nullptr, nullptr, FALSE,
                                    0, nullptr, nullptr, &si, &pi);
    if (pi.hProcess)
    {
        CloseHandle(pi.hProcess);
    }
    if (pi.hThread)
    {
        CloseHandle(pi.hThread);
    }
    return succedded;
}

std::wstring get_process_path(HWND window) noexcept
{
    static constexpr std::wstring_view app_frame_host = L"ApplicationFrameHost.exe";
    DWORD pid{};
    GetWindowThreadProcessId(window, &pid);
    auto name = get_process_path(pid);
    if (std::size(name) >= std::size(app_frame_host) &&
        name.compare(std::size(name) - std::size(app_frame_host), std::size(app_frame_host), app_frame_host) == 0)
    {
        // It is a UWP app. We will enumerate the windows and look for one created
        // by something with a different PID
        DWORD new_pid = pid;
        EnumChildWindows(
            window,
            [](HWND hwnd, LPARAM param) -> BOOL {
                auto new_pid_ptr = reinterpret_cast<DWORD *>(param);
                DWORD pid;
                GetWindowThreadProcessId(hwnd, &pid);
                if (pid != *new_pid_ptr)
                {
                    *new_pid_ptr = pid;
                    return FALSE;
                }
                else
                {
                    return TRUE;
                }
            },
            reinterpret_cast<LPARAM>(&new_pid));
        // If we have a new pid, get the new name.
        if (new_pid != pid)
        {
            return get_process_path(new_pid);
        }
    }
    return name;
}

std::wstring get_resource_string(UINT resource_id, HINSTANCE instance, const wchar_t *fallback)
{
    wchar_t *text_ptr;
    auto length = LoadStringW(instance, resource_id, reinterpret_cast<wchar_t *>(&text_ptr), 0);
    if (length == 0)
    {
        return fallback;
    }
    else
    {
        return {text_ptr, static_cast<std::size_t>(length)};
    }
}

std::wstring get_module_filename(HMODULE mod)
{
    wchar_t buffer[MAX_PATH + 1];
    DWORD actual_length = GetModuleFileNameW(mod, buffer, MAX_PATH);
    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        const DWORD long_path_length = 0xFFFF; // should be always enough
        std::wstring long_filename(long_path_length, L'\0');
        actual_length = GetModuleFileNameW(mod, long_filename.data(), long_path_length);
        return long_filename.substr(0, actual_length);
    }
    return {buffer, actual_length};
}

std::wstring get_module_folderpath(HMODULE mod)
{
    wchar_t buffer[MAX_PATH + 1];
    DWORD actual_length = GetModuleFileNameW(mod, buffer, MAX_PATH);
    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        const DWORD long_path_length = 0xFFFF; // should be always enough
        std::wstring long_filename(long_path_length, L'\0');
        actual_length = GetModuleFileNameW(mod, long_filename.data(), long_path_length);
        PathRemoveFileSpecW(long_filename.data());
        long_filename.resize(actual_length);
        long_filename.shrink_to_fit();
        return long_filename;
    }

    PathRemoveFileSpecW(buffer);
    return {buffer, (UINT)lstrlenW(buffer)};
}