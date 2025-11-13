#include <windows.h>
#include <fstream>

// Déclaration de la fonction de traitement des messages
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND hEdit;
HWND hImage;
HBITMAP hBitmap;
OPENFILENAME ofn;
HBITMAP hBmp = NULL;

// Point d'entrée de l'application Windows
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 1️ Définir la classe de fenêtre
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;             // Fonction de traitement des messages
    wc.hInstance = hInstance;             // Instance de l'application
    wc.lpszClassName = L"MainWindow";     // Nom unique de la classe
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // Couleur de fond par défaut (blanche)

    // 2️ Enregistrer la classe auprès du système
    if (!RegisterClass(&wc)) {
        MessageBox(NULL, L"Erreur : Enregistrement de la classe échoué.", L"Erreur", MB_ICONERROR);
        return 0;
    }

    // 3️ Créer une fenêtre basée sur la classe enregistrée
    HWND hwnd = CreateWindowEx(
        0, L"MainWindow", L"Interface window", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1600, 900,
        NULL, NULL, hInstance, NULL
    );

    hEdit = CreateWindowEx(
        0, L"EDIT", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT,
        20, 60, 200, 25,
        hwnd, (HMENU)1, hInstance, NULL
    );

    HWND hLabel = CreateWindowEx(
        0, L"STATIC", L"Zone de texte :",
        WS_VISIBLE | WS_CHILD,
        20, 40, 200, 20,
        hwnd, NULL, hInstance, NULL
    );

    hImage = CreateWindowEx(
        0, L"STATIC", L"Zone de texte :",
        WS_VISIBLE | WS_CHILD,
        20, 40, 200, 20,
        hwnd, NULL, hInstance, NULL
    );

    // 3.b Créer l'affichage du menu contextuel

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


    if (!hwnd) {
        MessageBox(NULL, L"Erreur : Création de la fenêtre échouée.", L"Erreur", MB_ICONERROR);
        return 0;
    }

    // 4️ Afficher la fenêtre à l'écran
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // 5️ Boucle principale de messages
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg); // Traduit les entrées clavier (ex: touches accentuées)
        DispatchMessage(&msg);  // Envoie le message à WndProc
    }

    return (int)msg.wParam;
}

// Fonction de traitement des messages (callback)
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    wchar_t szFile[260] = {};

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"Text Files\0*.txt\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    switch (uMsg) {
    case WM_CLOSE:
        DestroyWindow(hwnd);  // Quand l’utilisateur ferme la fenêtre
        break;

    case WM_DESTROY:
        PostQuitMessage(0);   // Termine la boucle principale
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == 2)
        {

        }
        else if (LOWORD(wParam) == 3)
        {

        }
        else if (LOWORD(wParam) == 4) { // Charger BMP
            if (GetOpenFileName(&ofn)) {
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
                else {
                    InvalidateRect(hwnd, NULL, TRUE); // force un repaint
                }
            }
        }

        else if (LOWORD(wParam) == 5)
        {

        }
        else if (LOWORD(wParam) == 6)
        {
            DestroyWindow(hwnd);
        }
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        if (hBmp) {
            // Créer un DC compatible
            HDC hdcMem = CreateCompatibleDC(hdc);

            // Sélectionner le bitmap dans ce DC
            HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcMem, hBmp);

            // Récupérer les dimensions du bitmap
            BITMAP bmp;
            GetObject(hBmp, sizeof(BITMAP), &bmp);

            // Copier le bitmap dans la fenêtre
            BitBlt(hdc, 0, 0, bmp.bmWidth, bmp.bmHeight, hdcMem, 0, 0, SRCCOPY);

            // Nettoyage
            SelectObject(hdcMem, hOldBmp);
            DeleteDC(hdcMem);
        }

        EndPaint(hwnd, &ps);
    }
    break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam); // Traitement par défaut
    }
    return 0;
}