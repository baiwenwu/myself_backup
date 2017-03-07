CountPath=/home/xiaobai/coding/laborator/compress_try/newtry20170306/fasDecompressOrg/
DATAPATH=./50MB/

data1=${DATAPATH}"dblp.xml"
data2=${DATAPATH}"dna"
data3=${DATAPATH}"kernel"
data4=${DATAPATH}"para"
data5=${DATAPATH}"pitches"
data6=${DATAPATH}"sources"
#data7=${DATAPATH}"proteins"
data7=${DATAPATH}"world_leaders"
data8=${DATAPATH}"influenza"
data9=${DATAPATH}"english.500MB"


for data in $data1 $data2 $data3 $data4 $data5 $data6 $data7 $data8 $data9 ;do
	cd $CountPath
	echo $data
	./wzip -c $data
done