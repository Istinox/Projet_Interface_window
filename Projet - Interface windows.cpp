#include <windows.h>
#include <string>
#include "ImageManager.h"
#include "StegEngine.h"

#define ID_BTN_WRITE     1
#define ID_MENU_EXTRACT  2
#define ID_MENU_SAVE     3
#define ID_MENU_LOAD     4
#define ID_MENU_QUIT     5
#define ID_EDIT_MESSAGE  10
#define ID_STATIC_IMG    11

ImageManager imageManager;
StegEngine stegEngine;

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND hImage = NULL;
HWND hEmbedLSB = NULL;
HBITMAP hBitmap = NULL;

OPENFILENAMEW ofn;

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"MainWindow";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClassW(&wc)) {
        MessageBoxW(NULL, L"Erreur : Enregistrement de la classe echoue.", L"Erreur", MB_ICONERROR);
        return 0;
    }

    HWND hwnd = CreateWindowExW(
        0, L"MainWindow", L"Gestionnaire de fichier BMP", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1600, 900,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) {
        MessageBoxW(NULL, L"Erreur : Creation de la fenetre echouee.", L"Erreur", MB_ICONERROR);
        return 0;
    }

    hEmbedLSB = CreateWindowExW(
        0, L"EDIT", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
        40, 150, 300, 150,
        hwnd, (HMENU)ID_EDIT_MESSAGE, hInstance, NULL
    );

    CreateWindowExW(
        0, L"STATIC", L"Ecrire un message code :",
        WS_VISIBLE | WS_CHILD,
        40, 130, 200, 20,
        hwnd, NULL, hInstance, NULL
    );

    CreateWindowExW(
        0, L"BUTTON", L"Enregistrer dans l'image",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        40, 300, 200, 24,
        hwnd, (HMENU)ID_BTN_WRITE, hInstance, NULL
    );

    // Contrôle pour afficher l'image (STM_SETIMAGE)
    hImage = CreateWindowExW(
        0, L"STATIC", NULL,
        WS_VISIBLE | WS_CHILD | SS_BITMAP | WS_BORDER,
        400, 150, 800, 600,
        hwnd, (HMENU)ID_STATIC_IMG, hInstance, NULL
    );

    // Créer menu
    HMENU hMenu = CreateMenu();
    HMENU hSubMenu = CreatePopupMenu();

    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"Fichier");
    AppendMenuW(hSubMenu, MF_STRING, ID_MENU_EXTRACT, L"Extraire le message");
    AppendMenuW(hSubMenu, MF_STRING, ID_MENU_SAVE, L"Sauvegarder une image...");
    AppendMenuW(hSubMenu, MF_STRING, ID_MENU_LOAD, L"Charger une image...");
    AppendMenuW(hSubMenu, MF_SEPARATOR, 0, NULL); // Pour séparer le bouton quitter des autres boutons
    AppendMenuW(hSubMenu, MF_STRING, ID_MENU_QUIT, L"Quitter");
    SetMenu(hwnd, hMenu);

    // Préparer OPENFILENAMEW structure (utilisée pour Open/Save)
    static wchar_t szFile[260] = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
    ofn.lpstrFilter = L"Fichiers BMP\0*.bmp\0Tous fichiers\0*.*\0";
    ofn.lpstrDefExt = L".bmp";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    // Cleanup
    if (hBitmap) { DeleteObject(hBitmap); hBitmap = NULL; }

    return (int)msg.wParam;
}

HDC hdcTitle;
PAINTSTRUCT psTitle;
SIZE sizeTitle;
const char* title = "GESTIONNAIRE DE FICHIER BMP";
unsigned int offsetTitle = 275;      // Pour faire le décalage du texte plus efficacement
int xTitle = 800 - offsetTitle;      // Position où afficher le texte
int yTitle = 50;


LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_COMMAND:
    {
        int id = LOWORD(wParam);
        switch (id)
        {
        case ID_BTN_WRITE:
        {
            if (!hBitmap) {
                MessageBoxW(hwnd, L"Aucune image chargee.", L"Erreur", MB_ICONERROR);
                break;
            }

            // Lire le texte depuis la zone d'édition
            wchar_t buffer[2048] = {};
            GetWindowTextW(hEmbedLSB, buffer, _countof(buffer));
            std::wstring err;
            if (!stegEngine.EmbedLSB(hBitmap, buffer, err)) {
                MessageBoxW(hwnd, err.c_str(), L"Erreur", MB_ICONERROR);
            }
            else {
                MessageBoxW(hwnd, L"Ecriture effectuee avec succes !", L"Succes", MB_OK);
                // Redessiner la fenêtre pour voir l'image mise à jour
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        }

        case ID_MENU_LOAD:
        {
            ofn.lpstrFile[0] = L'\0';
            if (GetOpenFileNameW(&ofn))
            {
                std::wstring err;
                HBITMAP newBmp = imageManager.LoadFromFile(ofn.lpstrFile, err);
                if (!newBmp)
                {
                    MessageBoxW(hwnd, err.c_str(), L"Erreur", MB_ICONERROR);
                }
                else
                {
                    if (hBitmap) { DeleteObject(hBitmap); hBitmap = NULL; }

                    hBitmap = newBmp;

                    // Récupérer taille réelle du BMP
                    BITMAP bmp = {};
                    GetObject(hBitmap, sizeof(bmp), &bmp);
                    int imgW = bmp.bmWidth;
                    int imgH = bmp.bmHeight;

                    // Taille de la fenêtre
                    RECT rc;
                    GetClientRect(hwnd, &rc);

                    int winW = rc.right - rc.left;
                    int winH = rc.bottom - rc.top;

                    // Centrage
                    int posX = (winW - imgW) / 2;
                    int posY = (winH - imgH) / 2;

                    // Déplacer/redimensionner le STATIC
                    MoveWindow(hImage, posX, posY, imgW, imgH, TRUE);

                    // Afficher l'image
                    SendMessageW(hImage, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);

                    InvalidateRect(hwnd, NULL, TRUE);
                }
            }
            break;
        }


        case ID_MENU_SAVE:
        {
            if (!hBitmap) {
                MessageBoxW(hwnd, L"Aucune image à sauvegarder.", L"Erreur", MB_ICONERROR);
                break;
            }

            static wchar_t saveFile[260] = {};
            ofn.lpstrFile = saveFile;
            ofn.lpstrFile[0] = L'\0';
            ofn.nMaxFile = sizeof(saveFile) / sizeof(wchar_t);
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

            if (GetSaveFileNameW(&ofn)) {
                std::wstring err;
                if (!imageManager.SaveToFile(hBitmap, ofn.lpstrFile, err)) {
                    MessageBoxW(hwnd, err.c_str(), L"Erreur", MB_ICONERROR);
                }
                else {
                    MessageBoxW(hwnd, L"Image sauvegardee avec succes.", L"Succes", MB_OK);
                }
            }
            static wchar_t openBuf[260] = {};
            ofn.lpstrFile = openBuf;
            ofn.nMaxFile = sizeof(openBuf) / sizeof(wchar_t);
            break;
        }

        case ID_MENU_EXTRACT:
        {
            if (!hBitmap) {
                MessageBoxW(hwnd, L"Aucune image chargee.", L"Erreur", MB_ICONERROR);
                break;
            }
            std::wstring extracted;
            std::wstring err;
            if (!stegEngine.ExtractLSB(hBitmap, extracted, err)) {
                MessageBoxW(hwnd, err.c_str(), L"Erreur", MB_ICONERROR);
            }
            else {
                MessageBoxW(hwnd, extracted.c_str(), L"Message extrait", MB_OK);
            }
            break;
        }

        case ID_MENU_QUIT:
            DestroyWindow(hwnd);
            break;

        default:
            break;
        }
        break;
    }
    case WM_SIZE:
    {
        if (hImage && hBitmap)
        {
            int winW = LOWORD(lParam);
            int winH = HIWORD(lParam);

            int imgW = 800;
            int imgH = 600;

            int posX = (winW - imgW) / 2;
            int posY = (winH - imgH) / 2;

            MoveWindow(hImage, posX, posY, imgW, imgH, TRUE);
        }
    }
    break;

    case WM_PAINT:
    {
        imageManager.AddTitle(hdcTitle, hwnd, psTitle, title, sizeTitle, xTitle, yTitle);
    }
    break;

    default:
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}