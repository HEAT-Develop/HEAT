# MySQLデータベース操作完全ガイド - heat_protoプロジェクト

## 目次

1. [プロジェクト概要](#プロジェクト概要)
2. [MySQL接続からデータベースアクセスまで](#mysql接続からデータベースアクセスまで)
3. [データベースの構造とデータ内容](#データベースの構造とデータ内容)
4. [データのインポート方法](#データのインポート方法)
5. [用語解説](#用語解説)
6. [トラブルシューティング](#トラブルシューティング)
7. [実践的な操作手順](#実践的な操作手順)

---

## プロジェクト概要

### プロジェクト名
`heat_proto` - Hayabusa2 Thermal Infrared Imager (HEAT) プロジェクトのプロトタイプ

### データベース構成

- **データベース名**: `HEAT_DB`, `TIRI_pre`
- **ストレージエンジン**: InnoDB
- **元のMySQLバージョン**: 8.0.45
- **現在のMySQLバージョン**: 9.6.0（ポート3307）

### 主要テーブル

- **ピクセルデータテーブル**: `pix01` ～ `pix96`（96個のテーブル）
- **画像情報テーブル**: `tirimageinfo`
- **T.inertia関連テーブル**: `t_key`, `t_tir`, `t_inpol`, `t_best_fit`, `t_roughnessmodel`

---

## MySQL接続からデータベースアクセスまで

### 1. MySQLサーバーの起動・停止

#### サーバーの起動

```bash
# MySQLサービスの起動
brew services start mysql

# または、特定のバージョンを使用する場合
brew services start mysql@8.4
brew services start mysql@8.0
```

#### サーバーの停止

```bash
# mysqladminを使用した正常停止（推奨）
MYSQL_PWD='パスワード' mysqladmin -S /tmp/mysql_database_dev.sock -u root shutdown

# または、Homebrewサービス経由で停止
brew services stop mysql
```

#### サーバーの状態確認

```bash
# TCPポートの確認
nc -vz 127.0.0.1 3307

# UNIXソケットの確認
ls -l /tmp/mysql_database_dev.sock

# プロセスの確認
ps aux | grep mysqld

# Homebrewサービスの確認
brew services list | grep mysql
```

### 2. 接続設定ファイル

#### `assets/heat_old.json`

MySQL接続情報を定義する設定ファイル：

```json
{
    "Database": {
        "user": "demo",
        "password": "demo2_DEMO",
        "host": "127.0.0.1",
        "port": 3307,
        "db_connect": [
            {
                "table_name": "HEAT_DB",
                "conn_name": "default"
            },
            {
                "table_name": "TIRI_pre",
                "conn_name": "konkontiri"
            }
        ]
    }
}
```

### 3. MySQLユーザーの作成と権限設定

#### ユーザーの作成

```sql
-- TCP接続用ユーザー（127.0.0.1）
CREATE USER IF NOT EXISTS 'demo'@'127.0.0.1' 
IDENTIFIED WITH caching_sha2_password BY 'demo2_DEMO';

-- ローカルソケット接続用ユーザー（localhost）
CREATE USER IF NOT EXISTS 'demo'@'localhost' 
IDENTIFIED WITH caching_sha2_password BY 'demo2_DEMO';
```

#### データベースへの権限付与

```sql
-- HEAT_DBスキーマへの権限付与
GRANT ALL PRIVILEGES ON `HEAT_DB`.* TO 'demo'@'127.0.0.1';
GRANT ALL PRIVILEGES ON `HEAT_DB`.* TO 'demo'@'localhost';

-- TIRI_preスキーマへの権限付与
GRANT ALL PRIVILEGES ON `TIRI_pre`.* TO 'demo'@'127.0.0.1';
GRANT ALL PRIVILEGES ON `TIRI_pre`.* TO 'demo'@'localhost';

-- 権限の反映
FLUSH PRIVILEGES;
```

### 4. 接続テスト

#### TCP接続のテスト

```bash
# パスワードを環境変数で指定
MYSQL_PWD='demo2_DEMO' mysql -h 127.0.0.1 -P 3307 -u demo \
  --protocol=TCP --connect-timeout=3 \
  -e "SELECT CURRENT_USER(), VERSION();"

# 対話的に接続
mysql -h 127.0.0.1 -P 3307 -u demo -p
```

#### UNIXソケット接続のテスト

```bash
# パスワードを環境変数で指定
MYSQL_PWD='demo2_DEMO' mysql -S /tmp/mysql_database_dev.sock -u demo \
  --connect-timeout=3 \
  -e "SELECT CURRENT_USER(), VERSION();"

# 対話的に接続
mysql -S /tmp/mysql_database_dev.sock -u demo -p
```

### 5. データベーススキーマの作成

```sql
-- HEAT_DBスキーマの作成
CREATE DATABASE IF NOT EXISTS `HEAT_DB` 
CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

-- TIRI_preスキーマの作成
CREATE DATABASE IF NOT EXISTS `TIRI_pre` 
CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

-- データベース一覧の確認
SHOW DATABASES;
```

---

## データベースの構造とデータ内容

### pix01テーブルのデータ内容

#### テーブル構造

| 列名 | データ型 | 説明 |
|------|---------|------|
| `img_file` | VARCHAR(40) | 画像ファイル名 |
| `x` | INT | X座標（0-31） |
| `y` | INT | Y座標（0-31） |
| `pixel` | INT | ピクセル値（DN値） |
| `pixel_modified` | INT | 修正後のピクセル値 |
| `mask` | TINYINT(1) | マスクフラグ（0または1） |

#### データ統計

| 項目 | 値 |
|------|-----|
| **総レコード数** | 10,006,528行 |
| **ユニークな画像ファイル数** | 9,516ファイル |
| **X座標の範囲** | 0 ～ 31 |
| **Y座標の範囲** | 0 ～ 31 |
| **ピクセル値の範囲** | 32,816 ～ 58,557 |

#### データのソース

**元のデータベース**:
- **場所**: `/Volumes/Transcend/MySQL80/mac/arm/mysql8/heat_db/pix01.ibd`
- **作成時刻**: 2023年6月12日 18:08:46
- **最後の更新**: 2023年11月21日 17:25
- **データサイズ**: 約1.5GB（`.ibd`ファイル）

**データの出所**:
- Hayabusa2ミッションの熱赤外カメラ（TIR）で撮影された画像データから抽出されたピクセル値
- 画像ファイル名の形式: `0001.0004006A.close.img`（シーケンス番号、16進数ID、観測モード）

#### テーブル選択ロジック

`src/calibration.py`の`_judgeTableName`メソッド:

```python
def _judgeTableName(self, x: int, y: int) -> str:
    """Determine pixel table name based on coordinates"""
    # 32x32 blocks for 384x256 image -> 12 columns, 8 rows = 96 tables
    row = y // 32  # 0-7
    col = x // 32  # 0-11
    table_num = row * 12 + col + 1  # 1-96
    return f"{self.schema}.pix{table_num:02d}"
```

**説明**:
- 384×256ピクセルの画像を32×32のブロックに分割
- 12列×8行 = 96個のテーブル（`pix01`～`pix96`）
- 座標（x, y）に基づいて、対応するテーブルを選択

### データの更新情報

#### テーブルの作成時刻

すべてのテーブルは**2023年6月12日～14日**に作成されています：

| テーブル名 | 作成時刻 |
|-----------|----------|
| pix01 | 2023-06-12 18:08:46 |
| pix02 | 2023-06-12 18:25:01 |
| ... | ... |
| tirimageinfo | 2023-06-14 10:37:04 |

#### ファイルシステムのタイムスタンプ

- **pix01.ibd**: 2023年6月12日 18:25:52
- **tirimageinfo.ibd**: 2023年6月14日 10:38:43
- **最新の更新**: 2023年11月21日 17:25（`pix84`～`pix96`など）

---

## データのインポート方法

### 方法1: mysqldumpを使用（推奨）

#### エクスポート

```bash
# 元のMySQLサーバーからエクスポート
mysqldump -S /tmp/mysql_8.0.33.sock -u root \
  --databases HEAT_DB \
  --tables pix01 \
  --single-transaction \
  --routines \
  --triggers \
  > /tmp/pix01_dump.sql
```

**結果**: ✅ エクスポート成功
- **ファイルサイズ**: 447MB
- **ファイルパス**: `/tmp/pix01_dump.sql`

#### インポート

```bash
# 現在のMySQLサーバーにインポート
mysql -h 127.0.0.1 -P 3307 -u demo -p HEAT_DB < /tmp/pix01_dump.sql
```

**結果**: ✅ インポート成功
- **総レコード数**: 10,006,528行

### 方法2: .ibdファイルからの直接インポート

#### 前提条件

- `.ibd`ファイルと`.cfg`ファイルが必要
- MySQLバージョンが一致している必要がある

#### 手順

```sql
-- 1. テーブルを作成（空のテーブル）
CREATE TABLE `pix01` (
  `img_file` VARCHAR(40) NOT NULL,
  `x` INT NOT NULL,
  `y` INT NOT NULL,
  `pixel` INT NOT NULL,
  `pixel_modified` INT NOT NULL,
  `mask` TINYINT(1) NOT NULL,
  PRIMARY KEY (`img_file`, `x`, `y`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 2. テーブルスペースを破棄
ALTER TABLE `pix01` DISCARD TABLESPACE;

-- 3. .ibdファイルと.cfgファイルをコピー
-- （MySQLのデータディレクトリに配置）

-- 4. テーブルスペースをインポート
ALTER TABLE `pix01` IMPORT TABLESPACE;
```

#### エラー例

```
ERROR 1808 (HY000): Schema mismatch (Clustered index validation failed. 
Because the .cfg file is missing...)
```

**原因**: `.cfg`ファイルが存在しないため、MySQLがテーブル定義を検証できない

### 方法3: ibd2sdiを使用（テーブル構造のみ）

#### SDIの抽出

```bash
# .ibdファイルからSDIを抽出
cd /Volumes/Transcend/database_dev/mysql/HEAT_DB
/opt/homebrew/bin/ibd2sdi pix01.ibd > pix01.sdi.json
```

**結果**: ✅ SDI抽出成功
- **ファイル**: `pix01.sdi.json`
- **内容**: テーブル構造情報（列定義、インデックス、PRIMARY KEYなど）

**制限**:
- テーブル構造のみを抽出できる
- データ自体は取得できない

### 残りのpix02～pix96テーブルのインポート

#### 個別にエクスポート・インポート（推奨）

```bash
# pix02のエクスポート
mysqldump -S /tmp/mysql_8.0.33.sock -u root \
  --databases HEAT_DB \
  --tables pix02 \
  --single-transaction \
  > /tmp/pix02_dump.sql

# pix02のインポート
mysql -h 127.0.0.1 -P 3307 -u demo -p HEAT_DB < /tmp/pix02_dump.sql

# 確認
mysql -h 127.0.0.1 -P 3307 -u demo -p -D HEAT_DB \
  -e "SELECT COUNT(*) AS total_rows FROM pix02;"
```

#### バッチスクリプトで一括処理

```bash
#!/bin/bash
# export_all_pix.sh

for i in {02..96}; do
    echo "Exporting pix${i}..."
    mysqldump -S /tmp/mysql_8.0.33.sock -u root \
      --databases HEAT_DB \
      --tables pix${i} \
      --single-transaction \
      > /tmp/pix${i}_dump.sql 2>&1
    
    if [ $? -eq 0 ]; then
        echo "  ✓ pix${i} exported successfully"
    else
        echo "  ✗ pix${i} export failed"
    fi
done
```

---

## 用語解説

### mysqldumpとは

`mysqldump`は、MySQLデータベースの**バックアップとエクスポート**を行うための標準的なコマンドラインツールです。

**主な用途**:
1. データベースのバックアップ
2. データベースの移行
3. データのエクスポート
4. スキーマの複製

**出力されるSQLファイルの内容**:
- 設定とヘッダー
- テーブルの削除と作成（`DROP TABLE`, `CREATE TABLE`）
- データの挿入（`INSERT INTO`文）
- 設定の復元

### dumpとは

**dump（ダンプ）**は、データベースの**データをファイルに出力（エクスポート）する**操作です。

データベースの**構造（スキーマ）とデータ**を、**SQL文の形式**でファイルに保存します。

**主な特徴**:
- ✅ テキスト形式（SQL文）
- ✅ 可読性が高い
- ✅ 異なる環境間で移植可能
- ✅ バックアップやデータ移行に使用

### information_schemaとは

`information_schema`は、MySQLデータベースの**メタデータ（データに関する情報）**を提供する**読み取り専用のデータベース**です。

**主な特徴**:
- 読み取り専用（データ変更不可）
- メモリ内のビュー（実際のテーブルではない）
- 標準SQL準拠
- すべてのユーザーがアクセス可能

**主なテーブル（ビュー）**:
- `SCHEMATA`: データベースの一覧
- `TABLES`: テーブルの一覧とメタデータ
- `COLUMNS`: 列（カラム）の定義情報
- `STATISTICS`: インデックスと統計情報

**注意事項**:
- InnoDBテーブルの`UPDATE_TIME`は`NULL`になることが多い
- `TABLE_ROWS`は概算値

### .cfgファイルとは

`.cfg`ファイルは、**テーブルスペースのメタデータファイル**です。

**内容**:
- テーブル構造の詳細な情報
- インデックスの定義
- カラムの型情報
- その他のメタデータ

**生成方法**:
```sql
-- テーブルをエクスポート可能な状態にする
FLUSH TABLES pix01 FOR EXPORT;

-- この時点で、以下のファイルが生成される：
-- - pix01.ibd（テーブルデータ）
-- - pix01.cfg（メタデータ）

-- ファイルをコピー後、ロックを解除
UNLOCK TABLES;
```

**必要性**:
- `ALTER TABLE ... IMPORT TABLESPACE`を使用する場合: ✅ **必要**
- `mysqldump`を使用する場合: ❌ 不要
- `ibd2sdi`を使用する場合: ❌ 不要（構造のみ）

---

## トラブルシューティング

### よくあるエラーと対処法

#### `ERROR 1146 (42S02): Table 'heat_db.heat_db' doesn't exist`

**原因**: データベース名とテーブル名を混同している

**対処法**:
```sql
USE HEAT_DB;
SHOW TABLES;
DESCRIBE pix01;
```

#### `ERROR 1814 (HY000): Tablespace is discarded for table, 'pix01'`

**原因**: テーブルスペースが破棄（DISCARD）された状態

**対処法**:
```sql
-- 方法1: テーブルスペースをインポート
ALTER TABLE pix01 IMPORT TABLESPACE;

-- 方法2: テーブルを再作成してmysqldumpからインポート（推奨）
DROP TABLE IF EXISTS pix01;
-- その後、mysqldumpからインポート
```

#### `ERROR 1808 (HY000): Schema mismatch`

**原因**: `.cfg`ファイルが存在しないか、MySQLバージョンが一致していない

**対処法**: `mysqldump`を使用した方法に切り替える

#### `ERROR 1045 (28000): Access denied`

**原因**: パスワードが正しくない、またはユーザーが指定したホストから接続不可

**対処法**:
- パスワードを確認
- ユーザーが指定したホストから接続可能か確認（`@'127.0.0.1'` vs `@'localhost'`）
- `FLUSH PRIVILEGES;`を実行

#### `ERROR 2003 (HY000): Can't connect to MySQL server`

**原因**: MySQLサーバーが起動していない

**対処法**:
- MySQLサーバーが起動しているか確認
- ポート番号が正しいか確認
- ファイアウォール設定を確認

---

## 実践的な操作手順

### pix01テーブルのインポート（成功例）

#### 1. 元のMySQLサーバーからエクスポート

```bash
mysqldump -S /tmp/mysql_8.0.33.sock -u root \
  --databases HEAT_DB \
  --tables pix01 \
  --single-transaction \
  --routines \
  --triggers \
  > /tmp/pix01_dump.sql
```

**結果**: ✅ エクスポート成功（447MB）

#### 2. 現在のMySQLサーバーにインポート

```bash
# テーブルを削除（既存の場合）
mysql -h 127.0.0.1 -P 3307 -u demo -p'demo2_DEMO' -D HEAT_DB \
  -e "DROP TABLE IF EXISTS pix01;"

# インポート
mysql -h 127.0.0.1 -P 3307 -u demo -p'demo2_DEMO' HEAT_DB < /tmp/pix01_dump.sql
```

**結果**: ✅ インポート成功（10,006,528行）

#### 3. データの確認

```sql
USE HEAT_DB;

-- レコード数の確認
SELECT COUNT(*) AS total_rows FROM pix01;

-- 最初の10行を表示
SELECT * FROM pix01 LIMIT 10;

-- ユニークな画像ファイル数の確認
SELECT COUNT(DISTINCT img_file) AS unique_images FROM pix01;

-- 座標の範囲確認
SELECT 
    MIN(x) AS min_x, MAX(x) AS max_x,
    MIN(y) AS min_y, MAX(y) AS max_y 
FROM pix01;
```

### オリジナルデータベースの.ibdファイルアクセス

#### ファイル確認

```bash
# ディレクトリ内の.ibdファイル一覧
cd /Volumes/Transcend/database_dev/mysql/HEAT_DB
ls -lh *.ibd | head -20

# 特定のファイルの確認
ls -lh pix01.ibd
file pix01.ibd
```

#### SDIの抽出

```bash
# ibd2sdiツールでSDIを抽出
/opt/homebrew/bin/ibd2sdi pix01.ibd > pix01.sdi.json

# 抽出結果を確認
head -50 pix01.sdi.json
```

#### ファイルシステムのタイムスタンプ確認

```bash
# 最新に更新された.ibdファイルを確認
ls -ltu /Volumes/Transcend/MySQL80/mac/arm/mysql8/heat_db/*.ibd | head -10

# 詳細なタイムスタンプ
stat -f "%Sm" -t "%Y-%m-%d %H:%M:%S" \
  /Volumes/Transcend/MySQL80/mac/arm/mysql8/heat_db/pix01.ibd
```

---

## データの更新情報の格納場所

### 1. information_schema.TABLES

**格納される情報**:
- `CREATE_TIME`: テーブルの作成時刻（✅ 正確）
- `UPDATE_TIME`: テーブルの更新時刻（❌ InnoDBでは`NULL`になることが多い）

**確認方法**:
```sql
SELECT TABLE_NAME, CREATE_TIME, UPDATE_TIME
FROM information_schema.TABLES
WHERE TABLE_SCHEMA = 'heat_db';
```

### 2. ファイルシステムのタイムスタンプ（.ibdファイル）

**格納される情報**:
- ファイルが最後に書き込まれた時刻（`mtime`）
- ファイルが作成された時刻（`ctime`）

**特徴**:
- InnoDBテーブルでは、この情報が最も信頼できる
- データの変更だけでなく、`OPTIMIZE TABLE`やインデックスの再構築でも更新される

**確認方法**:
```bash
ls -ltu /path/to/mysql/data/heat_db/*.ibd | head -10
stat -f "%Sm" -t "%Y-%m-%d %H:%M:%S" /path/to/mysql/data/heat_db/pix01.ibd
```

### 3. .ibdファイル内のSDI（MySQL 8.0以降）

**格納される情報**:
- テーブル構造（列定義、インデックス、制約など）
- テーブルの作成時刻
- メタデータの完全な情報

**確認方法**:
```bash
/opt/homebrew/bin/ibd2sdi /path/to/mysql/data/heat_db/pix01.ibd > pix01.sdi.json
cat pix01.sdi.json | jq '.[1].object.dd_object'
```

---

## まとめ

### 主要な操作手順

1. **MySQLサーバーの起動**: `brew services start mysql`
2. **ユーザーの作成と権限設定**: `CREATE USER`, `GRANT`
3. **データベースの作成**: `CREATE DATABASE`
4. **データのインポート**: `mysqldump`を使用（推奨）
5. **データの確認**: `SELECT`文で確認

### 重要なポイント

- **`.cfg`ファイル**: `ALTER TABLE ... IMPORT TABLESPACE`を使用する場合のみ必要
- **`mysqldump`**: `.cfg`ファイルがなくても使用可能（推奨）
- **`information_schema`**: メタデータを取得するための読み取り専用データベース
- **ファイルシステムのタイムスタンプ**: InnoDBテーブルの更新時刻を確認する最も信頼できる方法

### このプロジェクトでの実際の結果

- ✅ `pix01`テーブルのインポート成功（10,006,528行）
- ✅ `mysqldump`を使用した方法で成功
- ❌ `.ibd`ファイルからの直接インポートは`.cfg`ファイルがないため失敗
- ✅ `ibd2sdi`でテーブル構造の抽出に成功

---

## 参考資料

### 関連ファイル

- `mysql_connection_guide.md`: MySQL接続の詳細ガイド
- `mysqldump_explanation.md`: mysqldumpの詳細説明
- `pix01_data_analysis.md`: pix01テーブルのデータ分析
- `ibd_import_cfg_requirement.md`: .ibdファイルインポートと.cfgファイルの必要性
- `original_db_ibd_access_history.md`: オリジナルデータベースの.ibdファイルアクセス履歴
- `pre_2024_data_analysis.md`: 2024年以前のデータ分析

### 接続情報

- **現在のMySQLサーバー**: `127.0.0.1:3307`（MySQL 9.6.0）
- **元のMySQLサーバー**: `/tmp/mysql_8.0.33.sock`（MySQL 8.0.45）
- **データディレクトリ**: `/opt/homebrew/var/mysql/`
- **バックアップディレクトリ**: `/Volumes/Transcend/database_dev/mysql/HEAT_DB/`

---

**最終更新**: 2026年2月9日
