#include <windows.h>
#include <string>
#include <locale>

// Global variables
HINSTANCE hInst;
const wchar_t* lpzClass = L"ClipboardViewerWindowClass";
HWND hwndMain, hwndContent, hwndTextContent, hwndImageContent;

// Function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void UpdateClipboardData(HWND hwnd);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    std::locale::global(std::locale("English_United States.1252"));

    hInst = hInstance;
    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = lpzClass;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    hwndMain = CreateWindowEx(
        0, lpzClass, L"Clipboard Viewer", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 600, NULL, NULL, hInstance, NULL);

    if (hwndMain == NULL) {
        MessageBox(NULL, L"Window Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Create a static control to display text content
    hwndTextContent = CreateWindow(L"STATIC", NULL,
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        0, 0, 600, 600,
        hwndMain, NULL, hInstance, NULL);

    // Create a static control to display image content
    hwndImageContent = CreateWindow(L"STATIC", NULL,
        WS_CHILD | WS_VISIBLE | SS_BITMAP,
        0, 0, 600, 600,
        hwndMain, NULL, hInstance, NULL);

    ShowWindow(hwndMain, nCmdShow);
    UpdateWindow(hwndMain);

    // Set timer to update the clipboard content every 1000 ms (1 second)
    SetTimer(hwndMain, 1, 1000, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        // Set timer to update the clipboard content every 1000 ms (1 second)
        SetTimer(hwnd, 1, 1000, NULL);
        break;
    }
    case WM_TIMER: {
        UpdateClipboardData(hwnd);
        break;
    }
    case WM_SIZE: {
        // Resize the content window to fill the entire client area
        MoveWindow(hwndTextContent, 10, 10, LOWORD(lParam) - 20, HIWORD(lParam) - 20, TRUE);
        MoveWindow(hwndImageContent, 10, 10, LOWORD(lParam) - 20, HIWORD(lParam) - 20, TRUE);
        break;
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        break;
    }
    default: {
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    }
    return 0;
}

DWORD g_LastClipboardSequenceNumber = 0;

// Function to check if the clipboard content has changed
bool ClipboardContentChanged() {
    DWORD currentSequenceNumber = GetClipboardSequenceNumber();
    if (currentSequenceNumber != g_LastClipboardSequenceNumber) {
        g_LastClipboardSequenceNumber = currentSequenceNumber;
        return true;
    }
    return false;
}

// Function to check if the clipboard contains text
bool ClipboardContainsText() {
    if (OpenClipboard(NULL)) {
        HANDLE hData = GetClipboardData(CF_UNICODETEXT);
        CloseClipboard();
        return hData != NULL;
    }
    return false;
}

// Function to check if the clipboard contains an image
bool ClipboardContainsImage() {
    if (OpenClipboard(NULL)) {
        HANDLE hData = GetClipboardData(CF_DIB);
        CloseClipboard();
        return hData != NULL;
    }
    return false;
}

void UpdateClipboardData(HWND hwnd) {
    if (ClipboardContentChanged()) {
        if (OpenClipboard(hwnd)) {
            // Check for text data first
            HANDLE hTextData = GetClipboardData(CF_UNICODETEXT);
            if (hTextData != nullptr) {
                wchar_t* pszText = static_cast<wchar_t*>(GlobalLock(hTextData));
                if (pszText != nullptr) {
                    SetWindowText(hwndTextContent, pszText);
                    GlobalUnlock(hTextData);
                    CloseClipboard();
                    // Show text content control and hide image content control
                    ShowWindow(hwndTextContent, SW_SHOW);
                    ShowWindow(hwndImageContent, SW_HIDE);
                    return; // Exit function if text data is found
                }
            }

            // If no text data is found, check for image data
            HANDLE hBitmapData = GetClipboardData(CF_BITMAP);
            if (hBitmapData != nullptr) {
                HBITMAP hBitmap = static_cast<HBITMAP>(hBitmapData);
                SendMessage(hwndImageContent, STM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(hBitmap));
                CloseClipboard();
                // Show image content control and hide text content control
                ShowWindow(hwndTextContent, SW_HIDE);
                ShowWindow(hwndImageContent, SW_SHOW);
                return; // Exit function if image data is found
            }

            // If neither text nor image data is available, clear both content controls
            SetWindowText(hwndTextContent, L"No data available on clipboard");
            SendMessage(hwndImageContent, STM_SETIMAGE, IMAGE_BITMAP, NULL);
            CloseClipboard();
            // Show both content controls
            ShowWindow(hwndTextContent, SW_SHOW);
            ShowWindow(hwndImageContent, SW_SHOW);
        }
        else {
            // If unable to open the clipboard, display an error message in the text content control
            SetWindowText(hwndTextContent, L"Error accessing clipboard");
            // Clear image content control
            SendMessage(hwndImageContent, STM_SETIMAGE, IMAGE_BITMAP, NULL);
            // Show both content controls
            ShowWindow(hwndTextContent, SW_SHOW);
            ShowWindow(hwndImageContent, SW_SHOW);
        }
    }
}
