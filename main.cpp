// Created 2022 - Baltazarus

#include <Windows.h>
#include <CommCtrl.h>
#include <ctime>
#include <fstream>
#include <string>
#include <sstream>
#include "resource.h"

const wchar_t className[] = L"Mini_Random_Picker";

void InitUI(HWND, HINSTANCE);
std::string OpenFileWithDialog(const char*, HWND, int);
void UpdateStats();

LRESULT __stdcall DlgProc_About(HWND, UINT, WPARAM, LPARAM);

static HWND w_ListBoxMain = nullptr;
static HWND w_ButtonInsert = nullptr;
static HWND w_ButtonDelete = nullptr;
static HWND w_ButtonSelect = nullptr;
static HWND w_EditInsert = nullptr;
static HWND w_StaticStats = nullptr;
static HWND w_StatusBar = nullptr;

LRESULT __stdcall WndProc(HWND w_Handle, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    static HBRUSH defhbr = CreateSolidBrush((COLORREF)GetSysColor(COLOR_WINDOW));

    switch(Msg)
    {
        case WM_CREATE:
        {
            InitUI(w_Handle, GetModuleHandleW(nullptr));
            break;
        }
        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case IDC_BUTTON_INSERT:
                {
                    int len = GetWindowTextLengthW(w_EditInsert) + 2;
                    if((len - 2) < 1)
                        break;

                    wchar_t* buffer = new wchar_t[len];
                    GetWindowTextW(w_EditInsert, buffer, len);
                    SendMessageW(w_ListBoxMain, LB_ADDSTRING, 0u, reinterpret_cast<LPARAM>(buffer));
                    SetWindowTextW(w_EditInsert, nullptr);
                    delete[] buffer;

                    UpdateStats();
                    break;
                }
                case IDC_BUTTON_DELETE:
                {
                    int index = SendMessageW(w_ListBoxMain, LB_GETCURSEL, 0u, 0u);
                    if(index != LB_ERR)
                    {
                        SendMessageW(w_ListBoxMain, LB_DELETESTRING, static_cast<WPARAM>(index), 0u);
                        SendMessageW(w_ListBoxMain, LB_SETCURSEL, 0u, 0u);
                        UpdateStats();
                    }
                    else
                        MessageBoxW(w_Handle, L"No selection.", L"Delete", MB_OK);
                    break;
                }
                case IDC_BUTTON_SELECT:
                {
                    int count = SendMessageW(w_ListBoxMain, LB_GETCOUNT, 0u, 0u);
                    if(count > 0)
                    {
                        int select_index = rand() % count;
                        int len = SendMessageW(w_ListBoxMain, LB_GETTEXTLEN, static_cast<WPARAM>(select_index), 0u) + 1;
                        wchar_t* buffer = new wchar_t[len];

                        SendMessageW(w_ListBoxMain, LB_GETTEXT, static_cast<WPARAM>(select_index), reinterpret_cast<LPARAM>(buffer));
                        MessageBoxW(w_Handle, buffer, L"Selected Item", MB_OK | MB_ICONINFORMATION);

                        delete[] buffer;
                    }
                    else
                        MessageBoxW(w_Handle, L"Nothing to select.", L"Select", MB_OK);
                    break;
                }
                case ID_FILE_CLEAR:
                    SendMessageW(w_ListBoxMain, LB_RESETCONTENT, 0u, 0u);
		    SendMessageW(w_StatusBar, SB_SETTEXTW, 0u, reinterpret_cast<LPARAM>(L"Items: 0"));
                    break;
                case ID_FILE_EXIT:
                    DestroyWindow(w_Handle);
                    break;
                case ID_FILE_IMPORT:
                {
                    std::string path = OpenFileWithDialog("All Files\0*.*\0", w_Handle, 1);
                    if(path.length() < 1)
                        break;

                    std::wifstream file;
                    file.open(path);
                    if(file.is_open())
                    {
                        std::wstring line;
                        while(std::getline(file, line))
                            SendMessageW(w_ListBoxMain, LB_ADDSTRING, 0u, reinterpret_cast<LPARAM>(line.c_str()));
                        file.close();
                    }

                    UpdateStats();
                    break;
                }
                case ID_FILE_EXPORT:
                {
                    if(!SendMessageW(w_ListBoxMain, LB_GETCOUNT, 0u, 0u))
                    {
                        MessageBoxW(w_Handle, L"Nothing to export.", L"Export", MB_OK);
                        break;
                    }
                    
                    std::string path = OpenFileWithDialog("All Files\0*.*\0", w_Handle, 0);
                    if(path.length() < 1)
                        break;

                    std::wofstream file;
                    file.open(path);
                    if(file.is_open())
                    {
                        int count = SendMessageW(w_ListBoxMain, LB_GETCOUNT, 0u, 0u);
                        for(int i = 0; i < count; ++i)
                        {
                            int item_len = SendMessageW(w_ListBoxMain, LB_GETTEXTLEN, static_cast<WPARAM>(i), 0u) + 2;
                            wchar_t* buffer = new wchar_t[item_len];
                            SendMessageW(w_ListBoxMain, LB_GETTEXT, static_cast<WPARAM>(i), reinterpret_cast<LPARAM>(buffer));
                            file << buffer << std::endl;
                            delete[] buffer;
                        }
                        file.close();
                    }
                    break;
                }
                case ID_HELP_ABOUT:
                {
                    DialogBoxW(
                        GetModuleHandleW(nullptr),
                        MAKEINTRESOURCEW(IDD_ABOUT),
                        w_Handle,
                        reinterpret_cast<DLGPROC>(&DlgProc_About)
                    );
                    break;
                }
            }
            break;
        }
        case WM_SIZE:
        {
            RECT wRect;
            GetClientRect(w_Handle, &wRect);

            MoveWindow(w_ListBoxMain, 15, 15, wRect.right - wRect.left - 150, wRect.bottom - wRect.top - 30 - (25 + 2) - 10, TRUE);
            
            RECT lbRect;
            GetClientRect(w_ListBoxMain, &lbRect);

            MoveWindow(w_ButtonInsert, (lbRect.right - lbRect.left) + 20, 15, wRect.right - wRect.left - lbRect.right - lbRect.left - 25, 30, TRUE);
            MoveWindow(w_ButtonDelete, (lbRect.right - lbRect.left) + 20, 46, wRect.right - wRect.left - lbRect.right - lbRect.left - 25, 30, TRUE);
            MoveWindow(w_ButtonSelect, (lbRect.right - lbRect.left) + 20, 77, wRect.right - wRect.left - lbRect.right - lbRect.left - 25, 30, TRUE);

            MoveWindow(w_EditInsert, 15, lbRect.bottom - lbRect.top + 15 + 3, lbRect.right - lbRect.left + 2, 25, TRUE);
            
            MoveWindow(w_StaticStats, (lbRect.right - lbRect.left) + 20, 108 + 15, wRect.right - wRect.left - lbRect.right - lbRect.left - 25, wRect.bottom - wRect.top - 15 - 108 - 2, TRUE);
            
            SendMessageW(w_StatusBar, WM_SIZE, 0, 0);
            MoveWindow(w_StatusBar, 0, 0, wRect.right, 0, TRUE);

            break;
        }
        case WM_CTLCOLORBTN:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORSTATIC:
        {
            HDC hdc = reinterpret_cast<HDC>(wParam);
            SetTextColor(hdc, RGB(0x00, 0x00, 0x77));
            SetBkColor(hdc, RGB(0xFF, 0xFF, 0xFF));
            return reinterpret_cast<LRESULT>(defhbr);
        }
        case WM_CLOSE:
            DeleteObject(defhbr);
            DestroyWindow(w_Handle);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProcW(w_Handle, Msg, wParam, lParam);
    }
    return 0;
}

