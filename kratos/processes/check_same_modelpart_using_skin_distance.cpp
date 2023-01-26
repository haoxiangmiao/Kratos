//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Vicente Mataix Ferrandiz
//
//

// System includes
#include <limits>

// External includes

// Project includes
#include "processes/check_same_modelpart_using_skin_distance.h"
// #include "processes/calculate_distance_to_skin_process.h" // Continuous version
#include "processes/calculate_discontinuous_distance_to_skin_process.h" // Discontinuous version

namespace Kratos
{
template<std::size_t TDim>
void CheckSameModelPartUsingSkinDistance<TDim>::Execute()
{
    KRATOS_TRY

    // We get the names of the model parts
    const std::string& r_model_part_1_name = mThisParameters["model_part_1_name"].GetString();
    const std::string& r_model_part_2_name = mThisParameters["model_part_2_name"].GetString();
    const std::string& r_skin_model_part_1_name = mThisParameters["skin_model_part_1_name"].GetString();
    const std::string& r_skin_model_part_2_name = mThisParameters["skin_model_part_2_name"].GetString();

    // We get the model parts
    ModelPart& r_model_part_1 = mrModel.GetModelPart(r_model_part_1_name);
    ModelPart& r_skin_model_part_1 = mrModel.GetModelPart(r_skin_model_part_1_name);
    ModelPart& r_model_part_2 = mrModel.GetModelPart(r_model_part_2_name);
    ModelPart& r_skin_model_part_2 = mrModel.GetModelPart(r_skin_model_part_2_name);

    // We check that the model parts have the same number of elements (minor check to avoid problems)
    // TODO: Add MPI version
    KRATOS_ERROR_IF(r_model_part_1.NumberOfElements() != r_model_part_2.NumberOfElements()) << "The model part 1 has " << r_model_part_1.NumberOfElements() << " elements and the model part 2 has " << r_model_part_2.NumberOfElements() << " elements" << std::endl;

    // Compute the distance to the skin
    Parameters distance_parameters = mThisParameters["discontinuous_distance_settings"];
    CalculateDiscontinuousDistanceToSkinProcess<TDim> distance_calculator_1(r_model_part_1, r_skin_model_part_1, distance_parameters);
    distance_calculator_1.Execute();
    CalculateDiscontinuousDistanceToSkinProcess<TDim> distance_calculator_2(r_model_part_2, r_skin_model_part_2, distance_parameters);
    distance_calculator_2.Execute();

    // Elements iterators
    auto it_elem_begin_1 = r_model_part_1.ElementsBegin();
    auto it_elem_begin_2 = r_model_part_2.ElementsBegin();

    // Compute the average length of the elements in order to compute the tolerance
    double average_length = IndexPartition<std::size_t>(r_model_part_1.NumberOfElements()).for_each<SumReduction<double>>([&](std::size_t i) {
        auto it_elem_1 = it_elem_begin_1 + i;
        return it_elem_1->GetGeometry().Length();
    });
    average_length /= static_cast<double>(r_model_part_1.NumberOfElements());

    // Now we check that the difference is near a tolerance
    const double tolerance = average_length * mThisParameters["tolerance"].GetDouble();

    // Interate over the elements
    const Variable<Vector>& r_elem_dist_var = KratosComponents<Variable<Vector>>::Get(distance_parameters["elemental_distances_variable"].GetString());
    const double error = IndexPartition<std::size_t>(r_model_part_1.NumberOfElements()).for_each<SumReduction<double>>([&](std::size_t i) {
        auto it_elem_1 = it_elem_begin_1 + i;
        auto it_elem_2 = it_elem_begin_2 + i;
        const Vector& r_elem_dist_1 = it_elem_1->GetValue(r_elem_dist_var);
        const Vector& r_elem_dist_2 = it_elem_2->GetValue(r_elem_dist_var);
        return norm_2(r_elem_dist_1 - r_elem_dist_2);
    });
    if (error > tolerance) {
        KRATOS_ERROR << "The distance between the two model parts is " << error << " and the tolerance is " << tolerance << std::endl;
    } else {
        KRATOS_INFO("CheckSameModelPartUsingSkinDistance") << "The distance between the two model parts is " << error << " and the tolerance is " << tolerance << ". It would be assumed it is the same model part." << std::endl;
    }

    // TODO: Add MPI version

    KRATOS_CATCH("")
}


/***********************************************************************************/
/***********************************************************************************/

template<std::size_t TDim>
const Parameters CheckSameModelPartUsingSkinDistance<TDim>::GetDefaultParameters() const
{
    KRATOS_TRY

    const Parameters default_parameters = Parameters(R"(
    {
        "model_part_1_name"     : "PLEASE_SPECIFY_MODEL_PART_1_NAME",
        "skin_model_part_1_name": "PLEASE_SPECIFY_SKIN_MODEL_PART_NAME",
        "model_part_2_name"     : "PLEASE_SPECIFY_MODEL_PART_2_NAME",
        "skin_model_part_2_name": "PLEASE_SPECIFY_SKIN_MODEL_PART_NAME",
        "tolerance"             : 1.0e-3,
        //"continuous_distance"   : false, // TODO: Add continuous version if needed in the future
        "discontinuous_distance_settings": {
            "elemental_distances_variable"                          : "ELEMENTAL_DISTANCES",
            "elemental_edge_distances_variable"                     : "ELEMENTAL_EDGE_DISTANCES",
            "elemental_edge_distances_extrapolated_variable"        : "ELEMENTAL_EDGE_DISTANCES_EXTRAPOLATED",
            "embedded_velocity_variable"                            : "EMBEDDED_VELOCITY",
            "calculate_elemental_edge_distances"                    : false,
            "calculate_elemental_edge_distances_extrapolated"       : false,
            "use_positive_epsilon_for_zero_values"                  : true
        }
    })" );

    return default_parameters;

    KRATOS_CATCH("")
}

/***********************************************************************************/
/***********************************************************************************/

template class Kratos::CheckSameModelPartUsingSkinDistance<2>;
template class Kratos::CheckSameModelPartUsingSkinDistance<3>;

} // namespace Kratos
