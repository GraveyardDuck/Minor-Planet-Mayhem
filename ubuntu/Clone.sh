#!/bin/bash
git clone https://github.com/ShacharAvni/Minor-Planet-Mayhem
git submodule init
git submodule update
git submodule foreach git pull origin master