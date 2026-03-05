
## Mysql server の起動()
brew services start mysql
## mysqlログイン（UNIXソケットファイルで接続、root権限）
mysql -S /tmp/mysql_database_dev.sock -u root