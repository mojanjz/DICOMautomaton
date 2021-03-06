
#pragma once

#include <exception>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <sys/time.h>

#include <cstdlib> //Needed for exit() calls.
#include <utility> //Needed for std::pair.

#include "YgorMath.h" //Needed for samples_1D.

void Launch_SCSI(samples_1D<double> &, std::vector<samples_1D<double>> &);

/* For Benchmarking Purposes */
typedef unsigned long long timestamp_t;

timestamp_t get_timestamp(void);
