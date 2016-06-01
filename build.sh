dir=$PWD
cd $dir/src/expand
make clean
make
cd $dir/src/lz
make clean
make
cd $dir
cp src/expand/EXPAND.exe $dir/EXPAND
cp src/lz/LZ.exe $dir/LZ