#!/bin/sh
m_B=4096
m_step=2

#CountPath=/home/kkzone/kkzone/git/MKD-GBWT/count_char/
CountPath=/home/xiaobai/coding/laborator/fm_adapt/count_char/
DATAPATH=/home/xiaobai/coding/laborator/data/

data1="dblp.xml"
data2="dna"
data3="kernel"
data4="para"
data5="pitches"
data6="sources"
data7="world_leaders"
data8="influenza"
data9="english.500MB"


for data in $data1 $data2 $data3 $data4 $data5 $data6 $data7 $data8 $data9 ;
do
	#cd $DATAPATH
	ls
	head -c 200000 ${DATAPATH}$data >>./$data
done