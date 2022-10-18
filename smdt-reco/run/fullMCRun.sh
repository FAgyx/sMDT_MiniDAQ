if [ "$#" -ne 1 ]; then 
    echo "Precisely one argument required: path to .conf file"
else
    ./build/runBeam         --conf $1
    ./build/autoCalibration --conf $1
    ./build/residuals       --conf $1
    ./build/residuals -h    --conf $1
fi
