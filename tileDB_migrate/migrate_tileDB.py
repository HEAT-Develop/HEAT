import mysql.connector
import numpy as np
import pandas as pd
from matplotlib import pyplot as plt
import tiledb
import os

HEIGHT = 256
WIDTH = 384

conn = mysql.connector.connect(
    user='root',
    password='',
    unix_socket='/tmp/mysql_database_dev.sock',
    database='heat_db'
)
dim_filename = tiledb.Dim(name="filename", domain=None, dtype="ascii")
domain = tiledb.Domain(dim_filename)
attr_pixel = tiledb.Attr(name="pixel", dtype=np.float32)
attr_pixel_modified = tiledb.Attr(name="pixel_modified", dtype=np.float32)
attr_mask = tiledb.Attr(name="mask", dtype=np.float32)

attr_height = tiledb.Attr(name="height", dtype=np.int32)
attr_width = tiledb.Attr(name="width", dtype=np.int32)
attrs = [
        tiledb.Attr(name="pixel", dtype=np.float32, var=True),
        tiledb.Attr(name="pixel_modified", dtype=np.float32, var=True),
        tiledb.Attr(name="mask", dtype=np.float32, var=True),
        tiledb.Attr(name="height", dtype=np.int32),
        tiledb.Attr(name="width", dtype=np.int32),
    ]
schema = tiledb.ArraySchema(
    domain=domain,
    attrs=attrs,
    sparse=True
)
base_dir = "/Volumes/Transcend/tileDB"
os.makedirs(base_dir, exist_ok=True)
array_uri = os.path.join(base_dir, "Haya2TIRTest")
tiledb.Array.create(array_uri, schema)

if not conn.is_connected():
    raise Exception("MySQLサーバへの接続に失敗しました")
cur = conn.cursor(dictionary=True)

def store_to_2darrays(img_file):
    DN = np.zeros((HEIGHT,WIDTH))
    Modified_DN = np.zeros((HEIGHT,WIDTH))
    Mask = np.zeros((HEIGHT,WIDTH))
    for i in range(96):
        pix_num = f'pix{i + 1:02d}'
        query__for_fetching_data = f"SELECT * FROM {pix_num} where img_file = '{img_file}'"
        cur.execute(query__for_fetching_data)
        for fetched_line in cur.fetchall():
            x = fetched_line['x']
            y = fetched_line['y']
            pixel = fetched_line['pixel']
            pixel_modified = fetched_line['pixel_modified']
            mask = fetched_line['mask']
            DN[y,x] = np.nan if pixel is None else pixel
            Modified_DN[y,x] = np.nan if pixel_modified is None else pixel_modified
            Mask[y,x] = np.nan if mask is None else mask
    return DN, Modified_DN, Mask

def store_to_tiledb(img_file):
    DN, Modified_DN, Mask = store_to_2darrays(img_file)
    img_files = [img_file]
    with tiledb.SparseArray(array_uri, mode="w") as array:
        coords = np.array(img_files, dtype=object)
        pixel_var = np.empty(1, dtype=object)
        pixel_var[0] = DN.reshape(-1)
        pixel_modified_var = np.empty(1, dtype=object)
        pixel_modified_var[0] = Modified_DN.reshape(-1)
        mask_var = np.empty(1, dtype=object)
        mask_var[0] = Mask.reshape(-1)
        array[coords] = {
            "pixel": pixel_var,
            "pixel_modified": pixel_modified_var,
            "mask": mask_var,
            "height": np.array([HEIGHT], dtype=np.int32),
            "width": np.array([WIDTH], dtype=np.int32),
        }
    print(f"stored: {img_file}")

def main():
    query = "SELECT DISTINCT img_file FROM pix01"
    cur.execute(query)
    for fetched_line in cur.fetchall():
        img_file = fetched_line['img_file']
        store_to_tiledb(img_file)
    cur.close()
    conn.close()

if __name__ == "__main__":
    main()