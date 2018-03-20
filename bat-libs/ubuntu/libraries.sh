#!/bin/bash
apt-get update >> /dev/null
apt-get install openssh-server -y 
apt-get install g++ -y
apt-get install cmake -y
apt-get install git -y
#installing the mongoc dependencies and driver
apt-get install pkg-config libssl-dev libsasl2-dev -y
cd ~
wget https://github.com/mongodb/mongo-c-driver/releases/download/1.6.2/mongo-c-driver-1.6.2.tar.gz
tar xzf mongo-c-driver-1.6.2.tar.gz
cd mongo-c-driver-1.6.2
./configure --disable-automatic-init-and-cleanup
make
make install
cd ~ 
rm mongo-c-driver-1.6.2.tar.gz
rm -rf mongo-c-driver-1.6.2


#installing mongocxx driver - connects c++ to mongo
wget https://github.com/mongodb/mongo-cxx-driver/archive/r3.1.1.tar.gz
tar -xzf r3.1.1.tar.gz
cd mongo-cxx-driver-r3.1.1/build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local ..
make EP_mnmlstc_core
make
make install
cd ~
rm r3.1.1.tar.gz
rm -rf mongo-cxx-driver-r3.1.1

# install curl library - requirement for curlpp
wget https://curl.haxx.se/download/curl-7.53.1.tar.gz
tar -xzf curl-7.53.1.tar.gz
cd curl-7.53.1
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
make
make install
cd ~
rm curl-7.53.1.tar.gz
rm -rf curl-7.53.1

# install curlpp library - fetches data from url (api)
wget https://github.com/jpbarrette/curlpp/archive/v0.8.1.tar.gz
tar -xzf v0.8.1.tar.gz
cd curlpp-0.8.1
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
make
make install
cd ~
rm v0.8.1.tar.gz
rm -rf curlpp-0.8.1

# install libxml2 - helps to convertv from xml 
apt-get install libpython-dev -y
wget ftp://xmlsoft.org/libxml2/LATEST_LIBXML2
gunzip -c LATEST_LIBXML2 | tar xvf -
cd libxml2-*
./configure --prefix=/usr/local
make
make install
cd ~
rm LATEST_LIBXML2
rm -rf libxml2*


# install log4cxx
apt-get install autoconf automake -y
apt-get install liblog4cxx10v5 liblog4cxx10-dev -y


# install jsoncpp
cd ~
git clone https://github.com/open-source-parsers/jsoncpp.git
cd jsoncpp
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_INSTALL_INCLUDEDIR=include/jsoncpp
make
make install
cd ~
rm -tf jsoncpp

#install talib
cd ~
wget http://prdownloads.sourceforge.net/ta-lib/ta-lib-0.4.0-src.tar.gz
tar -xvzf ta-lib-0.4.0-src.tar.gz
cd ta-lib
./configure
make
make install
cd ~
rm ta-lib-0.4.0-src.tar.gz
rm -rf 

apt-get install valgrind -y

apt-get install liblog4cxx10-dev -y

# update the list of installed shared libs. 
ldconfig