int __stdcall WinMain(HINSTANCE w_Inst, HINSTANCE w_PrevInst, char* lpCmdLine, int nCmdShow)
{
    srand(time(nullptr));

    WNDCLASSEXW wcex;
    HWND w_Handle = nullptr;

    memset(&wcex, 0, sizeof(wcex));
    wcex.cbSize = sizeof(wcex);
    wcex.style = 0;
    wcex.lpfnWndProc = &::WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = w_Inst;
    wcex.hIcon = LoadIcon(w_Inst, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(w_Inst, IDC_ARROW);
    wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wcex.lpszClassName = className;
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDR_MENUBAR);
    wcex.hIconSm = LoadIcon(w_Inst, IDI_APPLICATION);

    if(!RegisterClassExW(&wcex))
        return -1;
    
    w_Handle = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        className,
        L"Mini Random Picker",
        WS_OVERLAPPEDWINDOW,
        100, 100, 640, 480,
        nullptr, nullptr, w_Inst, nullptr
    );

    if(w_Handle == NULL)
        return -1;

    ShowWindow(w_Handle, nCmdShow);
    UpdateWindow(w_Handle);

    MSG Msg = { };
    while(GetMessageW(&Msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessageW(&Msg);
    }
    return 0;
}

void InitUI(HWND w_Handle, HINSTANCE w_Inst)
{
    DWORD defStyles = (WS_VISIBLE | WS_CHILD);

    w_ListBoxMain = CreateWindowW(
        WC_LISTBOXW, nullptr,
        defStyles | WS_BORDER | WS_VSCROLL,
        0, 0, 0, 0,
        w_Handle, nullptr, w_Inst, nullptr
    );

    w_ButtonInsert = CreateWindowW(
        WC_BUTTONW, L"Insert",
        defStyles | BS_PUSHBUTTON,
        0, 0, 0, 0,
        w_Handle, reinterpret_cast<HMENU>(IDC_BUTTON_INSERT), w_Inst, nullptr
    );

    w_ButtonDelete = CreateWindowW(
        WC_BUTTONW, L"Delete",
        defStyles | BS_PUSHBUTTON,
        0, 0, 0, 0,
        w_Handle, reinterpret_cast<HMENU>(IDC_BUTTON_DELETE), w_Inst, nullptr
    );

    w_EditInsert = CreateWindowW(
        WC_EDITW, nullptr,
        defStyles | WS_BORDER,
        0, 0, 0, 0,
        w_Handle, nullptr, w_Inst, nullptr
    );

    w_ButtonSelect = CreateWindowW(
        WC_BUTTONW, L"Select",
        defStyles | BS_PUSHBUTTON,
        0, 0, 0, 0,
        w_Handle, reinterpret_cast<HMENU>(IDC_BUTTON_SELECT), w_Inst, nullptr
    );

    w_StaticStats = CreateWindowW(
        WC_STATICW, L"Items: 0",
        defStyles,
        0, 0, 0, 0,
        w_Handle, nullptr, w_Inst, nullptr
    ); 

    w_StatusBar = CreateWindowW(
        STATUSCLASSNAMEW, nullptr,
        defStyles | SBS_SIZEGRIP,
        0, 0, 0, 0,
        w_Handle, nullptr, w_Inst, nullptr
    );

    int sbparts[] = { 100, -1 };
    SendMessageW(
        w_StatusBar, SB_SETPARTS, 
        static_cast<WPARAM>(sizeof(sbparts) / sizeof(sbparts[0])), 
        reinterpret_cast<LPARAM>(sbparts)
    );

    SendMessageW(w_StatusBar, SB_SETTEXTW, 0u, reinterpret_cast<LPARAM>(L"Items: 0"));

    HWND w_Controls[] = { 
        w_ListBoxMain, w_ButtonInsert, w_ButtonDelete, 
        w_ButtonSelect, w_EditInsert, w_StaticStats,
        w_StatusBar
    };

    HFONT hFont = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));

    for(int i = 0; i < (sizeof(w_Controls) / sizeof(w_Controls[0])); ++i)
        SendMessageW(w_Controls[i], WM_SETFONT, reinterpret_cast<WPARAM>(hFont), 1u);
    DeleteObject(hFont);
    return;
}

