make

./generafile.sh
    
echo " "
echo "#######################################################################"
echo " "
echo "	./farm -h"
echo " "
./farm -h

echo " "
echo "#######################################################################"
echo " "
echo "	valgrind -s --leak-check=full --show-leak-kinds=all ./farm -d ."
echo " "
valgrind -s --leak-check=full --show-leak-kinds=all ./farm -d .

echo " "
echo "#######################################################################"
echo " "
echo "	valgrind -s --leak-check=full ./farm *.dat -t 100"
echo " "
valgrind -s --leak-check=full ./farm *.dat -t 100

echo " "
echo "#######################################################################"
echo " "
echo "	valgrind -s --leak-check=full ./farm -d . -t 1000"
echo " "
valgrind -s --leak-check=full ./farm -d . -t 1000

echo " "
echo "#######################################################################"
echo " "
echo "	valgrind -s --leak-check=full --show-leak-kinds=all ./farm -d testdir"
echo " "
valgrind -s --leak-check=full --show-leak-kinds=all ./farm -d testdir

echo " "
echo "#######################################################################"
echo " "
echo "	valgrind --leak-check=full ./farm -d testdir -n 2 -q 4 -t 100 file1.dat file2.dat"
echo " "
valgrind -s --leak-check=full ./farm -n 2 -q 4 -t 100 file1.dat file2.dat -d testdir
