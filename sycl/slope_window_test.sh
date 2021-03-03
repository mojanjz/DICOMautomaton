#!/bin/bash

slope_window_set="15 20 50 100 200 300 400"
for slope_window in $slope_window_set; do
    echo "slope_window: $slope_window"
    run_model -a data/aif.txt -v data/vif.txt -c data/c.txt -w $slope_window >> slope_window_exp.txt
done
