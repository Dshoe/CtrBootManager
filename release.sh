#!/bin/sh
./make.sh
mkdir -p release/
cp build/CtrBootManager9.bin release/
cp data/a9lh.cfg release/a9lh.cfg
cd release/
zip CtrBootManager9.zip a9lh.cfg CtrBootManager9.bin
rm a9lh.cfg CtrBootManager9.bin
