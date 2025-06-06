from datetime import datetime, timedelta
from pathlib import Path
from dask.distributed import Client
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import xarray as xr
import dask.array as da
import zarr



def load_netcdf_lazy(path):
    return xr.open_dataset(path, chunks={})

def save_to_zarr(dataset, path):
    dataset.to_zarr(path, mode='w')

def load_zarr(path):
    #per accorpare tutti gli open in un unica soluzione si può usare xr.open_dataset(... engine="") che permette di cambiare il tipo di apertura in real time
    return xr.open_zarr(path)

def main():
    dir_root='C://Users//utente//OneDrive//Desktop'
    input_netcdf_path = Path(dir_root + '//goes19_abi_full_disk_lat_lon.nc')
    zarr_output_path = Path(dir_root + '//goes19_abi_full_disk_lat_lon.zarr')
    synthetic_netcdf_path = Path('forecast_model.nc')
    real_netcdf_path = Path(dir_root+'//NARR_19930313_0000.nc')

    client = Client()

    if input_netcdf_path.exists():
        ds_lazy = load_netcdf_lazy(input_netcdf_path)
        print(ds_lazy)
        print("Latitudine:", ds_lazy['latitude'])

        lat_data = ds_lazy['latitude']
        valid_mask = (~np.isnan(lat_data)).compute()
        lat_validi = lat_data.values[valid_mask]
        print("Latitudini valide:", lat_validi)

        save_to_zarr(ds_lazy, zarr_output_path)
        ds_zarr = load_zarr(zarr_output_path)
        print("Dataset caricato da Zarr:", ds_zarr)
    else:
        print("File NetCDF di input non trovato.")

    ds_prova = xr.Dataset(
        {"foo": (("x", "y"), np.random.rand(4, 5))},
        coords={
            "x": [10, 20, 30, 40],
            "y": pd.date_range("2000-01-01", periods=5),
            "z": ("x", list("abcd")),
        },
    )
    print(ds_prova)
    print("Valori x:", ds_prova.x.values)
    print("Coordinate:", ds_prova.coords)
    print("Media x:", ds_prova['x'].values.mean())
    print("Minimo x:", ds_prova['x'].values.min())

    start = datetime.utcnow().replace(hour=22, minute=0, second=0, microsecond=0)
    times = np.array([start + timedelta(hours=h) for h in range(13)])
    x = np.arange(-150, 153, 3)
    y = np.arange(-100, 100, 3)
    press = np.array([1000, 925, 850, 700, 500, 300, 250])
    temps = np.random.randn(times.size, press.size, y.size, x.size)

    nc2 = xr.Dataset(
        {"Temperature": (("forecast_time", "pressure", "y", "x"), temps)},
        coords={
            "forecast_time": times,
            "pressure": press,
            "x": x,
            "y": y,
        },
    )

    nc2.to_netcdf(synthetic_netcdf_path, format='NETCDF4_CLASSIC')
    print("File NetCDF sintetico salvato.")

    if real_netcdf_path.exists():
        ds = xr.open_dataset(real_netcdf_path, chunks={})
        print(ds.isobaric1)
        ds_1000 = ds.sel(isobaric1=1000.0)
        print("Dati isobarici a 1000hPa:", ds_1000)
        print("Temperatura a 1000hPa:", ds_1000.Temperature_isobaric.values)

        u_winds = ds['u-component_of_wind_isobaric']
        print("Deviazione standard del vento U:", u_winds.std(dim=['x', 'y']).compute())

        temps = ds['Temperature_isobaric']
        co_temps = temps.sel(x=slice(-182, 424), y=slice(-1450, -990))
        prof = co_temps.mean(dim=['x', 'y']).compute()
        print("Profilo medio:", prof)
        prof.plot()
        plt.title("Profilo medio temperatura")
        plt.show()

        temps.sel(isobaric1=1000).compute().plot()
        plt.xlabel("Coordinate X")
        plt.ylabel("Coordinate Y")
        plt.title("Temperatura a 1000hPa")
        plt.show()

        ds.Temperature_isobaric.mean(dim='x').compute().plot(size=7)
        plt.title("Media su X")
        plt.show()
    else:
        print("File NetCDF reale non trovato.")
    client.close()

if __name__ == '__main__':
    main()