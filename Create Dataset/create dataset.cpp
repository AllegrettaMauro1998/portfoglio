#include <windows.h> 
#include "framework.h"
#include "create dataset.h"
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <string>
#include <vector>
#include <iostream>
#include <fstream> 
#include <codecvt> 
#include <algorithm>

bool IsPathValid(const std::wstring& path);
std::string WStringToString(const std::wstring& wstr);

#define MAX_LOADSTRING 100
#define IDC_CONFIRM_BUTTON 105
#define IDC_NEXT_BUTTON 107
#define IDC_DELETE_BUTTON 108
#define IDC_Boxtext 101
#define IDC_SubFolderText 102
#define IDC_CROPIMAGE 103

namespace fs = std::filesystem;

// Dichiarazione delle variabili globali
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
HWND hEdit; // Aggiunta della dichiarazione per la casella di testo
HWND hEdit2;
HWND hConfirmButton; // Pulsante per confermare il percorso
HWND hNextButton; // Pulsante per la prossima immagine
HWND hDeleteButton; // Pulsante per cancellare l'immagine
HWND hSubFolderEdit; //casella per la cartella da dove iniziare la creazione
HWND hSubFolderEdit2;
HWND hCropButton; //pulsante per il crop
std::vector<std::string> imagePaths;
size_t currentImageIndex = 0;
cv::Mat currentImage;      
cv::Mat resizedImage;
bool cropMode = false;
cv::Point startPoint, endPoint; // Per il rettangolo di ritaglio
bool isDrawing = false;
double scaleWidth, scaleHeight;

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void LoadImagePaths(const std::wstring& datasetPath, const std::wstring& subFolde);
void DisplayCurrentImage(HWND hWnd);
void LogLoadedImages();

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CREATEDATASET, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 800, 600, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CREATEDATASET));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_CREATEDATASET);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId;
    switch (message) {
    case WM_CREATE:
        hEdit2 = CreateWindow(L"EDIT", L"Inserire percorso dataset", WS_CHILD | WS_VISIBLE  | ES_AUTOHSCROLL | ES_READONLY, 50, 20, 200, 30, hWnd, (HMENU)IDC_Boxtext, hInst, nullptr);
        hEdit = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 50, 45, 200, 30, hWnd, (HMENU)IDC_Boxtext, hInst, nullptr);

        // Creazione della nuova casella di testo per la sottocartella
        hSubFolderEdit2 = CreateWindow(L"EDIT", L"Sottocartella da cui iniziare (opzionale)", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_READONLY, 50, 75, 255, 30, hWnd, (HMENU)IDC_Boxtext, hInst, nullptr);
        hSubFolderEdit = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 50, 100, 200, 30, hWnd, (HMENU)IDC_SubFolderText, hInst, nullptr);
        hConfirmButton = CreateWindow(L"BUTTON", L"Conferma", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 50, 150, 100, 30, hWnd, (HMENU)IDC_CONFIRM_BUTTON, hInst, nullptr);

        // Creazione del pulsante per la prossima immagine
        hNextButton = CreateWindow(L"BUTTON", L"Prossima Immagine", WS_TABSTOP | SW_HIDE | WS_CHILD | BS_DEFPUSHBUTTON, 50, 200, 150, 30, hWnd, (HMENU)IDC_NEXT_BUTTON, hInst, nullptr);

        // Creazione del pulsante per cancellare l'immagine
        hDeleteButton = CreateWindow(L"BUTTON", L"Cancella Immagine", WS_TABSTOP | SW_HIDE | WS_CHILD | BS_DEFPUSHBUTTON, 50, 250, 150, 30, hWnd, (HMENU)IDC_DELETE_BUTTON, hInst, nullptr);
        
        hCropButton = CreateWindow(L"BUTTON", L"Ritaglia Immagine", WS_TABSTOP | SW_HIDE | WS_CHILD | BS_DEFPUSHBUTTON, 50, 300, 150, 30, hWnd, (HMENU)IDC_CROPIMAGE, hInst, nullptr);
        
        break;
    case WM_COMMAND:
        wmId = LOWORD(wParam);
        switch (wmId) {
        case IDC_CONFIRM_BUTTON: {
            WCHAR buffer[256];
            WCHAR subFolderBuffer[256];
            GetWindowText(hEdit, buffer, sizeof(buffer) / sizeof(WCHAR));
            GetWindowText(hSubFolderEdit, subFolderBuffer, sizeof(subFolderBuffer) / sizeof(WCHAR));

            std::wstring datasetPath = buffer;
            std::wstring subFolder = subFolderBuffer;

            if (subFolder == L"Inserisci sottocartella da cui iniziare") {
                subFolder.clear();  // Svuota la casella se il testo è quello di default
            }

            if (!datasetPath.empty()) {
                LoadImagePaths(datasetPath, subFolder);
                if (imagePaths.empty()) {
                    MessageBox(hWnd, L"Nessuna immagine trovata nella cartella specificata.", L"Errore", MB_OK);
                }
                else {
                    currentImageIndex = 0;
                    ShowWindow(hNextButton, SW_SHOW);
                    ShowWindow(hDeleteButton, SW_SHOW);
                    ShowWindow(hCropButton, SW_SHOW);
                    DisplayCurrentImage(hWnd);
                }
            }
            else {
                MessageBox(hWnd, L"Per favore, inserisci un nome.", L"Errore", MB_OK);
            }
            break;
        }
        case IDC_NEXT_BUTTON: {
            currentImageIndex++;
            if (currentImageIndex < imagePaths.size()) {
                DisplayCurrentImage(hWnd);
            }
            else {
                MessageBox(hWnd, L"Nessuna altra immagine disponibile.", L"Informazione", MB_OK);
                currentImageIndex--; // Ritorna all'ultimo indice valido
            }
            break;
        }
        case IDC_DELETE_BUTTON: {
            if (currentImageIndex < imagePaths.size()) {
                // Cancellazione dell'immagine
                std::string imagePath = imagePaths[currentImageIndex];
                if (fs::remove(imagePath)) {
                    MessageBox(hWnd, L"Immagine cancellata con successo.", L"Successo", MB_OK);
                    currentImageIndex = (currentImageIndex < imagePaths.size() - 1) ? currentImageIndex : currentImageIndex - 1; // Assicurati di non superare il limite
                    imagePaths.erase(imagePaths.begin() + currentImageIndex); // Rimuovo il percorso dalla lista
                    DisplayCurrentImage(hWnd); // Mostro l'immagine successiva
                }
                else {
                    MessageBox(hWnd, L"Errore durante la cancellazione dell'immagine.", L"Errore", MB_OK);
                }
            }
            break;
        }
        case IDC_CROPIMAGE: {
            if (!cropMode) {
                cropMode = true;
            }
            break;
        }
        }
        break;
    
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void onMouse(int event, int x, int y, int, void*) {
    if (!cropMode) return;

    // Calcolo la larghezza e l'altezza della finestra in cui stai visualizzando l'immagine
    cv::Rect windowRect = cv::getWindowImageRect("Immagine Corrente");
    int windowWidth = windowRect.width;  
    int windowHeight = windowRect.height;  

    // Calcolo il fattore di scala in base alla larghezza della finestra
    float scaleX = static_cast<float>(windowWidth) / currentImage.cols;
    float scaleY = static_cast<float>(windowHeight) / currentImage.rows;
    float scale = std::min(scaleX, scaleY);

    // Gestione del click sinistro del mouse
    if (event == cv::EVENT_LBUTTONDOWN) {
        isDrawing = true;
        // Scala le coordinate del mouse
        startPoint = cv::Point(static_cast<int>(x / scale), static_cast<int>(y / scale));  // Punto di inizio del rettangolo
        endPoint = startPoint;  // Punto finale inizialmente uguale al punto di inizio
    }
    // Gestione del movimento del mouse
    else if (event == cv::EVENT_MOUSEMOVE && isDrawing) {
        // Scala le coordinate del mouse
        endPoint = cv::Point(static_cast<int>(x / scale), static_cast<int>(y / scale));  // Aggiorno il punto finale con le nuove coordinate

        // Limito endPoint per non andare oltre i limiti dell'immagine
        endPoint.x = std::min(std::max(0, endPoint.x), currentImage.cols - 1);
        endPoint.y = std::min(std::max(0, endPoint.y), currentImage.rows - 1);

        // Clono l'immagine per disegnare sopra senza modificarla
        cv::Mat tempImage = currentImage.clone();

        // Disegno il rettangolo di selezione in tempo reale
        cv::rectangle(tempImage, startPoint, endPoint, cv::Scalar(0, 0, 255), 2);

        // Ridimensiono l'immagine per adattarla alla finestra di visualizzazione
        cv::Mat resizedImage;
        cv::resize(tempImage, resizedImage, cv::Size(), scale, scale);  // Ridimensiona l'immagine

        // Mostro l'immagine ridimensionata
        cv::imshow("Immagine Corrente", resizedImage);
    }
    // Gestione del rilascio del tasto sinistro del mouse (fine del disegno)
    else if (event == cv::EVENT_LBUTTONUP && isDrawing) {
        isDrawing = false;
        // Scala il punto finale
        endPoint = cv::Point(static_cast<int>(x / scale), static_cast<int>(y / scale));  // Imposta il punto finale al rilascio del mouse

        int x1 = std::min(startPoint.x, endPoint.x);
        int y1 = std::min(startPoint.y, endPoint.y);
        int width = abs(endPoint.x - startPoint.x);
        int height = abs(endPoint.y - startPoint.y);

        int imageWidth = currentImage.cols;
        int imageHeight = currentImage.rows;

        x1 = std::max(0, std::min(x1, imageWidth - 1));
        y1 = std::max(0, std::min(y1, imageHeight - 1));
        width = std::min(width, imageWidth - x1);
        height = std::min(height, imageHeight - y1);

        // Se il ritaglio è valido (ha larghezza e altezza maggiori di zero)
        if (width > 0 && height > 0) {
            cv::Rect cropRegion(x1, y1, width, height);
            cv::Mat croppedImage = currentImage(cropRegion);  

            // Ridimensiona anche l'immagine ritagliata per visualizzarla nella finestra
            cv::Mat resizedCroppedImage;
            cv::resize(croppedImage, resizedCroppedImage, cv::Size(), scale, scale);  

            // Mostro l'immagine ritagliata
            cv::imshow("Immagine Ritagliata", resizedCroppedImage);

            int result = MessageBox(
                NULL,
                L"Immagine ritagliata. Vuoi sovrascrivere l'immagine?",
                L"Conferma Salvataggio",
                MB_YESNO | MB_ICONQUESTION
            );
            if (result == IDYES) {
                cv::imwrite(imagePaths[currentImageIndex], croppedImage);  // Sovrascrivi l'immagine originale con la ritagliata
            }
        }
        cropMode = false;  
    }
}


