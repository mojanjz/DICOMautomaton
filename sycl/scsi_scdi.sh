#!/bin/bash
AIF_FILE='../sanitized_aif_vif/AIF_000000_sanitizied.txt'
VIF_FILE='../sanitized_aif_vif/VIF_000001_sanitizied.txt'
for FILE in *; do
    run_model -a $AIF_FILE -v $VIF_FILE -c $FILE
    run_model -b -a $AIF_FILE -v $VIF_FILE -c $FILE