#!/bin/sh

rm -f faktury.sq2.old
mv faktury.sq2 faktury.sq2.old

echo ".read lib/schema.sql" | sqlite faktury.sq2
echo ".read lib/dane.sql" | sqlite faktury.sq2

