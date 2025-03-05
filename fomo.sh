#!/bin/bash
pathDatasetFomo='/home/user/data/fomo_data/extracted' #Example, it is necesary to change it by the dataset path
trajectoryName='red' # red, blue

####################################
#           STEREO FOMO            #
####################################

# echo "Launching fomo rectified with Stereo sensor"
# ./Main/stereo/stereo_fomo ./Vocabulary/ORBvoc.txt ./Main/calibration_files/fomo_zed_rectified.yaml "$pathDatasetFomo"/$trajectoryName/images/left_rect/ "$pathDatasetFomo"/$trajectoryName/images/right_rect/ ./Results/

####################################
#           MONO FOMO              #
####################################

# echo "Launching fomo with Mono sensor"
echo "Launching fomo rectified with Mono sensor"
./Main/mono/mono_fomo ./Vocabulary/ORBvoc.txt ./Main/calibration_files/fomo_zed_rectified.yaml "$pathDatasetFomo"/$trajectoryName/images/left_rect/ ./Results/

