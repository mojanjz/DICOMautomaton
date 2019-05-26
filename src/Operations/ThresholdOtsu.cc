//ThresholdOtsu.cc - A part of DICOMautomaton 2019. Written by hal clark.

#include <asio.hpp>
#include <algorithm>
#include <experimental/optional>
#include <fstream>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <regex>
#include <set> 
#include <stdexcept>
#include <string>    
#include <utility>            //Needed for std::pair.
#include <vector>

#include "Explicator.h"       //Needed for Explicator class.
#include "YgorImages.h"
#include "YgorMath.h"         //Needed for vec3 class.
#include "YgorMisc.h"         //Needed for FUNCINFO, FUNCWARN, FUNCERR macros.
#include "YgorStats.h"        //Needed for Stats:: namespace.
#include "YgorString.h"       //Needed for GetFirstRegex(...)

#include "../Structs.h"
#include "../Regex_Selectors.h"
#include "../Thread_Pool.h"
#include "../YgorImages_Functors/ConvenienceRoutines.h"

#include "../Surface_Meshes.h"

#include "ThresholdOtsu.h"



OperationDoc OpArgDocThresholdOtsu(void){
    OperationDoc out;
    out.name = "ThresholdOtsu";

    out.desc = 
        "This routine performs Otsu thresholding (i.e., 'binarization') on an image volume."
        " The thresholding is limited within ROI(s)."
        " Otsu thresholding works best on images with a well-defined bimodal voxel intensity histogram."
        " It works by finding the threshold that partitions the voxel intensity historgram"
        " into two parts, essentially so that the sum of each partition's variance is minimal."
        " The number of histogram bins (i.e., number of distinct voxel magnitude levels) is configurable."
        " Voxels are binarized; the replacement values are also configurable.";
        
    out.notes.emplace_back(
        "The Otsu method will not necessarily cleanly separate bimodal peaks in the voxel intensity histogram."
    );
    
    out.args.emplace_back();
    out.args.back() = IAWhitelistOpArgDoc();
    out.args.back().name = "ImageSelection";
    out.args.back().default_val = "last";

    out.args.emplace_back();
    out.args.back().name = "HistogramBins";
    out.args.back().desc = "The number of equal-width bins the histogram should have."
                           " Classically, images were 8-bit integer-valued and thus 255 bins were commonly used."
                           " However, because floating-point numbers are used practically any number of bins are"
                           " supported. What is optimal (or acceptable) depends on the analytical requirements."
                           " If the threshold does not have to be exact, try use the smallest number of bins you"
                           " can get away with; 50-150 should suffice. This will speed up computation."
                           " If the threshold is being used for analytical purposes, use as many bins as the"
                           " data can support -- if the voxel values span only 8-bit integers, having more"
                           " than 255 bins will not improve the analysis. Likewise if voxels are discretized"
                           " or sparse. Experiment by gradually increasing the number of bins until the"
                           " threshold value converges to a reasonable number, and then use that number of"
                           " bins for future analysis.";
    out.args.back().default_val = "255";
    out.args.back().expected = true;
    out.args.back().examples = { "10", "50", "100", "200", "500" };

    out.args.emplace_back();
    out.args.back().name = "ReplacementLow";
    out.args.back().desc = "The value to give voxels which are below (exclusive) the Otsu threshold value.";
    out.args.back().default_val = "0.0";
    out.args.back().expected = true;
    out.args.back().examples = { "-1.0", "0.0", "1.23", "nan", "inf" };

    out.args.emplace_back();
    out.args.back().name = "ReplacementHigh";
    out.args.back().desc = "The value to give voxels which are above (inclusive) the Otsu threshold value.";
    out.args.back().default_val = "1.0";
    out.args.back().expected = true;
    out.args.back().examples = { "-1.0", "0.0", "1.23", "nan", "inf" };

    out.args.emplace_back();
    out.args.back().name = "OverwriteVoxels";
    out.args.back().desc = "Controls whether voxels should actually be binarized or not."
                           " Whether or not voxel intensities are overwritten, the Otsu threshold value is"
                           " written into the image metadata as 'OtsuThreshold' in case further processing"
                           " is needed.";
    out.args.back().default_val = "true";
    out.args.back().expected = true;
    out.args.back().examples = { "true", "false" };

    out.args.emplace_back();
    out.args.back().name = "Channel";
    out.args.back().desc = "The image channel to use. Zero-based.";
    out.args.back().default_val = "0";
    out.args.back().expected = true;
    out.args.back().examples = { "0", "1", "2" };

    out.args.emplace_back();
    out.args.back().name = "NormalizedROILabelRegex";
    out.args.back().desc = "A regex matching ROI labels/names to consider. The default will match"
                           " all available ROIs. Be aware that input spaces are trimmed to a single space."
                           " If your ROI name has more than two sequential spaces, use regex to avoid them."
                           " All ROIs have to match the single regex, so use the 'or' token if needed."
                           " Regex is case insensitive and uses extended POSIX syntax.";
    out.args.back().default_val = ".*";
    out.args.back().expected = true;
    out.args.back().examples = { ".*", ".*Body.*", "Body", "Gross_Liver",
                            R"***(.*Left.*Parotid.*|.*Right.*Parotid.*|.*Eye.*)***",
                            R"***(Left Parotid|Right Parotid)***" };

    out.args.emplace_back();
    out.args.back().name = "ROILabelRegex";
    out.args.back().desc = "A regex matching ROI labels/names to consider. The default will match"
                           " all available ROIs. Be aware that input spaces are trimmed to a single space."
                           " If your ROI name has more than two sequential spaces, use regex to avoid them."
                           " All ROIs have to match the single regex, so use the 'or' token if needed."
                           " Regex is case insensitive and uses extended POSIX syntax.";
    out.args.back().default_val = ".*";
    out.args.back().expected = true;
    out.args.back().examples = { ".*", ".*body.*", "body", "Gross_Liver",
                                 R"***(.*left.*parotid.*|.*right.*parotid.*|.*eyes.*)***",
                                 R"***(left_parotid|right_parotid)***" };

    out.args.emplace_back();
    out.args.back().name = "ContourOverlap";
    out.args.back().desc = "Controls overlapping contours are treated."
                           " The default 'ignore' treats overlapping contours as a single contour, regardless of"
                           " contour orientation. The option 'honour_opposite_orientations' makes overlapping contours"
                           " with opposite orientation cancel. Otherwise, orientation is ignored. The latter is useful"
                           " for Boolean structures where contour orientation is significant for interior contours (holes)."
                           " The option 'overlapping_contours_cancel' ignores orientation and cancels all contour overlap.";
    out.args.back().default_val = "ignore";
    out.args.back().expected = true;
    out.args.back().examples = { "ignore", "honour_opposite_orientations", 
                            "overlapping_contours_cancel", "honour_opps", "overlap_cancel" }; 

    out.args.emplace_back();
    out.args.back().name = "Inclusivity";
    out.args.back().desc = "Controls how voxels are deemed to be 'within' the interior of the selected ROI(s)."
                           " The default 'center' considers only the central-most point of each voxel."
                           " There are two corner options that correspond to a 2D projection of the voxel onto the image plane."
                           " The first, 'planar_corner_inclusive', considers a voxel interior if ANY corner is interior."
                           " The second, 'planar_corner_exclusive', considers a voxel interior if ALL (four) corners are interior.";
    out.args.back().default_val = "center";
    out.args.back().expected = true;
    out.args.back().examples = { "center", "centre", 
                                 "planar_corner_inclusive", "planar_inc",
                                 "planar_corner_exclusive", "planar_exc" };

    return out;
}



