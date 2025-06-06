#!/bin/bash
set -e

# Install build tools and dependencies
sudo dnf install -y git gcc-c++ make cmake openssl-devel

# Build and install Paho C if not present
if ! ldconfig -p | grep -q libpaho-mqtt3as; then
    git clone https://github.com/eclipse/paho.mqtt.c.git
    pushd paho.mqtt.c
    cmake -Bbuild -H. -DPAHO_WITH_SSL=ON
    cmake --build build/ --target install
    sudo ldconfig
    popd
    rm -rf paho.mqtt.c
fi

# Build and install Paho C++ if not present
if ! ldconfig -p | grep -q libpaho-mqttpp3; then
    git clone https://github.com/eclipse/paho.mqtt.cpp.git
    pushd paho.mqtt.cpp
    cmake -Bbuild -H. -DPAHO_BUILD_SHARED=ON -DPAHO_WITH_SSL=ON
    cmake --build build/ --target install
    sudo ldconfig
    popd
    rm -rf paho.mqtt.cpp
fi

make clean
make
