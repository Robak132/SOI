#!/bin/sh
VFS_FILE_NAME='testdisc'

echo 'Create'
./manager $VFS_FILE_NAME create 40960

echo 'Write-read long test [abc_big > abc_big_download]'
./manager $VFS_FILE_NAME push abc_big.txt abcd.txt >/dev/null
i=0
while [ $i -lt 1000 ]
do
    ./manager $VFS_FILE_NAME pull abcd.txt abc_big_download.txt >/dev/null
    ./manager $VFS_FILE_NAME push abc_big_download.txt abcd.txt >/dev/null
    true $(( i=i+1 ))
done
echo 'Test finished'

./manager $VFS_FILE_NAME delete
./manager $VFS_FILE_NAME create 40960

echo 'Save-delete test'
i=0
while [ $i -lt 20 ]
do
    ./manager $VFS_FILE_NAME push abc.txt $i.txt >/dev/null
    true $(( i=i+1 ))
done

./manager $VFS_FILE_NAME dump
sleep 5

i=0
while [ $i -lt 20 ]
do
    ./manager $VFS_FILE_NAME remove $i.txt >/dev/null
    true $(( i=i+1 ))
done
./manager $VFS_FILE_NAME dump

echo 'Delete'
./manager $VFS_FILE_NAME delete