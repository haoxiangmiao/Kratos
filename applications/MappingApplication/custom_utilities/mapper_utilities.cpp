//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Philipp Bucher, Jordi Cotela
//
// See Master-Thesis P.Bucher
// "Development and Implementation of a Parallel
//  Framework for Non-Matching Grid Mapping"

// System includes

// External includes

// Project includes
#include "mapper_utilities.h"
#include "mapping_application_variables.h"

namespace Kratos
{
namespace MapperUtilities
{

using IndexType = std::size_t;
using SizeType = std::size_t;

void AssignInterfaceEquationIds(Communicator& rModelPartCommunicator)
{
    const int num_nodes_local = rModelPartCommunicator.LocalMesh().NumberOfNodes();

    int num_nodes_accumulated;

    rModelPartCommunicator.ScanSum(num_nodes_local, num_nodes_accumulated);

    const int start_equation_id = num_nodes_accumulated - num_nodes_local;

    const auto nodes_begin = rModelPartCommunicator.LocalMesh().NodesBegin();

    #pragma omp parallel for
    for (int i=0; i<num_nodes_local; ++i)
    {
        // TODO this should be working in omp, not usre though
        ( nodes_begin + i )->SetValue(INTERFACE_EQUATION_ID, start_equation_id + i);
    }

    rModelPartCommunicator.SynchronizeVariable(INTERFACE_EQUATION_ID);
}

double ComputeSearchRadius(ModelPart& rModelPart, int EchoLevel)
{
    double search_safety_factor = 1.2;
    double max_element_size = 0.0;

    int num_conditions_global = ComputeNumberOfConditions(rModelPart);
    int num_elements_global = ComputeNumberOfElements(rModelPart);

    if (num_conditions_global > 0)
        max_element_size = ComputeMaxEdgeLengthLocal(rModelPart.GetCommunicator().LocalMesh().Conditions());
    else if (num_elements_global > 0)
        max_element_size = ComputeMaxEdgeLengthLocal(rModelPart.GetCommunicator().LocalMesh().Elements());
    else
    {
        if (EchoLevel >= 2 && rModelPart.GetCommunicator().MyPID() == 0)
            std::cout << "MAPPER WARNING, no conditions/elements for search radius "
                        << "computations in ModelPart \"" << rModelPart.Name() << "\" found, "
                        << "using nodes (less efficient, bcs search radius will be larger)" << std::endl;
        max_element_size = ComputeMaxEdgeLengthLocal(rModelPart.GetCommunicator().LocalMesh().Nodes());
    }

    rModelPart.GetCommunicator().MaxAll(max_element_size); // Compute the maximum among the partitions
    return max_element_size * search_safety_factor;
}

void CheckInterfaceModelParts(const int CommRank)
{
    // const int num_nodes_origin = MapperUtilities::ComputeNumberOfNodes(mrModelPartOrigin);
    // const int num_conditions_origin = MapperUtilities::ComputeNumberOfConditions(mrModelPartOrigin);
    // const int num_elements_origin = MapperUtilities::ComputeNumberOfElements(mrModelPartOrigin);

    // const int num_nodes_destination = MapperUtilities::ComputeNumberOfNodes(mrModelPartDestination);
    // const int num_conditions_destination = MapperUtilities::ComputeNumberOfConditions(mrModelPartDestination);
    // const int num_elements_destination = MapperUtilities::ComputeNumberOfElements(mrModelPartDestination);

    // // Check if the ModelPart contains entities
    // KRATOS_ERROR_IF(num_nodes_origin + num_conditions_origin + num_elements_origin < 1)
    //     << "Neither Nodes nor Conditions nor Elements found "
    //     << "in the Origin ModelPart" << std::endl;

    // KRATOS_ERROR_IF(num_nodes_destination + num_conditions_destination + num_elements_destination < 1)
    //     << "Neither Nodes nor Conditions nor Elements found "
    //     << "in the Destination ModelPart" << std::endl;

    // // Check if the inpt ModelParts contain both Elements and Conditions
    // // This is NOT possible, bcs the InterfaceObjects are constructed
    // // with whatever exists in the Modelpart (see the InterfaceObjectManagerBase,
    // // function "InitializeInterfaceGeometryObjectManager")
    // KRATOS_ERROR_IF(num_conditions_origin > 0 && num_elements_origin > 0)
    //     << "Origin ModelPart contains both Conditions and Elements "
    //     << "which is not permitted" << std::endl;

    // KRATOS_ERROR_IF(num_conditions_destination > 0 && num_elements_destination > 0)
    //     << "Destination ModelPart contains both Conditions and Elements "
    //     << "which is not permitted" << std::endl;

    // if (mEchoLevel >= 2) {
    //     std::vector<double> model_part_origin_bbox = MapperUtilities::ComputeModelPartBoundingBox(mrModelPartOrigin);
    //     std::vector<double> model_part_destination_bbox = MapperUtilities::ComputeModelPartBoundingBox(mrModelPartDestination);

    //     bool bbox_overlapping = MapperUtilities::ComputeBoundingBoxIntersection(
    //                                                 model_part_origin_bbox,
    //                                                 model_part_destination_bbox);
    //     if(CommRank == 0)
    //     {
    //         if (!bbox_overlapping) {
    //             std::cout << "MAPPER WARNING, the bounding boxes of the "
    //                         << "Modelparts do not overlap! "
    //                         << MapperUtilities::PrintModelPartBoundingBoxes(model_part_origin_bbox,
    //                                                                         model_part_destination_bbox)
    //                         << std::endl;
    //         } else if (mEchoLevel >= 3)
    //         {
    //             std::cout << MapperUtilities::PrintModelPartBoundingBoxes(model_part_origin_bbox,
    //                                                                         model_part_destination_bbox)
    //                         << std::endl;
    //         }
    //     }
    // }
}

std::vector<double> ComputeLocalBoundingBox(ModelPart& rModelPart)
{
    std::vector<double> local_bounding_box {-1e10, 1e10, -1e10, 1e10, -1e10, 1e10}; // initialize "inverted"
    // xmax, xmin,  ymax, ymin,  zmax, zmin

    // loop over all nodes (local and ghost(necessary if conditions have only ghost nodes) )
    for (auto &r_node : rModelPart.Nodes())
    {
        local_bounding_box[0] = std::max(r_node.X(), local_bounding_box[0]);
        local_bounding_box[1] = std::min(r_node.X(), local_bounding_box[1]);
        local_bounding_box[2] = std::max(r_node.Y(), local_bounding_box[2]);
        local_bounding_box[3] = std::min(r_node.Y(), local_bounding_box[3]);
        local_bounding_box[4] = std::max(r_node.Z(), local_bounding_box[4]);
        local_bounding_box[5] = std::min(r_node.Z(), local_bounding_box[5]);
    }
    return local_bounding_box;
}

void ComputeBoundingBoxesWithTolerance(const std::vector<double>& rBoundingBoxes,
                                       const double Tolerance,
                                       std::vector<double>& rBoundingBoxesWithTolerance)
{
    const SizeType size_vec = rBoundingBoxes.size();

    KRATOS_ERROR_IF_NOT(std::fmod(size_vec, 6) == 0)
        << "Bounding Boxes size has to be a multiple of 6!" << std::endl;

    if (rBoundingBoxesWithTolerance.size() != size_vec)
        rBoundingBoxesWithTolerance.resize(size_vec);

    // Apply Tolerances
    for (IndexType i=0; i<size_vec; i+=2)
        rBoundingBoxesWithTolerance[i] = rBoundingBoxes[i] + Tolerance;

    for (IndexType i=1; i<size_vec; i+=2)
        rBoundingBoxesWithTolerance[i] = rBoundingBoxes[i] - Tolerance;
}

std::string BoundingBoxStringStream(const std::vector<double>& rBoundingBox)
{
    // xmax, xmin,  ymax, ymin,  zmax, zmin
    std::stringstream buffer;
    buffer << "[" << rBoundingBox[1] << " "    // xmin
                  << rBoundingBox[3] << " "    // ymin
                  << rBoundingBox[5] << "]|["  // zmin
                  << rBoundingBox[0] << " "    // xmax
                  << rBoundingBox[2] << " "    // ymax
                  << rBoundingBox[4] << "]";   // zmax
    return buffer.str();
}

bool PointIsInsideBoundingBox(const std::vector<double>& rBoundingBox,
                              const Point& rPoint)
{   // The Bounding Box should have some tolerance already!
    if (rPoint.X() < rBoundingBox[0] && rPoint.X() > rBoundingBox[1])   // check x-direction
    {
        if (rPoint.Y() < rBoundingBox[2] && rPoint.Y() > rBoundingBox[3])   // check y-direction
        {
            if (rPoint.Z() < rBoundingBox[4] && rPoint.Z() > rBoundingBox[5])   // check z-direction
                return true;
        }
    }
    return false;
}

} // namespace MapperUtilities
} // namespace Kratos.
