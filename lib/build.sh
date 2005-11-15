#!/bin/sh

rm -f kesa.sq2.old
mv kesa.sq2 kesa.sq2.old

echo ".read lib/schema.sql" | sqlite kesa.sq2
echo ".read lib/data.sql" | sqlite kesa.sq2