Drover ThresholdOtsu(Drover DICOM_data, OperationArgPkg OptArgs, std::map<std::string,std::string> /*InvocationMetadata*/, std::string FilenameLex){

    Explicator X(FilenameLex);

    //---------------------------------------------- User Parameters --------------------------------------------------
    const auto ImageSelectionStr = OptArgs.getValueStr("ImageSelection").value();

    const auto OverwriteVoxelsStr = OptArgs.getValueStr("OverwriteVoxels").value();

    const auto HistogramBins = std::stol( OptArgs.getValueStr("HistogramBins").value() );

    const auto ReplacementLow = std::stod( OptArgs.getValueStr("ReplacementLow").value() );
    const auto ReplacementHigh = std::stod( OptArgs.getValueStr("ReplacementHigh").value() );

    const auto Channel = std::stol( OptArgs.getValueStr("Channel").value() );

    const auto InclusivityStr = OptArgs.getValueStr("Inclusivity").value();
    const auto ContourOverlapStr = OptArgs.getValueStr("ContourOverlap").value();

    const auto NormalizedROILabelRegex = OptArgs.getValueStr("NormalizedROILabelRegex").value();
    const auto ROILabelRegex = OptArgs.getValueStr("ROILabelRegex").value();

    //-----------------------------------------------------------------------------------------------------------------

    const auto regex_true = Compile_Regex("^tr?u?e?$");

    const auto regex_centre = Compile_Regex("^cent.*");
    const auto regex_pci = Compile_Regex("^planar_?c?o?r?n?e?r?s?_?inc?l?u?s?i?v?e?$");
    const auto regex_pce = Compile_Regex("^planar_?c?o?r?n?e?r?s?_?exc?l?u?s?i?v?e?$");

    const auto regex_interior = Compile_Regex("^int?e?r?i?o?r?$");
    const auto regex_exterior = Compile_Regex("^ext?e?r?i?o?r?$");

    const auto regex_ignore = Compile_Regex("^ig?n?o?r?e?$");
    const auto regex_honopps = Compile_Regex("^ho?n?o?u?r?_?o?p?p?o?s?i?t?e?_?o?r?i?e?n?t?a?t?i?o?n?s?$");
    const auto regex_cancel = Compile_Regex("^ov?e?r?l?a?p?p?i?n?g?_?c?o?n?t?o?u?r?s?_?c?a?n?c?e?l?s?$");

    if(!isininc(2,HistogramBins,100'000)){
        throw std::invalid_argument("Number of histogram bins requested cannot be accomodated. Refusing to continue.");
        // There is no need for an upper limit, but if it is too large it might indicate a failure somewhere.
        // The upper limit can be grown as needed.
    }

    std::function<void(long int, long int, long int, std::reference_wrapper<planar_image<float,double>>, float &)> f_noop;

    //-----------------------------------------------------------------------------------------------------------------

    // Gather contours.
    auto cc_all = All_CCs( DICOM_data );
    auto cc_ROIs = Whitelist( cc_all, { { "ROIName", ROILabelRegex },
                                        { "NormalizedROIName", NormalizedROILabelRegex } } );
    if(cc_ROIs.empty()){
        throw std::invalid_argument("No contours selected. Cannot continue.");
    }

    auto IAs_all = All_IAs( DICOM_data );
    auto IAs = Whitelist( IAs_all, ImageSelectionStr );
    for(auto & iap_it : IAs){
        if((*iap_it)->imagecoll.images.empty()) continue;

        std::vector<double> voxel_vals;
        std::mutex voxel_vals_access;

        // First-pass: collect voxel magnitudes for analysis.
        {
            PartitionedImageVoxelVisitorMutatorUserData ud;

            ud.mutation_opts.editstyle = Mutate_Voxels_Opts::EditStyle::InPlace;
            ud.mutation_opts.aggregate = Mutate_Voxels_Opts::Aggregate::First;
            ud.mutation_opts.adjacency = Mutate_Voxels_Opts::Adjacency::SingleVoxel;
            ud.mutation_opts.maskmod   = Mutate_Voxels_Opts::MaskMod::Noop;
            ud.description = "";

            if(false){
            }else if( std::regex_match(ContourOverlapStr, regex_ignore) ){
                ud.mutation_opts.contouroverlap = Mutate_Voxels_Opts::ContourOverlap::Ignore;
            }else if( std::regex_match(ContourOverlapStr, regex_honopps) ){
                ud.mutation_opts.contouroverlap = Mutate_Voxels_Opts::ContourOverlap::HonourOppositeOrientations;
            }else if( std::regex_match(ContourOverlapStr, regex_cancel) ){
                ud.mutation_opts.contouroverlap = Mutate_Voxels_Opts::ContourOverlap::ImplicitOrientations;
            }else{
                throw std::invalid_argument("ContourOverlap argument '"_s + ContourOverlapStr + "' is not valid");
            }
            if(false){
            }else if( std::regex_match(InclusivityStr, regex_centre) ){
                ud.mutation_opts.inclusivity = Mutate_Voxels_Opts::Inclusivity::Centre;
            }else if( std::regex_match(InclusivityStr, regex_pci) ){
                ud.mutation_opts.inclusivity = Mutate_Voxels_Opts::Inclusivity::Inclusive;
            }else if( std::regex_match(InclusivityStr, regex_pce) ){
                ud.mutation_opts.inclusivity = Mutate_Voxels_Opts::Inclusivity::Exclusive;
            }else{
                throw std::invalid_argument("Inclusivity argument '"_s + InclusivityStr + "' is not valid");
            }

            ud.f_unbounded = f_noop;
            ud.f_visitor = f_noop;
            ud.f_bounded = [&]( long int /*row*/,
                                long int /*col*/,
                                long int chan,
                                std::reference_wrapper<planar_image<float,double>> /*img_refw*/,
                                float &voxel_val ) -> void {
                if( (Channel < 0) || (Channel == chan) ){
                    std::lock_guard<std::mutex> lock(voxel_vals_access);
                    voxel_vals.emplace_back(voxel_val);
                }
                return;
            };

            if(!(*iap_it)->imagecoll.Process_Images_Parallel( GroupIndividualImages,
                                                              PartitionedImageVoxelVisitorMutator,
                                                              {}, cc_ROIs, &ud )){
                throw std::runtime_error("Unable to generate a histogram from the specified ROI(s).");
            }
        }


        // Generate a histogram.
        if(voxel_vals.empty()){
            throw std::invalid_argument("No voxels were selected; unable to perform Otsu thresholding.");
        }
        const bool ExplicitBins = false;
        const samples_1D<double> hist = Bag_of_numbers_to_N_equal_bin_samples_1D_histogram(voxel_vals, HistogramBins, ExplicitBins);

        // Write the histogram to a file for inspection/debugging.
        if(false){
            if(!hist.Write_To_File("/tmp/histogram.dat")){
                throw std::runtime_error("Unable to write histogram to file. Cannot continue.");
            }
        }

        // ============== Otsu's method =============
        //const auto total_bin_magnitude = 1.0; // Sum of all bins.
        const auto total_bin_magnitude = [&](void) -> double {
            double out = 0.0;
            for(const auto &s : hist.samples) out += s[2];
            return out;
        }();

        const double total_moment = [&](void) -> double {
            double moment = 0.0;
            double i = 0.0;
            for(const auto &s : hist.samples){
                moment += (s[2] * i);
                i += 1.0;
            }
            return moment;
        }();

        double running_low_moment = 0.0;
        double running_magnitude_low = 0.0;

        double max_variance = -1.0;
        size_t threshold_index = 0;

        size_t i = 0;
        for(const auto &s : hist.samples){
            running_magnitude_low += s[2];

            // If no bins with any height have yet been seen, skip them.
            if(running_magnitude_low == 0.0){
                ++i;
                continue;
            }

            const auto running_magnitude_high = total_bin_magnitude - running_magnitude_low;

            // If we've reached the end, or numerical losses will cause issues, then bail.
            if(running_magnitude_high <= 0.0) break;

            running_low_moment += (s[2] * static_cast<double>(i));
            const auto mean_low = running_low_moment / running_magnitude_low;
            const auto mean_high = (total_moment - running_low_moment) / running_magnitude_high;

            // If numerical issues cause negative or non-finite means (which should always be >= 0), then bail.
            if( !std::isfinite(mean_low)
            ||  !std::isfinite(mean_high)
            ||  (mean_low < 0.0)
            ||  (mean_high < 0.0) ){
                break;
            }

            // Test if the current threshold's variance is maximal.
            const auto current_variance = running_magnitude_low 
                                        * running_magnitude_high 
                                        * std::pow(mean_high - mean_low, 2.0);
            if(max_variance < current_variance){
                max_variance = current_variance;
                threshold_index = i;
            }
            ++i;
        }

        if(max_variance < 0.0){
            throw std::logic_error("Unable to perform Otsu thresholding; no suitable thresholds were identified");
        }

        const auto f_threshold = hist.samples.at(threshold_index)[0];
        FUNCINFO("Threshold found to be in bin " << threshold_index << "/" << HistogramBins 
                 << ", which has a voxel value = " << f_threshold);

        // ==========================================================

        // Imbue the images with the threshold.
        for(auto &animg : (*iap_it)->imagecoll.images){
            animg.metadata["OtsuThreshold"] = std::to_string(f_threshold);
        }

        // Second-pass: binarize voxels according to the threshold, if desired.
        if( std::regex_match(OverwriteVoxelsStr, regex_true) ){
            PartitionedImageVoxelVisitorMutatorUserData ud;

            ud.mutation_opts.editstyle = Mutate_Voxels_Opts::EditStyle::InPlace;
            ud.mutation_opts.aggregate = Mutate_Voxels_Opts::Aggregate::First;
            ud.mutation_opts.adjacency = Mutate_Voxels_Opts::Adjacency::SingleVoxel;
            ud.mutation_opts.maskmod   = Mutate_Voxels_Opts::MaskMod::Noop;
            ud.description = "Otsu thresholded (binarized)";

            if(false){
            }else if( std::regex_match(ContourOverlapStr, regex_ignore) ){
                ud.mutation_opts.contouroverlap = Mutate_Voxels_Opts::ContourOverlap::Ignore;
            }else if( std::regex_match(ContourOverlapStr, regex_honopps) ){
                ud.mutation_opts.contouroverlap = Mutate_Voxels_Opts::ContourOverlap::HonourOppositeOrientations;
            }else if( std::regex_match(ContourOverlapStr, regex_cancel) ){
                ud.mutation_opts.contouroverlap = Mutate_Voxels_Opts::ContourOverlap::ImplicitOrientations;
            }else{
                throw std::invalid_argument("ContourOverlap argument '"_s + ContourOverlapStr + "' is not valid");
            }
            if(false){
            }else if( std::regex_match(InclusivityStr, regex_centre) ){
                ud.mutation_opts.inclusivity = Mutate_Voxels_Opts::Inclusivity::Centre;
            }else if( std::regex_match(InclusivityStr, regex_pci) ){
                ud.mutation_opts.inclusivity = Mutate_Voxels_Opts::Inclusivity::Inclusive;
            }else if( std::regex_match(InclusivityStr, regex_pce) ){
                ud.mutation_opts.inclusivity = Mutate_Voxels_Opts::Inclusivity::Exclusive;
            }else{
                throw std::invalid_argument("Inclusivity argument '"_s + InclusivityStr + "' is not valid");
            }

            ud.f_unbounded = f_noop;
            ud.f_visitor = f_noop;
            ud.f_bounded = [&]( long int /*row*/,
                                long int /*col*/,
                                long int chan,
                                std::reference_wrapper<planar_image<float,double>> /*img_refw*/,
                                float &voxel_val ) -> void {
                if( (Channel < 0) || (Channel == chan) ){
                    voxel_val = (voxel_val < f_threshold) ? ReplacementLow
                                                          : ReplacementHigh;
                }
                return;
            };

            if(!(*iap_it)->imagecoll.Process_Images_Parallel( GroupIndividualImages,
                                                              PartitionedImageVoxelVisitorMutator,
                                                              {}, cc_ROIs, &ud )){
                throw std::runtime_error("Unable to implement Otsu thresholding within the specified ROI(s).");
            }
        }

    }


    return DICOM_data;
}

