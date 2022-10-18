if [ "$#" -ne 1 ]; then 
    echo "Precisely one argument required: path to .conf file"
else
    ./build/decodeRawData   --conf $1
    ./build/doT0Fit         --conf $1
    ./build/autoCalibration --conf $1

    ./build/residuals       --conf $1
    ./build/residuals -h    --conf $1
    ./build/residuals       --conf $1 --syst t0
    ./build/residuals -h    --conf $1 --syst t0
    ./build/residuals       --conf $1 --syst t0 -d
    ./build/residuals -h    --conf $1 --syst t0 -d
    ./build/residuals       --conf $1 --syst slope
    ./build/residuals -h    --conf $1 --syst slope
    ./build/residuals       --conf $1 --syst slope -d
    ./build/residuals -h    --conf $1 --syst slope -d
    ./build/residuals       --conf $1 --syst intercept
    ./build/residuals -h    --conf $1 --syst intercept
    ./build/residuals       --conf $1 --syst intercept -d
    ./build/residuals -h    --conf $1 --syst intercept -d
    ./build/residuals       --conf $1 --syst Partition1
    ./build/residuals -h    --conf $1 --syst Partition1
    ./build/residuals       --conf $1 --syst Partition2
    ./build/residuals -h    --conf $1 --syst Partition2
    ./build/residuals       --conf $1 --syst Partition3
    ./build/residuals -h    --conf $1 --syst Partition3
    ./build/residuals       --conf $1 --syst Partition4
    ./build/residuals -h    --conf $1 --syst Partition4
    ./build/residuals       --conf $1 --syst Partition5
    ./build/residuals -h    --conf $1 --syst Partition5
    ./build/residuals       --conf $1 --syst slew
    ./build/residuals -h    --conf $1 --syst slew
    ./build/residuals       --conf $1 --syst slew -d
    ./build/residuals -h    --conf $1 --syst slew -d

    ./build/resolution      --conf $1 --syst t0
    ./build/resolution      --conf $1 --syst t0 -d
    ./build/resolution      --conf $1 --syst slope
    ./build/resolution      --conf $1 --syst slope -d
    ./build/resolution      --conf $1 --syst intercept
    ./build/resolution      --conf $1 --syst intercept -d
    ./build/resolution      --conf $1 --syst Partition1
    ./build/resolution      --conf $1 --syst Partition2
    ./build/resolution      --conf $1 --syst Partition3
    ./build/resolution      --conf $1 --syst Partition4
    ./build/resolution      --conf $1 --syst Partition5
    ./build/resolution      --conf $1 --syst slew
    ./build/resolution      --conf $1
fi
