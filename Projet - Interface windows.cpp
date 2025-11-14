#include <windows.h>
#include <fstream>

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND hImage;

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
    AppendMenu(hSubMenu, MF_STRING, 3, L"Sauvegarder l'image code");
    AppendMenu(hSubMenu, MF_STRING, 4, L"Charger l'image code");
    AppendMenu(hSubMenu, MF_STRING, 5, L"Integrer un message texte dans l'image");
    AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL); // Sépare le bouton quitter des autre boutons
    AppendMenu(hSubMenu, MF_STRING, 6, L"Quitter");

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
unsigned int offsetTitle = 275; // Pour faire le décalage du texte plus efficacement
int xTitle = 800 - offsetTitle;      // Position où afficher le texte
int yTitle = 50;

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
            if (GetOpenFileName(&ofn)) {
                if (hBmp) { DeleteObject(hBmp); }
                hBmp = (HBITMAP)LoadImage(NULL, ofn.lpstrFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

                // Si cette erreur apparait, vérifier que l'image BMP n'est pas compressée.
                if (!hBmp) { MessageBox(hwnd, L"Impossible de charger le fichier BMP.", L"Erreur de chargement", MB_ICONERROR); }
                else {
                    // Redimensionner la fenêtre d'affichage par rapport au titre.
                    BITMAP bmp;
                    GetObject(hBmp, sizeof(BITMAP), &bmp);

                    int imageX = xTitle; // = (Pos début du texte + Pos fin du texte) / 2
                    int imageY = yTitle + sizeTitle.cy + 10;
                    int cx = bmp.bmWidth;
                    int cy = bmp.bmHeight;

                    SetWindowPos(hImage, NULL, imageX, imageY, cx, cy, SWP_NOZORDER);

                    SendMessage(hImage, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);
                }
            }
        }

        else if (LOWORD(wParam) == 6) {
            DestroyWindow(hwnd);
        }
        break;

    case WM_PAINT: {
        HFONT hfontTitle = CreateFontA(
            40,                        // taille du texte (en pixels)
            0,                         // largeur (0 = automatique)
            0, 0,                      // angle d'orientation (x, y)
            FW_NORMAL,                 // texte en gras
            FALSE,                     // italique
            FALSE,                     // souligné
            FALSE,                     // barré
            DEFAULT_CHARSET,           // charset
            OUT_CHARACTER_PRECIS,      // précision de sortie
            CLIP_CHARACTER_PRECIS,     // précision du clipping du texte
            CLEARTYPE_QUALITY,         // qualité du rendu du texte
            DEFAULT_PITCH | FF_DONTCARE, // ???
            "Arial"                    // nom de la police
        );

        psTitle;
        hdcTitle = BeginPaint(hwnd, &psTitle);
        GetTextExtentPoint32A(hdcTitle, title, strlen(title), &sizeTitle);

        (HFONT)SelectObject(hdcTitle, hfontTitle);
        TextOutA(hdcTitle, xTitle, yTitle, title, strlen(title));
        EndPaint(hwnd, &psTitle);
    }
    break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}