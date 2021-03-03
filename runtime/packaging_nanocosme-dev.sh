#
#   packaging_nanocosme-dev.sh
# 
#   v2.0 CCC 5/2018
#

echo "Packaging nanocosme-dev..."

rm DEADJOE *~ *.o _* *.bak *.gz ./config/*~ > /dev/null 2>&1
cd ..
tar -cvvzf nanocosme.tar.gz * > /dev/null 2>&1
mv nanocosme.tar.gz nanocosme-dev_`date +%Y_%m_%d_%H_%M_%S`.tar.gz
