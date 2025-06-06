#from urllib.parse import quote
#import requests
#import webbrowser

#def build_url(root,opzion):
#    if(opzion=="search"):
#        parametro = input("Parametro di selezione: ")
#       return f"{root}{opzion}/index.html?page=1&itemsPerPage=1000&searchFor={quote(parametro)}"
#    if(opzion=="info"):
#        return f"{root}{opzion}/index.html?page=1&itemsPerPage=1000"

#root = "https://www.ncei.noaa.gov/erddap/"

#opzion = input("Che vuoi fare? (es. 'search'): ").strip().lower()


#url = build_url(root,opzion)
#print("URL generato:", url)

#response = requests.get(url)
#if response.ok:
#    print("Risultato ottenuto!")
#    with open("output.html", "w", encoding="utf-8") as f:
#        f.write(response.text)
#    webbrowser.open("output.html")
#else:
#    print("Errore:", response.status_code)

from erddapy import ERDDAP
import pandas as pd
import webbrowser
import xarray as xr

def selezione_dataset(e, opzion):
    if opzion == "search":
        parametro = input("Parametro di selezione: ").strip()
        # Costruisce l'URL per la ricerca
        search_url = e.get_search_url(search_for=parametro, response="html")
        print(f"URL generato per la ricerca: {search_url}")
        webbrowser.open(search_url)
    elif opzion == "info":
        info_url = f"{root}/info/index.html?page=1&itemsPerPage=1000"
        print(f"URL per info: {info_url}")
        webbrowser.open(info_url) 
    else:
        print("❌ Opzione non riconosciuta.")

def apertura(idds,typeds,f):
    if typeds not in ["tabledap", "griddap"]:
        print("❌ Tipo non valido.")
    else:
        e = ERDDAP(
            server=root,
            protocol=typeds,
            response=f)
        e.dataset_id=idds;
        try:
            if(typeds=="tabledap"):
                dstabledap(e)
            else:
                dsgriddap(e)
        except Exception as ex:
            print("❌ Errore durante il recupero dati:", ex)

def dstabledap(e):
    info_url = e.get_info_url(response='csv')
    info_df = pd.read_csv(info_url)
    variables = info_df[info_df['Row Type'] == 'variable']['Variable Name'].tolist()
    print(variables)
    key = input("Inserire una o più variabili separate da virgola: ").split(",")
    if(key==""):
        e.variables=variables
    else: 
        e.variables = [k.strip() for k in key]
    e.constraints = {}
    print(f"\n⏳ Recupero dati per '{e.dataset_id}'...")
    df = e.to_pandas()
    print("\n📊 Prime righe del dataset:")
    print(df.head())

def dsgriddap(e):
    info_url = e.get_info_url(response='csv')
    info_df = pd.read_csv(info_url)
    print(info_df.head(50))
    print("\n==== Colonne disponibili ====")
    print(info_df.columns.tolist())
    variables = info_df[info_df['Row Type'] == 'variable']['Variable Name'].tolist()
    dimensions = info_df[info_df['Row Type'] == 'dimension']['Variable Name'].tolist()
    print(variables)
    key = input("Inserire una o più variabili separate da virgola: ").split(",")
    if not any(k.strip() for k in key):
       e.variables = variables
    else:
        e.variables = [k.strip() for k in key]
    print(f"Dimensioni disponibili per constraints: {dimensions}")
    constraints = {}
    for dim in dimensions:
        use_dim = input(f"Vuoi usare constraints su '{dim}'? (y/n): ").strip().lower()
        if use_dim == "y":
            min_val = input(f"Inserire minimo per {dim}: ").strip()
            max_val = input(f"Inserire massimo per {dim}: ").strip()
        else:
            dim_info = info_df[
                (info_df['Row Type'] == 'dimension') & 
                (info_df['Variable Name'] == dim) &
                (info_df['Attribute Name'] == 'actual_range')
            ]
            actual_range = dim_info['Attribute Value'].values
            if len(actual_range) > 0:
                actual_range = actual_range[0].replace('[','').replace(']','').split()
                min_val = actual_range[0]
                max_val = actual_range[1]
            else:
                min_val = "0"
                max_val = "0"

    constraints[f"{dim}>="] = min_val
    constraints[f"{dim}<="] = max_val
    e.constraints = constraints
    e.griddap_initialize()
    url = e.get_download_url()
    ds = xr.open_dataset(url)
    print(ds)

opzion = input("Che vuoi fare? (search / info): ").strip().lower()

root = "https://www.ncei.noaa.gov/erddap"

e = ERDDAP(server=root)

selezione_dataset(e,opzion)

dataset_id=input("Inserire id del dataset: ").strip()
type_dataset=input("Inserire il tipo di dataset(tabledap' o 'griddap'): ").strip().lower()
formato=input("Inserire il formato: ").strip().lower()

apertura(dataset_id,type_dataset,formato)

