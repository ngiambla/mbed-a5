current=$(pwd)

cd ../KEY_SW_Driver;
make;
lsmod | grep KEY_SW
retVal=$?
if [ $retVal -ne 0 ]; then
    insmod KEY_SW.ko
fi

cd ../LEDR_HEX_Driver;
make;
lsmod | grep LEDR_HEX
retVal=$?
if [ $retVal -ne 0 ]; then
    insmod LEDR_HEX.ko
fi

cd ../P1P2;
make;
lsmod | grep stopwatch
retVal=$?
if [ $retVal -ne 0 ]; then
    insmod stopwatch.ko
fi

cd $current;