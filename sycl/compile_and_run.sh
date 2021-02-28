#!/bin/bash

read -p "Running this script will replace all files in DICOMautomaton/sycl/data are you sure you want to continue? ([Y/y] or ^C)" -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]
then
    cd /DICOMautomaton/sycl/
    #bash ./compile_and_install.sh 
    cd data/input/
    rm -r *
    gen_test_inputs

    cd ../output/

    if (( $# == 0 )); then
        echo "Running model with smooth generated data..."
        run_model -a ../input/aif.txt -v ../input/vif.txt -c ../input/c.txt
    else
        while getopts "n" OPTION
        do
            case $OPTION in
                n)
                    echo "Running model with noisy generated data..."
                    run_model -a ../input/aif_noise.txt -v ../input/vif_noise.txt -c ../input/c_noise.txt
                    exit
                    ;;
                *)
                    echo "Flag not recognized. Please use -n to run model with noise."
                    exit
                    ;;
        esac
    done
    fi

    text=$(cat kParams.txt)
    IFS=' '
    read -a kParams <<< "$text"

    echo "k1a:${kParams[0]} k1v:${kParams[1]} k2:${kParams[2]}"
fi

