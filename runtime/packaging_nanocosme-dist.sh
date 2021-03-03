#
#   packaging_nanocosme-dist.sh
# 
#   v2.0 CCC 5/2018
#

echo "Packaging nanocosme-dist..."

rm DEADJOE a.out *~ _* *.bak *.gz ./config/*~ > /dev/null 2>&1
cd ..
rm DEADJOE a.out *~ _* *.bak *.gz ./config/*~ > /dev/null 2>&1
rm ../nanocosme_dist -r
cd ..
mkdir ./nanocosme_dist
cd ./nanocosme_dist
cp ../nanocosme5/* . -r
rm ./runtime/*.sh
#rm loop_handlers.o
#rm application.o

rm ./runtime/preprocessor.cpp
rm ./runtime/name_server.cpp
rm ./runtime/gateway_mqtt.cpp
rm ./runtime/runtime.cpp
rm ./runtime/makefile

rm application.h
rm application.cpp 
mv application_demo.cpp application.cpp
rm nanocosme_app 

tar -cvvzf nanocosme-dist.tar.gz * > /dev/null 2>&1
tar -cvvf nanocosme-dist.tar nanocosme-dist.tar.gz

grep "NANOCOSME_VERSION" ./runtime/runtime.h > runtime_version.txt
tar -rvvf nanocosme-dist.tar runtime_version.txt
tar -rvvf nanocosme-dist.tar README.TXT
rm nanocosme-dist.tar.gz 