std::string OpenFileWithDialog(const char* Filters, HWND w_Handle, int criteria)
{
	OPENFILENAMEA ofn = { };
	char* _Path = new char[MAX_PATH];

	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = w_Handle;
	ofn.lpstrFilter = Filters;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = _Path;
	ofn.lpstrFile[0] = '\0';
	if (criteria == 1) ofn.lpstrTitle = "Open File";
	else ofn.lpstrTitle = "Save File";
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = (OFN_EXPLORER | OFN_PATHMUSTEXIST);

	if (criteria == 1)
	{
		if (!GetOpenFileNameA(&ofn))
			return std::string("\0");
	}
	else
	{
		if (!GetSaveFileNameA(&ofn))
			return std::string("\0");
	}

	std::string __Path(_Path);
	delete[] _Path;
	return __Path;
}

void UpdateStats()
{
    int count = SendMessageW(w_ListBoxMain, LB_GETCOUNT, 0u, 0u);
    std::wostringstream woss;
    woss << L"Items: " << count;
    SetWindowTextW(w_StaticStats, woss.str().c_str());
    SendMessageW(w_StatusBar, SB_SETTEXTW, 0u, reinterpret_cast<LPARAM>(woss.str().c_str()));
    return;
}

LRESULT __stdcall DlgProc_About(HWND w_Dlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if(Msg == WM_CLOSE)
        EndDialog(w_Dlg, 0);
    else if(Msg == WM_COMMAND)
    {
        if(LOWORD(wParam) == IDOK)
            EndDialog(w_Dlg, IDOK);
    }
    return 0;
}
