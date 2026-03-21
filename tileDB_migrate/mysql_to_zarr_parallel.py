"""
MySQL から直接 Zarr に格納するスクリプト（並列版・高速化）
- UNION ALL: 96 クエリ → 1 クエリに集約
- チャンク (100, 256, 384): 書き込み I/O 効率向上
"""
import mysql.connector
import numpy as np
import pandas as pd
import zarr
import os
from concurrent.futures import ThreadPoolExecutor, as_completed

HEIGHT = 256
WIDTH = 384
N_WORKERS = 8  # 並列ワーカー数（MySQL の max_connections 以下に）
CHUNK_SIZE = 500  # メモリ節約のため、この件数ずつ取得→書き込み
ZARR_CHUNK_FILES = 1000  # Zarr チャンクのファイル次元（書き込み効率向上）

# パス設定
base_dir = "./zarrDB"
zarr_path = os.path.join(base_dir, "tirimageinfo.zarr")


# UNION ALL 用クエリを事前生成（96 クエリ → 1 クエリに集約）
_UNION_QUERY = " UNION ALL ".join(
    [f"SELECT x, y, pixel, pixel_modified, mask FROM pix{i + 1:02d} WHERE img_file = %s" for i in range(96)]
)


def fetch_img_file_with_conn(filename, conn):
    """接続を受け取り、1 ファイル分を 1 クエリで取得（UNION ALL）"""
    cur = conn.cursor(dictionary=True)
    DN = np.zeros((HEIGHT, WIDTH))
    Modified_DN = np.zeros((HEIGHT, WIDTH))
    Mask = np.zeros((HEIGHT, WIDTH))
    cur.execute(_UNION_QUERY, (filename,) * 96)
    for row in cur.fetchall():
        x, y = row["x"], row["y"]
        DN[y, x] = np.nan if row["pixel"] is None else row["pixel"]
        Modified_DN[y, x] = np.nan if row["pixel_modified"] is None else row["pixel_modified"]
        Mask[y, x] = np.nan if row["mask"] is None else row["mask"]
    cur.close()
    return DN, Modified_DN, Mask


def worker(args):
    """ワーカー: 独立接続で 1 ファイル分を取得"""
    file_idx, filename = args
    conn = mysql.connector.connect(
        user="root",
        password="",
        unix_socket="/tmp/mysql_database_dev.sock",
        database="heat_db",
    )
    try:
        DN, Modified_DN, Mask = fetch_img_file_with_conn(filename, conn)
        return file_idx, DN, Modified_DN, Mask
    finally:
        conn.close()


def main():
    # ファイル一覧取得（メイン接続）
    conn_main = mysql.connector.connect(
        user="root",
        password="",
        unix_socket="/tmp/mysql_database_dev.sock",
        database="heat_db",
    )
    cur = conn_main.cursor(dictionary=True)
    cur.execute("SELECT DISTINCT img_file FROM tirimageinfo")
    filenames = [row["img_file"] for row in cur.fetchall()]
    n_files = len(filenames)
    cur.close()
    conn_main.close()

    print(f"MySQL → Zarr（並列）: {n_files} ファイル, {N_WORKERS} ワーカー")
    print(f"  出力: {zarr_path}")

    os.makedirs(base_dir, exist_ok=True)

    # Zarr 作成
    root = zarr.open(zarr_path, mode="w")
    zarr_chunks = (ZARR_CHUNK_FILES, HEIGHT, WIDTH)
    pixel = zarr.zeros(
        (n_files, HEIGHT, WIDTH),
        chunks=zarr_chunks,
        dtype=np.int32,
        store=root.store,
        path="pixel",
    )
    pixel_modified = zarr.zeros(
        (n_files, HEIGHT, WIDTH),
        chunks=zarr_chunks,
        dtype=np.int32,
        store=root.store,
        path="pixel_modified",
    )
    mask = zarr.zeros(
        (n_files, HEIGHT, WIDTH),
        chunks=zarr_chunks,
        dtype=np.int32,
        store=root.store,
        path="mask",
    )

    # チャンク単位で並列取得 → Zarr 書き込み
    tasks = list(enumerate(filenames))
    for chunk_start in range(0, n_files, CHUNK_SIZE):
        chunk_end = min(chunk_start + CHUNK_SIZE, n_files)
        chunk_tasks = tasks[chunk_start:chunk_end]
        results = [None] * len(chunk_tasks)

        with ThreadPoolExecutor(max_workers=N_WORKERS) as executor:
            futures = {executor.submit(worker, t): t[0] for t in chunk_tasks}
            for future in as_completed(futures):
                file_idx, DN, Modified_DN, Mask = future.result()
                idx_in_chunk = file_idx - chunk_start
                results[idx_in_chunk] = (DN, Modified_DN, Mask)

        for i, (DN, Modified_DN, Mask) in enumerate(results):
            file_idx = chunk_start + i
            pixel[file_idx, :, :] = np.nan_to_num(DN, nan=0).astype(np.int32)
            pixel_modified[file_idx, :, :] = np.nan_to_num(Modified_DN, nan=0).astype(np.int32)
            mask[file_idx, :, :] = np.nan_to_num(Mask, nan=0).astype(np.int32)

        print(f"  {chunk_end}/{n_files} 完了")

    # インデックスとファイル名の対応表を CSV に保存
    mapping_path = os.path.join(base_dir, "file_index_mapping.csv")
    pd.DataFrame({"file_idx": range(n_files), "filename": filenames}).to_csv(
        mapping_path, index=False
    )
    print(f"  file_index_mapping.csv 保存: {mapping_path}")
    print("完了")


if __name__ == "__main__":
    main()
