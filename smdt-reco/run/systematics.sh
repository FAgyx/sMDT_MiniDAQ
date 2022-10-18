#./build/decodeRawData --conf $1
./build/doT0Fit --conf $1
./build/autoCalibration --conf $1
./build/residuals --conf $1
./build/residuals -h --conf $1
./build/resolution --conf $1

./build/residuals --syst t0 --conf $1
./build/residuals --syst t0 -h --conf $1
./build/residuals --syst t0 -d --conf $1
./build/residuals --syst t0 -d -h --conf $1
./build/resolution --syst t0 --conf $1
./build/resolution --syst t0 -d --conf $1

./build/residuals --syst intercept --conf $1
./build/residuals --syst intercept -h --conf $1
./build/residuals --syst intercept -d --conf $1
./build/residuals --syst intercept -d -h --conf $1
./build/resolution --syst intercept --conf $1
./build/resolution --syst intercept -d --conf $1

./build/residuals --syst slope --conf $1
./build/residuals --syst slope -h --conf $1
./build/residuals --syst slope -d --conf $1
./build/residuals --syst slope -d -h --conf $1
./build/resolution --syst slope --conf $1
./build/resolution --syst slope -d --conf $1

./build/residuals --syst slew --conf $1
./build/residuals --syst slew -h --conf $1
./build/residuals --syst slew -d --conf $1
./build/residuals --syst slew -h -d --conf $1
./build/resolution --syst slew --conf $1
./build/resolution --syst slew -d --conf $1

#./build/autoCalibration --conf $1 --minEvent 100000
#./build/autoCalibration --conf $1 --minEvent 200000
#./build/autoCalibration --conf $1 --minEvent 300000
#./build/autoCalibration --conf $1 --minEvent 400000
#./build/autoCalibration --conf $1 --minEvent 500000

#./build/residuals --syst Partition1 --conf $1
#./build/residuals --syst Partition1 -h --conf $1
#./build/residuals --syst Partition2 --conf $1
#./build/residuals --syst Partition2 -h --conf $1
#./build/residuals --syst Partition3 --conf $1
#./build/residuals --syst Partition3 -h --conf $1
#./build/residuals --syst Partition4 --conf $1
#./build/residuals --syst Partition4 -h --conf $1
#./build/residuals --syst Partition5 --conf $1
#./build/residuals --syst Partition5 -h --conf $1
#./build/resolution --syst Partition1 --conf $1
#./build/resolution --syst Partition2 --conf $1
#./build/resolution --syst Partition3 --conf $1
#./build/resolution --syst Partition4 --conf $1
#./build/resolution --syst Partition5 --conf $1

./build/resolution --conf $1
