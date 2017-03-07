
#DATAPATH=/home/xiaobai/coding/laborator/fm_adapt/fm_test/FM/data/
DATAPATH=../data/
#DATAPATH=/home/xiaobai/coding/laborator/data/data_new/10MB/
data1=${DATAPATH}"dblp.xml.100MB"
data2=${DATAPATH}"dna.100MB"
data3=${DATAPATH}"kernel.100MB"
data4=${DATAPATH}"para.100MB"
data5=${DATAPATH}"proteins.100MB"
data6=${DATAPATH}"sources.100MB"
data7=${DATAPATH}"cere.100MB"
data8=${DATAPATH}"world_leaders.40MB"
data9=${DATAPATH}"influenza.100MB"
data10=${DATAPATH}"english.100MB"
#data1=${DATAPATH}"dblp.xml"
#data2=${DATAPATH}"dna"
#data3=${DATAPATH}"kernel"
#data4=${DATAPATH}"para"
#data5=${DATAPATH}"pitches"
#data6=${DATAPATH}"sources"
#data7=${DATAPATH}"world_leaders"
#data8=${DATAPATH}"influenza"
#data9=${DATAPATH}"english.500MB"

for data in $data1 $data2 $data3 $data4 $data5 $data6 $data7 $data8 $data9 $data10;do
	./my_fm $data 1
done
