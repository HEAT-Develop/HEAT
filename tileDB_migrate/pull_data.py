import sys
import numpy as np
import pandas as pd
import astropy.io.fits as fits
from matplotlib import pyplot as plt
import tiledb
import os

HEIGHT = 256
WIDTH = 384

base_dir = "/Volumes/Transcend/tileDB"
array_uri = os.path.join(base_dir, "Haya2TIRTest")
file_name = "0001.00047DD5.close.img"

# フラグメント統合（tiledb.open が遅い場合のみ True に。数分かかる場合あり）
RUN_CONSOLIDATION = True

print("1. 開始", flush=True)
if not os.path.exists(base_dir):
    print(f"エラー: {base_dir} が存在しません（ドライブがマウントされているか確認）", flush=True)
    sys.exit(1)
os.makedirs(base_dir, exist_ok=True)
print("2. base_dir OK", flush=True)

# tiledb.array_exists() はフラグメントが多いとフリーズするため、ディレクトリ存在で判定
if RUN_CONSOLIDATION and os.path.exists(os.path.join(array_uri, "__schema")):
    print("Consolidation 開始...")
    tiledb.consolidate(array_uri)
    print("Consolidate 完了。Vacuum 実行...")
    tiledb.vacuum(array_uri)
    print("Consolidation 完了")

print("3. CSV 読み込み...", flush=True)
filename_df = pd.read_csv(os.path.join(base_dir, "file_index_mapping.csv"))
print(filename_df)

file_idx = filename_df[filename_df["filename"] == file_name]["file_idx"].values[0]
print(f"4. tiledb.open 実行 (file_idx={file_idx})...", flush=True)
with tiledb.open(array_uri, "r") as A:
    data = A[file_idx, :, :]
    print(data)
    pixel = data["pixel"].reshape(HEIGHT, WIDTH)
    print(pixel.shape)
print("5. 完了", flush=True)
