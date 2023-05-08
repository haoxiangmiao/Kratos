//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main author:     Sebastian Ares de Parga Regalado
//
// This file is partly copied from
// "DEMApplication/custom_utilities/omp_dem_search.h"
// and modified to create only once the binary tree

// System includes
#include <string>
#include <iostream>

// External includes
#include "concurrentqueue/concurrentqueue.h"

// Project includes
#include "utilities/assign_mpcs_to_neighbours_utility.h"
// #include "utilities/openmp_utils.h"

namespace Kratos 
{
///@addtogroup Kratos Core
///@{

///@name Kratos Classes
///@{

/**
 * @class AssignMPCsToNeighboursUtility
 * @ingroup Kratos Core
 * @brief Assing MPCs to Neighbouring Nodes
 * @details This class provides a method to search for neighbouring nodes of one node
 * within a given radius and assign a multipoint constraint (MPC) using radial basis functions.
 * @author Sebastian Ares de Parga Regalado
 */

/// Default constructor.
AssignMPCsToNeighboursUtility::AssignMPCsToNeighboursUtility(NodesContainerType& rStructureNodes){
    KRATOS_TRY;
    NodesContainerType::ContainerType& nodes_model_part = rStructureNodes.GetContainer();
    mpBins = Kratos::make_unique<NodeBinsType>(nodes_model_part.begin(), nodes_model_part.end());
    mMaxNumberOfNodes = rStructureNodes.size();
    KRATOS_CATCH(""); 
} 

// Search for the neighbouring nodes (in rStructureNodes) of each rNode on a given array of rNodes
void AssignMPCsToNeighboursUtility::SearchNodesInRadiusForNodes(
    NodesContainerType const& rNodes,
    const double Radius,
    VectorResultNodesContainerType& rResults,
    const double MinNumOfNeighNodes)
{
    KRATOS_TRY;
    NodesContainerType::ContainerType& nodes_array     = const_cast<NodesContainerType::ContainerType&>(rNodes.GetContainer());

    rResults.resize(nodes_array.size());

    // Declare a counter variable outside the loop as std::atomic<int>
    std::atomic<int> i(0);

    block_for_each(nodes_array, [&](Node<3>::Pointer& pNode)
    {
        double localRadius = Radius;

        IndexType it = i.fetch_add(1); // Atomically increment the counter and get the previous value

        while (rResults[i].size()<MinNumOfNeighNodes){
            rResults[i].clear();
            SearchNodesInRadiusForNode(pNode, localRadius, rResults[it]);
            localRadius += Radius;
        }
    });
    KRATOS_CATCH("");
}


//Search for the neighbouring nodes (in rStructureNodes) of the given rNode
void AssignMPCsToNeighboursUtility::SearchNodesInRadiusForNode(
    NodeType::Pointer pNode,
    double const Radius,
    ResultNodesContainerType& rResults)
{

    KRATOS_TRY;

    ResultNodesContainerType local_results(mMaxNumberOfNodes);
    std::size_t num_of_results = 0;
    auto results_iterator = local_results.begin();
    num_of_results = mpBins->SearchInRadius(*pNode, Radius, results_iterator, mMaxNumberOfNodes);
    rResults.insert(rResults.begin(), local_results.begin(), local_results.begin() + num_of_results);

    KRATOS_CATCH("");
}

void AssignMPCsToNeighboursUtility::GetDofsAndCoordinatesForNode(
    NodeType::Pointer pNode,
    const Variable<double>& rVariable,
    DofPointerVectorType& rCloud_of_dofs,
    array_1d<double,3>& rSlave_coordinates
    )
{
    KRATOS_TRY;
    rCloud_of_dofs.push_back(pNode->pGetDof(rVariable));
    rSlave_coordinates = pNode->Coordinates();
    KRATOS_CATCH("");
}

// Get Dofs and Coordinates arrays for a given variable double. (For nodes)
void AssignMPCsToNeighboursUtility::GetDofsAndCoordinatesForNodes(
    ResultNodesContainerType nodes_array,
    const Variable<double>& rVariable,
    DofPointerVectorType& rCloud_of_dofs,
    Matrix& rCloud_of_nodes_coordinates
    )
{
    KRATOS_TRY;
    rCloud_of_dofs.resize(nodes_array.size());
    rCloud_of_nodes_coordinates.resize(nodes_array.size(),3);
    for(unsigned int i = 0; i < nodes_array.size(); i++){
        rCloud_of_dofs[i] = nodes_array[i]->pGetDof(rVariable);
        noalias(row(rCloud_of_nodes_coordinates,i)) = nodes_array[i]->Coordinates();
    }
    KRATOS_CATCH("");
}

void AssignMPCsToNeighboursUtility::AssignRotationToNodes(
    NodesContainerType pNodes,
    Matrix RotationMatrix
    )
{
    //TODO: Do it in parallel
    KRATOS_TRY;
    NodesContainerType::ContainerType& nodes_array     = const_cast<NodesContainerType::ContainerType&>(pNodes.GetContainer());

    // Assign rotation to given nodes
    for(unsigned int i = 0; i < nodes_array.size(); i++){
        auto coordinate_vector = nodes_array[i]->GetInitialPosition().Coordinates();
        Vector rotated_position(coordinate_vector.size());
        noalias(rotated_position) = prod(RotationMatrix,coordinate_vector);
        nodes_array[i]->X() = rotated_position[0];
        nodes_array[i]->Y() = rotated_position[1];
        nodes_array[i]->Z() = rotated_position[2];
    }
    KRATOS_CATCH("");
}

void AssignMPCsToNeighboursUtility::AssignMPCsToNodes(
    NodesContainerType pNodes,
    double const Radius,
    ModelPart& rComputingModelPart,
    const Variable<double>& rVariable,
    double const MinNumOfNeighNodes
    )
{
    KRATOS_TRY;

    BuiltinTimer build_and_assign_mpcs;

    int prev_num_mpcs = rComputingModelPart.NumberOfMasterSlaveConstraints();

    NodesContainerType::ContainerType& nodes_array = const_cast<NodesContainerType::ContainerType&>(pNodes.GetContainer());

    ModelPart::MasterSlaveConstraintType const& r_clone_constraint = KratosComponents<MasterSlaveConstraint>::Get("LinearMasterSlaveConstraint");

    // Create a concurrent queue for constraints
    using MasterSlaveConstarintQueue = moodycamel::ConcurrentQueue<ModelPart::MasterSlaveConstraintType::Pointer>;

    MasterSlaveConstarintQueue concurrent_constraints;

    // Declare a counter variable outside the loop as std::atomic<int>
    std::atomic<int> i(0);

    block_for_each(nodes_array, [&](Node<3>::Pointer& pNode) {

        double local_radius = Radius;

        ResultNodesContainerType r_result(0);

        while (r_result.size() < MinNumOfNeighNodes) {
            r_result.clear();
            SearchNodesInRadiusForNode(pNode, local_radius, r_result);
            local_radius += Radius;
        }

        // Get Dofs and Coordinates
        DofPointerVectorType r_cloud_of_dofs, r_slave_dof;
        Matrix r_cloud_of_nodes_coordinates;
        array_1d<double, 3> r_slave_coordinates;
        GetDofsAndCoordinatesForNode(pNode, rVariable, r_slave_dof, r_slave_coordinates);
        GetDofsAndCoordinatesForNodes(r_result, rVariable, r_cloud_of_dofs, r_cloud_of_nodes_coordinates);

        // Calculate shape functions
        Vector r_n_container;
        RBFShapeFunctionsUtility::CalculateShapeFunctions(r_cloud_of_nodes_coordinates, r_slave_coordinates, r_n_container);

        // Create MPCs
        Matrix shape_matrix(1, r_n_container.size());
        noalias(row(shape_matrix, 0)) = r_n_container; // Shape functions matrix
        const Vector constant_vector = ZeroVector(r_n_container.size());
        IndexType it = i.fetch_add(1); // Atomically increment the counter and get the previous value
        concurrent_constraints.enqueue(r_clone_constraint.Create(prev_num_mpcs + it + 1, r_cloud_of_dofs, r_slave_dof, shape_matrix, constant_vector));
    });

    // Create a temporary container to store the master-slave constraints
    ConstraintContainerType temp_constraints;
    ModelPart::MasterSlaveConstraintType::Pointer p_constraint;

    // Dequeue the constraints from the concurrent queue and add them to the temporary container
    while (concurrent_constraints.try_dequeue(p_constraint)) {
        temp_constraints.push_back(p_constraint);
    }

    // Add the constraints to the rComputingModelPart in a single call
    rComputingModelPart.AddMasterSlaveConstraints(temp_constraints.begin(), temp_constraints.end());

    KRATOS_INFO("AssignMPCsToNeighboursUtility") << "Build and Assign MPCs Time: " << build_and_assign_mpcs.ElapsedSeconds() << std::endl;
    KRATOS_CATCH("");
}



}  // namespace Kratos.