#!/bin/bash
dbname="tpch1"
dir=`pwd`
opts="-U postgres -h localhost -p 5433"

echo "Creating tables"
echo "DROP DATABASE IF EXISTS $dbname" | psql $opts
echo "CREATE DATABASE $dbname WITH ENCODING='UTF8'" | psql $optsfor 

opts="-U postgres -h localhost -p 5433 -d $dbname"

psql $opts < tpch-create.sql
psql $opts < execution_stats.sql

for tbl in nation region part customer supplier partsupp orders lineitem
do
    echo "Importing table: $tbl"
    psql $opts -c "COPY $tbl FROM '$dir/$tbl.tbl' WITH (FORMAT csv, DELIMITER '|')"    
done

echo "Creating indexes"
psql $opts < tpch-alter.sql

echo "VACUUM FULL VERBOSE ANALYZE"
psql $opts -c "VACUUM FULL ANALYZE"
