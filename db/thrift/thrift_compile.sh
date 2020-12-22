rm -rf gen
mkdir gen
thrift -gen cpp -out gen rpc.thrift
oldext="cpp"
newext="cc"
dir=./gen
cd $dir
for file in $(ls . | grep .$oldext)
do
	name=$(ls $file | cut -d. -f1)
	mv $file ${name}.$newext
done
