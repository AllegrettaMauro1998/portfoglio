# Progetto di Creazione Dataset con Ritaglio Immagini

Questo progetto consente di creare un dataset di immagini, visualizzarle, eliminarle o modificarle attraverso un'interfaccia grafica in Windows. È possibile anche effettuare operazioni di ritaglio delle immagini.

## Requisiti

Per utilizzare questo programma, è necessario avere installato:

- **Microsoft Visual Studio** con supporto per C++.
- **OpenCV** per la gestione delle immagini.
- **Windows SDK** per la creazione di applicazioni Windows.

Assicurati di avere le librerie e i file di intestazione necessari per OpenCV e Windows SDK correttamente configurati nel tuo ambiente di sviluppo.

## Funzionalità

### 1. **Caricamento e Visualizzazione Immagini**
   - Puoi caricare un dataset di immagini da una cartella specificata. Le immagini supportate includono i formati `.jpg`, `.png`, `.jpeg`, `.bmp`.
   - Le immagini vengono visualizzate adattate alle dimensioni della finestra dell'applicazione.

### 2. **Navigazione Immagini**
   - Una volta caricate le immagini, puoi navigare tra di esse con il pulsante "Prossima Immagine".
   - Se non ci sono più immagini da visualizzare, verrà mostrato un messaggio di errore.

### 3. **Eliminazione Immagini**
   - Puoi eliminare l'immagine corrente dal dataset con il pulsante "Cancella Immagine".

### 4. **Ritaglio Immagini**
   - Puoi ritagliare un'immagine selezionando una porzione di essa tramite il mouse. Una volta effettuato il ritaglio, puoi scegliere di sovrascrivere l'immagine originale o annullare l'operazione.
   - Il ritaglio viene effettuato in tempo reale sulla finestra di visualizzazione.

### 5. **Log delle Immagini Caricate**
   - Viene creato un file di log (`loaded_images.log`) che contiene i percorsi delle immagini caricate.

## Guida all'uso

1. **Inserisci il percorso della cartella**: All'avvio, inserisci il percorso della cartella contenente le immagini che desideri caricare. Puoi anche specificare una sottocartella da cui iniziare il caricamento.
   
2. **Carica le immagini**: Premi il pulsante "Conferma" per caricare le immagini dalla cartella selezionata.

3. **Naviga tra le immagini**: Usa il pulsante "Prossima Immagine" per visualizzare l'immagine successiva.

4. **Elimina un'immagine**: Se vuoi eliminare l'immagine corrente, clicca su "Cancella Immagine".

5. **Ritaglia un'immagine**: Per attivare la modalità di ritaglio, clicca su "Ritaglia Immagine". Poi, seleziona la porzione da ritagliare utilizzando il mouse.

6. **Salva il ritaglio**: Una volta effettuato il ritaglio, scegli se sovrascrivere l'immagine originale con il ritaglio effettuato.

## Funzioni Principali

- **LoadImagePaths**: Carica tutte le immagini da una cartella e le salva in una lista.
- **DisplayCurrentImage**: Visualizza l'immagine corrente nella finestra dell'applicazione.
- **onMouse**: Gestisce gli eventi del mouse per il ritaglio dell'immagine.
- **IsPathValid**: Verifica che il percorso della cartella o del file sia valido.
- **WStringToString**: Converte una stringa `std::wstring` in `std::string`.
- **LogLoadedImages**: Registra i percorsi delle immagini caricate in un file di log.

## Compilazione

1. **Installa OpenCV**: Scarica e installa OpenCV da [https://opencv.org/](https://opencv.org/).
2. **Configura il tuo ambiente di sviluppo**: Assicurati che Visual Studio sia configurato per utilizzare OpenCV. Devi configurare i percorsi delle librerie e dei file di intestazione correttamente.
3. **Compila il progetto**: Apri il progetto in Visual Studio e compila il programma.

## Licenza

Questo progetto è rilasciato sotto la **Licenza MIT**.
