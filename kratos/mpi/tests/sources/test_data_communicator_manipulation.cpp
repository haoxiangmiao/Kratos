//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Jordi Cotela
//
//

#include "includes/data_communicator.h"
#include "includes/parallel_environment.h"
#include "mpi/includes/mpi_data_communicator.h"

#include "testing/testing.h"

namespace Kratos {

namespace Testing {

// MPI Communicator splitting /////////////////////////////////////////////////

KRATOS_DISTRIBUTED_TEST_CASE_IN_SUITE(CreateMPIDataCommunicatorSplit, KratosMPICoreFastSuite)
{
    const DataCommunicator& r_comm = DataCommunicator::GetDefault();
    const int global_rank = r_comm.Rank();
    const int global_size = r_comm.Size();

    for (int i = 1; i < global_size; i++)
    {
        int color = global_rank < i ? 0 : 1;
        int key = global_rank < i ? global_rank : global_size - global_rank;
        std::stringstream name;
        name << "split_communicator_step_" << i;

        const DataCommunicator& r_split_comm = MPIDataCommunicator::SplitDataCommunicator(r_comm, color, key, name.str());

        int expected_size = global_rank < i ? i : global_size - i;
        int expected_rank = global_rank < i ? key : key - 1;
        // MPI_Comm_split assigns rank by order of increasing key.
        // Here keys in the global_rank >= i side of the split range between 1 and (global_size-i)
        KRATOS_CHECK_EQUAL(r_split_comm.Size(), expected_size);
        KRATOS_CHECK_EQUAL(r_split_comm.Rank(), expected_rank);

        ParallelEnvironment::UnregisterDataCommunicator(name.str());
    }
}

KRATOS_DISTRIBUTED_TEST_CASE_IN_SUITE(CreateMPIDataCommunicatorFromRanks, KratosMPICoreFastSuite)
{
    const DataCommunicator& r_comm = DataCommunicator::GetDefault();
    const int global_size = r_comm.Size();

    if (global_size > 1) // This test assumes at least two processes
    {
        // Create a communicator using all ranks except the first
        std::vector<int> rank_list(global_size-1);
        for (int i = 0; i < global_size-1; i++)
        {
            rank_list[i] = i+1;
        }

        // Note: communicator creation is a collective on all ranks of the "parent" communicator,
        // even if they do not participate in the new one
        const std::string new_comm_name("NewCommunicator");
        const DataCommunicator& r_new_comm = MPIDataCommunicator::CreateDataCommunicatorFromRanks(
            r_comm, rank_list, new_comm_name);

        const int global_rank = r_comm.Rank();
        if (global_rank == 0)
        {
            // The communicator is equal to MPI_COMM_NULL for ranks that do not participate in it
            KRATOS_CHECK(r_new_comm.IsNullOnThisRank());
            KRATOS_CHECK_IS_FALSE(r_new_comm.IsDefinedOnThisRank());
        }
        else
        {
            KRATOS_CHECK_EQUAL(r_new_comm.Rank(), global_rank-1);
            KRATOS_CHECK_EQUAL(r_new_comm.Size(), global_size-1);
        }

        ParallelEnvironment::UnregisterDataCommunicator("NewCommunicator");
    }
}

KRATOS_DISTRIBUTED_TEST_CASE_IN_SUITE(CreateMPIDataCommunicatorUnion, KratosMPICoreFastSuite)
{
    const DataCommunicator& r_comm = DataCommunicator::GetDefault();
    const int global_size = r_comm.Size();

    if (global_size > 2) // This test assumes at least three processes
    {
        // Create a communicator using all ranks except the first and another using all ranks except the last
        std::vector<int> all_except_first(global_size-1);
        std::vector<int> all_except_last(global_size-1);
        for (int i = 0; i < global_size-1; i++)
        {
            all_except_first[i] = i+1;
            all_except_last[i] = i;
        }

        const DataCommunicator& r_all_except_first_comm = MPIDataCommunicator::CreateDataCommunicatorFromRanks(
            r_comm, all_except_first, "AllExceptFirst");

        const DataCommunicator& r_all_except_last_comm = MPIDataCommunicator::CreateDataCommunicatorFromRanks(
            r_comm, all_except_last, "AllExceptLast");

        const DataCommunicator& r_union_comm = MPIDataCommunicator::CreateUnionDataCommunicator(
            r_all_except_first_comm, r_all_except_last_comm, r_comm, "UnionCommunicator");

        // our union MPI comm involves all ranks in the parent communicator, so it should be defined everywhere
        KRATOS_CHECK_IS_FALSE(r_union_comm.IsNullOnThisRank());

        KRATOS_CHECK_EQUAL(r_union_comm.Rank(), r_comm.Rank());
        KRATOS_CHECK_EQUAL(r_union_comm.Size(), global_size);

        // Clean up the ParallelEnvironment after test
        ParallelEnvironment::UnregisterDataCommunicator("AllExceptFirst");
        ParallelEnvironment::UnregisterDataCommunicator("AllExceptLast");
        ParallelEnvironment::UnregisterDataCommunicator("UnionCommunicator");
    }
}

KRATOS_DISTRIBUTED_TEST_CASE_IN_SUITE(CreateMPIDataCommunicatorIntersection, KratosMPICoreFastSuite)
{
    const DataCommunicator& r_comm = DataCommunicator::GetDefault();
    const int global_size = r_comm.Size();

    if (global_size > 2) // This test assumes at least three processes
    {
        // Create a communicator using all ranks except the first and another using all ranks except the last
        std::vector<int> all_except_first(global_size-1);
        std::vector<int> all_except_last(global_size-1);
        for (int i = 0; i < global_size-1; i++)
        {
            all_except_first[i] = i+1;
            all_except_last[i] = i;
        }

        const int global_rank = r_comm.Rank();

        const DataCommunicator& r_all_except_first_comm = MPIDataCommunicator::CreateDataCommunicatorFromRanks(
            r_comm, all_except_first, "_AllExceptFirst");

        const DataCommunicator& r_all_except_last_comm = MPIDataCommunicator::CreateDataCommunicatorFromRanks(
            r_comm, all_except_last, "_AllExceptLast");

        const DataCommunicator& r_intersection_comm = MPIDataCommunicator::CreateIntersectionDataCommunicator(
            r_all_except_first_comm, r_all_except_last_comm, r_comm, "IntersectionCommunicator");

        if ( (global_rank == 0) || (global_rank == global_size-1 ) )
        {
            // The first and last ranks do not participate in the intersection communicator
            KRATOS_CHECK(r_intersection_comm.IsNullOnThisRank());
        }
        else
        {
            KRATOS_CHECK_EQUAL(r_intersection_comm.Rank(), r_comm.Rank() - 1);
            KRATOS_CHECK_EQUAL(r_intersection_comm.Size(), r_comm.Size() - 2);
        }

        // Clean up the ParallelEnvironment after test
        ParallelEnvironment::UnregisterDataCommunicator("_AllExceptFirst");
        ParallelEnvironment::UnregisterDataCommunicator("_AllExceptLast");
        ParallelEnvironment::UnregisterDataCommunicator("IntersectionCommunicator");
    }
}

KRATOS_TEST_CASE_IN_SUITE(ParallelEnvironmentRegisterDataCommunicator, KratosMPICoreFastSuite)
{
    const DataCommunicator& r_comm = ParallelEnvironment::GetDefaultDataCommunicator();

    MPIDataCommunicator::SplitDataCommunicator(r_comm, r_comm.Rank() % 2, 0, "EvenOdd");

    KRATOS_CHECK(ParallelEnvironment::HasDataCommunicator("EvenOdd"));

    ParallelEnvironment::UnregisterDataCommunicator("EvenOdd");

    KRATOS_CHECK_IS_FALSE(ParallelEnvironment::HasDataCommunicator("EvenOdd"));

    MPIDataCommunicator::SplitDataCommunicator(r_comm, r_comm.Rank() % 2, 0, "EvenOdd");

    KRATOS_CHECK(ParallelEnvironment::HasDataCommunicator("EvenOdd"));
}

}
}