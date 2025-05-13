from tempfile import tempdir
from datetime import datetime, timedelta
import numpy as np
import pandas as pd
import xarray as xr
import matplotlib.pyplot as plt
import seaborn as sns
from netCDF4 import Dataset
from pathlib import Path

pf = pd.read_csv("C:/Users/utente/Onedrive/Desktop/DCCV_TAXDISOCCU1_11042025172944444.csv");
 
print(pf)

pf.columns = pf.columns.str.strip('"')

df_2024 = pf[pf["Seleziona periodo"]=="2024"]

tabella = df_2024.groupby(["Territorio"])["Value"].mean().reset_index()

print (tabella)

tabella = df_2024.groupby(["Territorio","Sesso"])["Value"].mean().reset_index()

print (tabella)

tabella_filtrata = tabella[tabella["Sesso"].isin(["maschi","femmine"])]

sns.set(style="whitegrid")

plt.figure(figsize=(12,6))

sns.barplot(
    data=tabella_filtrata,
    x="Territorio",
    y="Value",
    hue="Sesso",
    palette="pastel"
)

plt.title("Tasso medio di disoccupazione per genere e territorio (2024)", fontsize=14)
plt.ylabel("Tasso di disoccupazione (%)")
plt.xlabel("Territorio")
plt.xticks(rotation=45)
plt.tight_layout()
plt.show()
plt.savefig("grafico_disoccupazione.png", dpi= 300)

tabella = df_2024.groupby(["Territorio","Classe di eta"])["Value"].mean().reset_index()

pivot = tabella.pivot(index="Classe di eta", columns="Territorio", values="Value")

plt.figure(figsize=(10, 6))
sns.heatmap(pivot, annot=True, fmt=".1f", cmap="YlGnBu")

plt.title("Tasso medio di occupazione per eta e territorio (2024)")
plt.xlabel("Territorio")
plt.ylabel("Classe di eta")
plt.tight_layout()
plt.show()

df_anni = pf[pf["Seleziona periodo"].isin(["2024","2023"])]

tabella = df_anni.groupby(["Territorio", "Sesso", "Seleziona periodo"])["Value"].mean().reset_index()

df_2023 = tabella[tabella["Seleziona periodo"] == "2023"].rename(columns={"Value": "Value_2023"})

df_2024 = tabella[tabella["Seleziona periodo"] == "2024"].rename(columns={"Value": "Value_2024"})

merged = pd.merge(df_2023,df_2024, on =["Territorio", "Sesso"])

merged["Variazione_%"] = ((merged["Value_2024"] - merged["Value_2023"]) / merged["Value_2023"]) * 100

print(merged[["Territorio", "Sesso", "Variazione_%"]])

df_sesso = pf[pf["Sesso"].isin(["Maschi", "Femmine"])]

tabella = df_sesso.groupby(["Territorio", "Sesso"])["Value"].mean().reset_index()

maschi = tabella[tabella["Sesso"] == "Maschi"].rename(columns={"Value": "Value_maschi"})

femmine = tabella[tabella["Sesso"] == "Femmine"].rename(columns={"Value": "Value_femmine"})

merge = pd.merge(maschi, femmine, on="Territorio", suffixes=("_maschi", "_femmine"))

condizione = merge[merge["Value_femmine"] > (merge["Value_maschi"] * 1.5)]

print(condizione)

