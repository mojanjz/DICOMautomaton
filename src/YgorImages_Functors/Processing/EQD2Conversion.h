//EQD2Conversion.h.
#pragma once


#include <list>
#include <functional>
#include <limits>
#include <map>
#include <cmath>

#include <experimental/any>

#include "YgorMisc.h"
#include "YgorMath.h"
#include "YgorImages.h"


struct EQD2ConversionUserData {

    //Dose prescription and fractionation details. Remember that d=2 only for the prescription dose!
    double NumberOfFractions = -1.0;
    double PrescriptionDose = -1.0;

    double AlphaBetaRatioNormal = 3.0; // alpha/beta for non-tumourous tissues.
    double AlphaBetaRatioTumour = 10.0; // alpha/beta for tumourous tissues.

};


bool EQD2Conversion(planar_image_collection<float,double>::images_list_it_t first_img_it,
                    std::list<planar_image_collection<float,double>::images_list_it_t> ,
                    std::list<std::reference_wrapper<planar_image_collection<float,double>>>,
                    std::list<std::reference_wrapper<contour_collection<double>>>, 
                    std::experimental::any );