// Funzione di verifica del percorso
bool IsPathValid(const std::wstring& path) {
    // Controllo se il percorso è vuoto
    if (path.empty()) {
        return false;
    }

    // Controllo per caratteri di separazione di Windows
    for (size_t i = 0; i < path.length(); ++i) {
        if (path[i] == L'/' || path[i] == L'\\') {
            // Controlla se il carattere precedente è un separatore (doppio backslash non valido)
            if (i > 0 && (path[i - 1] == L'/' || path[i - 1] == L'\\')) {
                return false; // Doppio separatore non valido
            }
        }
    }

    // Controllo se il percorso esiste e se è un file regolare
    return fs::exists(path) && fs::is_regular_file(path);
}

// Funzione di conversione da wstring a string
std::string WStringToString(const std::wstring& wstr) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &str[0], size_needed, nullptr, nullptr);
    return str;
}

void LoadImagePaths(const std::wstring& datasetPath, const std::wstring& subFolder)
{
    bool trovato = false;
    imagePaths.clear();
    try {
        for (const auto& entry : fs::recursive_directory_iterator(datasetPath)) {
            std::wcout << L"Controllo file: " << entry.path() << std::endl; // Log del file corrente

            if (!entry.is_regular_file()) {
                std::wcout << L"Non è un file regolare: " << entry.path() << std::endl;
                continue; // Salta se non è un file regolare
            }

            std::wstring extension = entry.path().extension().wstring();
            std::transform(extension.begin(), extension.end(), extension.begin(), ::towlower); // Normalizza l'estensione

            if (extension == L".jpg" || extension == L".png" || extension == L".jpeg" || extension == L".bmp")
            {
                std::wstring imagePathW = entry.path().wstring();

                // Verifico la validità del percorso
                if (IsPathValid(imagePathW))
                {
                    std::string imagePath = WStringToString(imagePathW);
                    if (subFolder.empty() || imagePathW.find(subFolder) != std::wstring::npos) trovato = true;
                    if (trovato == true) {
                        imagePaths.push_back(imagePath);
                        std::wcout << L"Immagine caricata: " << entry.path() << std::endl; // Log dell'immagine caricata
                    }
                }
                else {
                    std::wcout << L"Percorso non valido: " << entry.path() << std::endl;
                }
            }
            else {
                std::wcout << L"Estensione non supportata: " << entry.path() << std::endl;
            }
        }
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Errore: " << e.what() << '\n'; // Log dell'errore
    }
}

