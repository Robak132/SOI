#!/bin/sh
VFS_FILE_NAME='testdisc'

echo 'Create'
./manager $VFS_FILE_NAME create 40960

echo 'Write-read long test [abc_big > abc_big_download]'
./manager $VFS_FILE_NAME push abc_big.txt abcd.txt >/dev/null
for i in 1 2 3 4 5 6 7 8 9 10
do
    for j in 1 2 3 4 5 6 7 8 9 10
    do
        for k in 1 2 3 4 5 6 7 8 9 10
        do
            ./manager $VFS_FILE_NAME pull abcd.txt abc_big_d.txt >/dev/null
            ./manager $VFS_FILE_NAME remove abcd.txt >/dev/null
            ./manager $VFS_FILE_NAME push abc_big_d.txt abcd.txt >/dev/null
        done
    done
done
echo 'Results (if no answer, files are the same)'
diff abc_big.txt abc_big_d.txt
echo 'Test finished'

./manager $VFS_FILE_NAME delete
./manager $VFS_FILE_NAME create 40960

echo 'Save-delete test'
for i in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18
do
    ./manager $VFS_FILE_NAME push abc.txt $i.txt >/dev/null
done
sleep 5
./manager $VFS_FILE_NAME dump
sleep 5

for i in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18
do
    ./manager $VFS_FILE_NAME remove $i.txt >/dev/null
done
./manager $VFS_FILE_NAME dump

echo 'Delete'
./manager $VFS_FILE_NAME delete