#!/bin/bash

# dummy0
sudo ip link add dummy0 type dummy
sudo ip link set dummy0 up
# dummy1
sudo ip link add dummy1 type dummy
sudo ip link set dummy1 up
 
ip a