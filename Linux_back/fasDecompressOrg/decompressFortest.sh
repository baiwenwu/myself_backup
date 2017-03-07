CountPath=/home/xiaobai/coding/laborator/compress_try/newtry20170306/fasDecompressOrg/
DATAPATH=./50MB/

data1=${DATAPATH}"dblp.xml.wz"
data2=${DATAPATH}"dna.wz"
data3=${DATAPATH}"kernel.wz"
data4=${DATAPATH}"para.wz"
data5=${DATAPATH}"pitches.wz"
data6=${DATAPATH}"sources.wz"
#data7=${DATAPATH}"proteins"
data7=${DATAPATH}"world_leaders.wz"
data8=${DATAPATH}"influenza.wz"
data9=${DATAPATH}"english.500MB.wz"

for data in $data1 $data2 $data3 $data4 $data5 $data6 $data7 $data8 $data9 ;do
	cd $CountPath
	echo $data
	./wzip -d $data
done