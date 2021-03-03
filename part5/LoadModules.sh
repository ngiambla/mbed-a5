current=$(pwd)

cd KEY_SW_Driver;
make;
lsmod | grep KEY_SW
retVal=$?
if [ $retVal -ne 0 ]; then
    insmod KEY_SW.ko
fi

cd $current;
