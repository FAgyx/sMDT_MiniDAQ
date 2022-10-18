./build/decodeRawData --conf $1
./build/doT0Fit --conf $1
./build/autoCalibration --conf $1
./build/residuals -e --conf $1
./build/efficiency --conf $1
