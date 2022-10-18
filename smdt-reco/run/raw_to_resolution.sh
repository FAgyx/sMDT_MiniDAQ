#./build/decodeRawData --conf $1
#./build/doT0Fit --conf $1
./build/autoCalibration --conf $1
./build/residuals --conf $1
./build/residuals -h --conf $1
./build/resolution --conf $1
