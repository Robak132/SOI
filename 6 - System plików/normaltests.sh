#!/bin/sh
VFS_FILE_NAME='testdisc'

echo 'Create file with 9 nodes'
./manager $VFS_FILE_NAME create 20480

echo 'Pull nonexisting 1.txt, should cause error'
./manager $VFS_FILE_NAME pull 1.txt 1.txt

echo 'And push nonexisting 1.txt, should cause error'
./manager $VFS_FILE_NAME push 1.txt 1.txt

echo 'Add 9 files'
for i in 0 1 2 3 4 5 6 7 8
do
    ./manager $VFS_FILE_NAME push abc.txt $i.txt >/dev/null
done

./manager $VFS_FILE_NAME list

echo 'Add another file, should cause error'
./manager $VFS_FILE_NAME push abc.txt abc.txt

echo 'Delete half files'
for i in 0 1 2 3
do
    ./manager $VFS_FILE_NAME remove $i.txt >/dev/null
done
./manager $VFS_FILE_NAME list

echo 'Add another 5.txt, should cause error'
./manager $VFS_FILE_NAME push abc.txt 5.txt

echo 'Delete nonexisting 1.txt, should cause error'
./manager $VFS_FILE_NAME remove 1.txt

echo 'Create file with 0 length'
touch zerofile.txt
./manager $VFS_FILE_NAME push zerofile.txt zerofile.txt
echo 'Check if added'
sleep 5
./manager $VFS_FILE_NAME dump | more

echo 'Remove file'
./manager $VFS_FILE_NAME delete