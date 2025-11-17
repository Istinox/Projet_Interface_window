#include <windows.h>
#include <fstream>
#include <bitset>
#include <vector>
#include "ImageManager.h"
#include "StegEngine.h"

#define WRITE_MESSAGE 1
#define EXTRACT_MESSAGE 2
#define SAVE_MESSAGE 3
#define LOAD_IMAGE 4
#define LEAVE 5

ImageManager imageManager;
StegEngine stegEngine;

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND hImage;
HWND hEmbedLSB;

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
        0, L"MainWindow", L"Gestionnaire de fichier BMP", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1600, 900,
        NULL, NULL, hInstance, NULL
    );

    hEmbedLSB = CreateWindowEx(
        0, L"EDIT", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT,
        40, 150, 200, 150,
        hwnd, (HMENU)10, hInstance, NULL
    );

    HWND hLabelEmbedLSB = CreateWindowEx(
        0, L"STATIC", L"Ecrire un message code :",
        WS_VISIBLE | WS_CHILD,
        40, 130, 200, 20,
        hwnd, NULL, hInstance, NULL
    );

    HWND hButtonEmbedLSB = CreateWindowEx(
        0, L"BUTTON", L"Enregistrer",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        40, 300, 200, 20,
        hwnd, (HMENU)1, hInstance, NULL
    );

    if (!hwnd) {
        MessageBox(NULL, L"Erreur : Création de la fenêtre échouée.", L"Erreur", MB_ICONERROR);
        return 0;
    }

    hImage = CreateWindowEx(
        0, L"STATIC", NULL,
        WS_VISIBLE | WS_CHILD | SS_BITMAP | WS_BORDER,
        300, 40, 800, 600,
        hwnd, (HMENU)10, hInstance, NULL
    );

    // Centrage de la zone d'image par rapport à la taille de la fenêtre
    RECT rc;
    GetClientRect(hwnd, &rc);

    int ctrlWidth = 800;
    int ctrlHeight = 600;

    // Calcul des positions pour le centrage
    int posX = (rc.right - ctrlWidth) / 2;
    int posY = (rc.bottom - ctrlHeight) / 2;

    // Déplacement du contrôle
    MoveWindow(hImage, posX, posY, ctrlWidth, ctrlHeight, TRUE);

    // Initialisation du menu
    HMENU hMenu = CreateMenu();
    HMENU hSubMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"Fichier");

    AppendMenu(hSubMenu, MF_STRING, 2, L"Extraire le message");
    AppendMenu(hSubMenu, MF_STRING, 3, L"Sauvegarder une image");
    AppendMenu(hSubMenu, MF_STRING, 4, L"Charger une image");
    AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL); // Sépare le bouton quitter des autre boutons
    AppendMenu(hSubMenu, MF_STRING, 5, L"Quitter");

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

HWND hEdit;
HBITMAP hBmp = NULL;
OPENFILENAME ofn;
HDC hdcTitle;
PAINTSTRUCT psTitle;
SIZE sizeTitle;
const char* title = "GESTIONNAIRE DE FICHIER BMP";
unsigned int offsetTitle = 275;      // Pour faire le décalage du texte plus efficacement
int xTitle = 800 - offsetTitle;      // Position où afficher le texte
int yTitle = 50;

HBITMAP hBitmap;
BITMAP bmp;

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
        if (LOWORD(lParam) == SAVE_MESSAGE)
        {
            // Sauvegarde l'image contenant le message codé
        }

        else if (LOWORD(lParam) == EXTRACT_MESSAGE)
        {
            // Extrait le message codé présent dans une image
        }

        else if (LOWORD(wParam) == WRITE_MESSAGE)
        {
            wchar_t buffer[1024] = {};
            stegEngine.EmbedLSB(hBitmap);
            GetWindowText(hEmbedLSB, buffer, sizeof(buffer) / sizeof(wchar_t));
            MessageBox(hwnd, L"Ecriture effectue avec succes !", L"Sauvegarde", MB_OK);
        }

        else if (LOWORD(wParam) == LOAD_IMAGE) {

            if (GetOpenFileName(&ofn)) {

                hBitmap = (HBITMAP)LoadImage(
                    NULL, L"exemple.bmp",
                    IMAGE_BITMAP, 0, 0,
                    LR_LOADFROMFILE | LR_CREATEDIBSECTION
                );

                SendMessage(hImage, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
            }
        }

        else if (LOWORD(wParam) == LEAVE) {
            DestroyWindow(hwnd);
        }
        break;

    case WM_PAINT: {
        imageManager.AddTitle(hdcTitle, hwnd, psTitle, title, sizeTitle, xTitle, yTitle);

        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        imageManager.DrawBMPFile(hwnd, hdc, hBitmap);
        EndPaint(hwnd, &ps);
    }
    break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}