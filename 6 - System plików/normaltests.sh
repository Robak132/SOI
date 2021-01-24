#!/bin/sh
VFS_FILE_NAME='testdisc'

echo 'Create file with 9 nodes'
./manager $VFS_FILE_NAME create 20480

echo '\nPull nonexisting 1.txt, should cause error'
./manager $VFS_FILE_NAME pull 1.txt 1.txt

echo 'And push nonexisting 1.txt, should cause error'
./manager $VFS_FILE_NAME push 1.txt 1.txt

echo '\nAdd 9 files'
i=0
while [ $i -lt 9 ]
do
    ./manager $VFS_FILE_NAME push abc.txt $i.txt >/dev/null
    true $(( i=i+1 ))
done

./manager $VFS_FILE_NAME list

echo '\nAdd another file, should cause error'
./manager $VFS_FILE_NAME push abc.txt abc.txt

echo '\nDelete half files'
i=0
while [ $i -lt 4 ]
do
    ./manager $VFS_FILE_NAME remove $i.txt >/dev/null
    true $(( i=i+1 ))
done
./manager $VFS_FILE_NAME list

echo '\nAdd another 5.txt, should cause error'
./manager $VFS_FILE_NAME push abc.txt 5.txt

echo '\nDelete nonexisting 1.txt, should cause error'
./manager $VFS_FILE_NAME remove 1.txt

sleep 5
echo '\nCreate file with 0 length'
touch zerofile.txt
./manager $VFS_FILE_NAME push zerofile.txt zerofile.txt
echo '\nCheck if added'
./manager $VFS_FILE_NAME dump
sleep 5

echo '\nRemove file'
./manager $VFS_FILE_NAME delete