#include <windows.h>
#include <fstream>

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND hEdit;
HWND hImage;
HBITMAP hBmp = NULL;
OPENFILENAME ofn;

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"MainWindow";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, L"Erreur : Enregistrement de la classe échoué.", L"Erreur", MB_ICONERROR);
        return 0;
    }

    HWND hwnd = CreateWindowEx(
        0, L"MainWindow", L"Interface window", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1600, 900,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) {
        MessageBox(NULL, L"Erreur : Création de la fenêtre échouée.", L"Erreur", MB_ICONERROR);
        return 0;
    }

    // Zone de texte
    hEdit = CreateWindowEx(
        0, L"EDIT", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT,
        20, 60, 200, 25,
        hwnd, (HMENU)1, hInstance, NULL
    );

    CreateWindowEx(
        0, L"STATIC", L"Zone de texte :",
        WS_VISIBLE | WS_CHILD,
        20, 40, 200, 20,
        hwnd, NULL, hInstance, NULL
    );

    hImage = CreateWindowEx(
        0, L"STATIC", NULL,
        WS_VISIBLE | WS_CHILD | SS_BITMAP | WS_BORDER,
        300, 40, 800, 600,
        hwnd, (HMENU)10, hInstance, NULL
    );

    HMENU hMenu = CreateMenu();
    HMENU hSubMenu = CreatePopupMenu();

    AppendMenu(hSubMenu, MF_STRING, 2, L"Extraire le message");
    AppendMenu(hSubMenu, MF_STRING, 3, L"Sauvegarder l'image code");
    AppendMenu(hSubMenu, MF_STRING, 4, L"Charger l'image code");
    AppendMenu(hSubMenu, MF_STRING, 5, L"Integrer un message texte dans l'image");
    AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hSubMenu, MF_STRING, 6, L"Quitter");

    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"Fichier");
    SetMenu(hwnd, hMenu);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    wchar_t szFile[260] = {};

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"Fichiers BMP\0*.bmp\0Tous fichiers\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    switch (uMsg)
    {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == 4) {

            if (GetOpenFileName(&ofn))
            {
                // Libérer un ancien bitmap
                if (hBmp) {
                    DeleteObject(hBmp);
                }

                // Charger le bitmap
                hBmp = (HBITMAP)LoadImage(
                    NULL,
                    ofn.lpstrFile,
                    IMAGE_BITMAP,
                    0, 0,
                    LR_LOADFROMFILE | LR_CREATEDIBSECTION
                );

                if (!hBmp) {
                    MessageBox(hwnd, L"Impossible de charger le BMP.", L"Erreur", MB_ICONERROR);
                }
                else
                {
                    // Afficher l'image dans la zone STATIC
                    SendMessage(hImage, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);

                    // Récupérer la taille du bitmap et ajuster la fenêtre d'affichage
                    BITMAP bmp;
                    GetObject(hBmp, sizeof(BITMAP), &bmp);

                    SetWindowPos(hImage, NULL, 300, 40, bmp.bmWidth, bmp.bmHeight, SWP_NOZORDER);
                }
            }
        }

        else if (LOWORD(wParam) == 6) {
            DestroyWindow(hwnd);
        }

        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}
