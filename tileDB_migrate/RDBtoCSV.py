import mysql.connector
import numpy as np
import pandas as pd
from matplotlib import pyplot as plt
import tiledb
conn = mysql.connector.connect(
    user='root',
    password='',
    unix_socket='/tmp/mysql_database_dev.sock',
    database='heat_db'
)
if not conn.is_connected():
    raise Exception("MySQLサーバへの接続に失敗しました")
cur = conn.cursor(dictionary=True)
DN = np.zeros((256,384))
Modified_DN = np.zeros((256,384))
Mask = np.zeros((256,384))
for i in range(96):
    pix_num = f'pix{i + 1:02d}'
    query__for_fetching_data = f"SELECT * FROM {pix_num} where img_file = '0001.00047DD5.close.img'"
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

pd.DataFrame(DN).to_csv('DN.csv', index=False, header=False)
pd.DataFrame(Modified_DN).to_csv('Modified_DN.csv', index=False, header=False)
pd.DataFrame(Mask).to_csv('Mask.csv', index=False, header=False)

df_dn = pd.DataFrame(DN)
df_mod = pd.DataFrame(Modified_DN)
df_mask = pd.DataFrame(Mask)

d1 = tiledb.Dim(name="x", domain=(0, 383), tile=64, dtype=np.int32)
d2 = tiledb.Dim(name="y", domain=(0, 255), tile=64, dtype=np.int32)
domain = tiledb.Domain(d1, d2)

attr_pixel = tiledb.Attr(name="pixel", dtype=np.float32)
attr_pixel_modified = tiledb.Attr(name="pixel_modified", dtype=np.float32)
attr_mask = tiledb.Attr(name="mask", dtype=np.float32)

schema = tiledb.ArraySchema(
    domain=domain,
    attrs=[attr_pixel, attr_pixel_modified, attr_mask],
    sparse=False
)

tiledb.DenseArray.create("1CAF.015B9C4A.close", schema)

with tiledb.DenseArray("1CAF.015B9C4A.close", mode="w") as A:
    A[:, :] = {
        "pixel": DN.T.astype(np.int32),                # (384, 256)
        "pixel_modified": Modified_DN.T.astype(np.int32),
        "mask": Mask.T.astype(np.int32),
    }
with tiledb.DenseArray("1CAF.015B9C4A.close", mode="r") as A:
    data = A[:, :]
    pixel_data = data["pixel"]
    pixel_modified_data = data["pixel_modified"]
    mask_data = data["mask"]
    print(pixel_data)
    print(pixel_modified_data)
    print(mask_data)