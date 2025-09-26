#!/bin/bash

sudo apt update
sudo apt-get install -y gcc
sudo apt-get install -y make
sudo apt-get install -y libc-dev
sudo apt-get install -y linux-headers-$(uname -r)