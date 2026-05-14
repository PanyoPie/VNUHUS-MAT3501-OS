rm *.o
make
./fsshell

--- (In shell) ---

format
ls

mkdir home
cd home
mkdir user1
cd user1
import story1.txt
ls
cat story1.txt

cd /
import story2.txt
ls
ln story2.txt hardlink
ls
ln -s story2.txt symlink
ls