#/bin/sh

clear

rm -rf ./mac-gnumake
mkdir mac-gnumake

cd mac-gnumake
#-DAMF_CORE_STATIC=1
cmake .. -DOpenCL_INCLUDE_DIR=-DOpenCL_INCLUDE_DIR="../../../../../../../thirdparty/OpenCL-Headers"
cd ..