void LogLoadedImages() {
    std::ofstream logFile("loaded_images.log", std::ios_base::app);
    if (logFile.is_open()) {
        for (const auto& path : imagePaths) {
            logFile << path << std::endl;
        }
        logFile.close();
    }
}

void DisplayCurrentImage(HWND hWnd) {
    if (currentImageIndex < imagePaths.size()) {
        // Caricamento deòl'immagine
        currentImage = cv::imread(imagePaths[currentImageIndex]);
        if (!currentImage.empty()) {
            // Ottiengo le dimensioni della finestra
            RECT rect;
            GetClientRect(hWnd, &rect);
            int windowWidth = rect.right - rect.left;
            int windowHeight = rect.bottom - rect.top;

            // Ottiengo le dimensioni dell'immagine
            int imageWidth = currentImage.cols;
            int imageHeight = currentImage.rows;

            // Calcolo il fattore di scala per adattare l'immagine alla finestra
            scaleWidth = static_cast<double>(windowWidth) / imageWidth;
            scaleHeight = static_cast<double>(windowHeight) / imageHeight;
            double scale = std::min(scaleWidth, scaleHeight);  // Usa il min per mantenere l'aspetto

            // Ridimensiona l'immagine se è più grande della finestra
            cv::Mat resizedImage;
            if (scale < 1.0) {
                cv::resize(currentImage, resizedImage, cv::Size(), scale, scale);
            }
            else {
                resizedImage = currentImage; // Se l'immagine è più piccola, usa l'originale
            }

            // Mostro l'immagine ridimensionata
            cv::imshow("Immagine Corrente", resizedImage);
            cv::waitKey(1);

            cv::setMouseCallback("Immagine Corrente", onMouse);
        }
        else {
            std::wstring errorMsg = L"Errore nel caricamento dell'immagine: " +
                std::wstring(imagePaths[currentImageIndex].begin(), imagePaths[currentImageIndex].end());
            MessageBox(hWnd, errorMsg.c_str(), L"Errore", MB_OK);
        }
    }
    else {
        MessageBox(hWnd, L"Nessuna immagine disponibile.", L"Errore", MB_OK);
    }
}
