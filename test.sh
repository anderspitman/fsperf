#!/bin/bash

inFile=$(mktemp)
outFile=$(mktemp)

echo "Hi there" > $inFile
./fsperf fread 4096 $inFile $outFile
diff $inFile $outFile

echo "Hi there" > $inFile
./fsperf mmap 4096 $inFile $outFile
diff $inFile $outFile

echo "lsidjflsjf9848s4jf84203fj4i2jf02j4389f0423fj0284f0284jf" > $inFile
./fsperf fread 4096 $inFile $outFile
diff $inFile $outFile

echo "lsidjflsjf9848s4jf84203fj4i2jf02j4389f0423fj0284f0284jf" > $inFile
./fsperf mmap 4096 $inFile $outFile
diff $inFile $outFile
