//  KRATOS  _____     _ _ _
//         |_   _| __(_) (_)_ __   ___  ___
//           | || '__| | | | '_ \ / _ \/ __|
//           | || |  | | | | | | | (_) \__
//           |_||_|  |_|_|_|_| |_|\___/|___/ APPLICATION
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Riccardo Rossi
//  Collaborators:   Vicente Mataix
//

#pragma once

// System includes
#include <unordered_set>

// External includes

/* Trilinos includes */
#include <Epetra_FECrsGraph.h>
#include <Epetra_IntVector.h>

// Project includes
#include "trilinos_space.h"
#include "solving_strategies/builder_and_solvers/builder_and_solver.h"
#include "utilities/timer.h"
#include "utilities/builtin_timer.h"
#include "utilities/parallel_utilities.h"
// #include "utilities/atomic_utilities.h" # TODO

#if !defined(START_TIMER)
#define START_TIMER(label, rank) \
    if (mrComm.MyPID() == rank)  \
        Timer::Start(label);
#endif
#if !defined(STOP_TIMER)
#define STOP_TIMER(label, rank) \
    if (mrComm.MyPID() == rank) \
        Timer::Stop(label);
#endif

namespace Kratos {

///@name Kratos Globals
///@{

///@}
///@name Type Definitions
///@{

///@}
///@name  Enum's
///@{

///@}
///@name  Functions
///@{

///@}
///@name Kratos Classes
///@{

/**
 * @class TrilinosBlockBuilderAndSolver
 * @ingroup TrilinosApplication
 * @brief Current class provides an implementation for trilinos builder and
 * solving operations.
 * @details The RHS is constituted by the unbalanced loads (residual)
 * Degrees of freedom are reordered putting the restrained degrees of freedom at
 * the end of the system ordered in reverse order with respect to the DofSet.
 * Imposition of the dirichlet conditions is naturally dealt with as the
 * residual already contains this information. Calculation of the reactions
 * involves a cost very similar to the calculation of the total residual
 * @author Riccardo Rossi
 * @author Vicente Mataix Ferrandiz (MPC)
 * @note Should be TrilinosResidualBasedBlockBuilderAndSolver?
 */
template <class TSparseSpace,
          class TDenseSpace,  //= DenseSpace<double>,
          class TLinearSolver //= LinearSolver<TSparseSpace,TDenseSpace>
          >
class TrilinosBlockBuilderAndSolver
    : public BuilderAndSolver<TSparseSpace, TDenseSpace, TLinearSolver> {
public:
    ///@name Type Definitions
    ///@{

    /// Definition of the flags
    KRATOS_DEFINE_LOCAL_FLAG( SILENT_WARNINGS );

    /// Definition of the pointer
    KRATOS_CLASS_POINTER_DEFINITION(TrilinosBlockBuilderAndSolver);

    /// Definition of the base class
    using BaseType = BuilderAndSolver<TSparseSpace, TDenseSpace, TLinearSolver>;

    /// The size_t types
    using SizeType = std::size_t;
    using IndexType = std::size_t;

    /// Definition of the classes from the base class
    using TSchemeType = typename BaseType::TSchemeType;
    using DofsArrayType = typename BaseType::DofsArrayType;

    /// Epetra definitions
    using EpetraCommunicatorType = Epetra_MpiComm;

    /// DoF types definition
    using NodeType = Node<3>;

    /// Defining the sparse matrices and vectors
    using TSystemMatrixType = typename BaseType::TSystemMatrixType;
    using TSystemVectorType = typename BaseType::TSystemVectorType;

    /// Defining the local matrices and vectors
    using LocalSystemMatrixType = typename BaseType::LocalSystemMatrixType;
    using LocalSystemVectorType = typename BaseType::LocalSystemVectorType;

    /// Definition of the pointer types
    using TSystemMatrixPointerType = typename BaseType::TSystemMatrixPointerType;
    using TSystemVectorPointerType = typename BaseType::TSystemVectorPointerType;

    ///@}
    ///@name Life Cycle
    ///@{

    /**
     * @brief Default constructor (empty)
     */
    explicit TrilinosBlockBuilderAndSolver() : BaseType()
    {
    }

    /**
     * @brief Default constructor.
     */
    explicit TrilinosBlockBuilderAndSolver(EpetraCommunicatorType& rComm,
                                  int GuessRowSize,
                                  typename TLinearSolver::Pointer pNewLinearSystemSolver)
        : BaseType(pNewLinearSystemSolver),
          mrComm(rComm),
          mGuessRowSize(GuessRowSize)
    { }

    /**
     * @brief Default constructor. (with parameters)
     */
    explicit TrilinosBlockBuilderAndSolver(
        EpetraCommunicatorType& rComm,
        typename TLinearSolver::Pointer pNewLinearSystemSolver,
        Parameters ThisParameters
        ) : BaseType(pNewLinearSystemSolver),
            mrComm(rComm)
    {
        // Validate and assign defaults
        ThisParameters = this->ValidateAndAssignParameters(ThisParameters, this->GetDefaultParameters());
        this->AssignSettings(ThisParameters);
    }

    /**
     * Copy constructor
     */
    TrilinosBlockBuilderAndSolver(const TrilinosBlockBuilderAndSolver& rOther) = delete;

    /**
     * Assignment operator
     */
    TrilinosBlockBuilderAndSolver& operator=(const TrilinosBlockBuilderAndSolver& rOther) = delete;

    // TODO: In order to create a Create method something must be done about the EpetraCommunicator. Maybe ge it from ProcessInfo

    ///@}
    ///@name Operators
    ///@{

    ///@}
    ///@name Operations
    ///@{

    /**
     * @brief Function to perform the build the system matrix and the residual
     * vector
     * @param pScheme The integration scheme considered
     * @param rModelPart The model part of the problem to solve
     * @param rA The LHS matrix
     * @param rb The RHS vector
     */
    void Build(typename TSchemeType::Pointer pScheme,
               ModelPart& rModelPart,
               TSystemMatrixType& rA,
               TSystemVectorType& rb) override
    {
        KRATOS_TRY

        KRATOS_ERROR_IF(!pScheme) << "No scheme provided!" << std::endl;

        // Resetting to zero the vector of reactions
        TSparseSpace::SetToZero(*BaseType::mpReactionsVector);

        // Contributions to the system
        LocalSystemMatrixType LHS_Contribution = LocalSystemMatrixType(0, 0);
        LocalSystemVectorType RHS_Contribution = LocalSystemVectorType(0);

        // Vector containing the localization in the system of the different terms
        Element::EquationIdVectorType equation_ids_vector;
        const ProcessInfo& r_current_process_info = rModelPart.GetProcessInfo();

        BuiltinTimer build_timer;

        // Assemble all elements
        for (auto it = rModelPart.Elements().ptr_begin(); it < rModelPart.Elements().ptr_end(); it++) {
            // Detect if the element is active or not. If the user did not make
            // any choice the element is active by default
            const bool element_is_active = !((*it)->IsDefined(ACTIVE)) || (*it)->Is(ACTIVE);

            if (element_is_active) {
                // Calculate elemental contribution
                pScheme->CalculateSystemContributions(**it, LHS_Contribution, RHS_Contribution, equation_ids_vector, r_current_process_info);

                // Assemble the elemental contribution
                TSparseSpace::AssembleLHS(rA, LHS_Contribution, equation_ids_vector);
                TSparseSpace::AssembleRHS(rb, RHS_Contribution, equation_ids_vector);
            }
        }

        LHS_Contribution.resize(0, 0, false);
        RHS_Contribution.resize(0, false);

        // Assemble all conditions
        for (auto it = rModelPart.Conditions().ptr_begin(); it < rModelPart.Conditions().ptr_end(); it++) {
            // detect if the element is active or not. If the user did not make
            // any choice the element is active by default
            const bool condition_is_active = !((*it)->IsDefined(ACTIVE)) || (*it)->Is(ACTIVE);
            if (condition_is_active) {
                // Calculate elemental contribution
                pScheme->CalculateSystemContributions(**it, LHS_Contribution, RHS_Contribution, equation_ids_vector, r_current_process_info);

                // Assemble the condition contribution
                TSparseSpace::AssembleLHS(rA, LHS_Contribution, equation_ids_vector);
                TSparseSpace::AssembleRHS(rb, RHS_Contribution, equation_ids_vector);
            }
        }

        // Finalizing the assembly
        rA.GlobalAssemble();
        rb.GlobalAssemble();

        KRATOS_INFO_IF("TrilinosBlockBuilderAndSolver", BaseType::GetEchoLevel() >= 1) << "Build time: " << build_timer.ElapsedSeconds() << std::endl;

        KRATOS_CATCH("")
    }

    /**
     * @brief Function to perform the building of the LHS
     * @details Depending on the implementation choosen the size of the matrix
     * could be equal to the total number of Dofs or to the number of
     * unrestrained dofs
     * @param pScheme The integration scheme considered
     * @param rModelPart The model part of the problem to solve
     * @param rA The LHS matrix
     */
    void BuildLHS(typename TSchemeType::Pointer pScheme,
                  ModelPart& rModelPart,
                  TSystemMatrixType& rA) override
    {
        KRATOS_TRY
        // Resetting to zero the vector of reactions
        TSparseSpace::SetToZero(*BaseType::mpReactionsVector);

        // Contributions to the system
        LocalSystemMatrixType LHS_Contribution = LocalSystemMatrixType(0, 0);
        LocalSystemVectorType RHS_Contribution = LocalSystemVectorType(0);

        // Vector containing the localization in the system of the different terms
        Element::EquationIdVectorType equation_ids_vector;
        const ProcessInfo& r_current_process_info = rModelPart.GetProcessInfo();

        BuiltinTimer build_timer;

        // Assemble all elements
        for (auto it = rModelPart.Elements().ptr_begin(); it < rModelPart.Elements().ptr_end(); it++) {
            pScheme->CalculateLHSContribution(**it, LHS_Contribution, equation_ids_vector, r_current_process_info);

            // Assemble the elemental contribution
            TSparseSpace::AssembleLHS(rA, LHS_Contribution, equation_ids_vector);
        }

        LHS_Contribution.resize(0, 0, false);

        // Assemble all conditions
        for (auto it = rModelPart.Conditions().ptr_begin(); it < rModelPart.Conditions().ptr_end(); it++) {
            // calculate elemental contribution
            pScheme->CalculateLHSContribution(**it, LHS_Contribution, equation_ids_vector, r_current_process_info);

            // Assemble the elemental contribution
            TSparseSpace::AssembleLHS(rA, LHS_Contribution, equation_ids_vector);
        }

        // finalizing the assembly
        rA.GlobalAssemble();

        KRATOS_INFO_IF("TrilinosBlockBuilderAndSolver", BaseType::GetEchoLevel() >= 1) << "Build time LHS: " << build_timer.ElapsedSeconds() << std::endl;

        KRATOS_CATCH("")
    }

    /**
     * @brief Build a rectangular matrix of size n*N where "n" is the number of
     * unrestrained degrees of freedom and "N" is the total number of degrees of
     * freedom involved.
     * @details This matrix is obtained by building the total matrix without the
     * lines corresponding to the fixed degrees of freedom (but keeping the
     * columns!!)
     * @param pScheme The integration scheme considered
     * @param rModelPart The model part of the problem to solve
     * @param A The LHS matrix
     */
    void BuildLHS_CompleteOnFreeRows(typename TSchemeType::Pointer pScheme,
                                     ModelPart& rModelPart,
                                     TSystemMatrixType& A) override
    {
        KRATOS_ERROR << "Method BuildLHS_CompleteOnFreeRows not implemented in "
                        "Trilinos Builder And Solver"
                     << std::endl;
    }

    /**
     * @brief This is a call to the linear system solver
     * @param A The LHS matrix
     * @param Dx The Unknowns vector
     * @param b The RHS vector
     */
    void SystemSolveWithPhysics(TSystemMatrixType& rA,
                                TSystemVectorType& rDx,
                                TSystemVectorType& rb,
                                ModelPart& rModelPart)
    {
        KRATOS_TRY

        double norm_b;
        if (TSparseSpace::Size(rb) != 0)
            norm_b = TSparseSpace::TwoNorm(rb);
        else
            norm_b = 0.00;

        if (norm_b != 0.00) {
            if (BaseType::mpLinearSystemSolver->AdditionalPhysicalDataIsNeeded())
                BaseType::mpLinearSystemSolver->ProvideAdditionalData(
                    rA, rDx, rb, BaseType::mDofSet, rModelPart);

            BaseType::mpLinearSystemSolver->Solve(rA, rDx, rb);
        }
        else {
            TSparseSpace::SetToZero(rDx);
            KRATOS_WARNING(
                "TrilinosResidualBasedBlockBuilderAndSolver")
                << "ATTENTION! setting the RHS to zero!" << std::endl;
        }

        // prints informations about the current time
        KRATOS_INFO_IF("TrilinosResidualBasedBlockBuilderAndSolver", (BaseType::GetEchoLevel() > 1))
            << *(BaseType::mpLinearSystemSolver) << std::endl;

        KRATOS_CATCH("")
    }

    /**
     * @brief Function to perform the building and solving phase at the same time.
     * @details It is ideally the fastest and safer function to use when it is
     * possible to solve just after building
     * @param pScheme The integration scheme considered
     * @param rModelPart The model part of the problem to solve
     * @param rA The LHS matrix
     * @param rDx The Unknowns vector
     * @param rb The RHS vector
     */
    void BuildAndSolve(typename TSchemeType::Pointer pScheme,
                       ModelPart& rModelPart,
                       TSystemMatrixType& rA,
                       TSystemVectorType& rDx,
                       TSystemVectorType& rb) override
    {
        KRATOS_TRY

        START_TIMER("Build", 0)

        Build(pScheme, rModelPart, rA, rb);

        STOP_TIMER("Build", 0)

        if(rModelPart.MasterSlaveConstraints().size() != 0) {
            const auto timer_constraints = BuiltinTimer();
            START_TIMER("ApplyConstraints", 0)
            ApplyConstraints(pScheme, rModelPart, rA, rb);
            STOP_TIMER("ApplyConstraints", 0)
            KRATOS_INFO_IF("TrilinosBlockBuilderAndSolver", BaseType::GetEchoLevel() >=1) << "Constraints build time: " << timer_constraints.ElapsedSeconds() << std::endl;
        }

        // Apply dirichlet conditions
        ApplyDirichletConditions(pScheme, rModelPart, rA, rDx, rb);

        KRATOS_INFO_IF("TrilinosBlockBuilderAndSolver", BaseType::GetEchoLevel() == 3)
            << "\nBefore the solution of the system"
            << "\nSystem Matrix = " << rA << "\nunknowns vector = " << rDx
            << "\nRHS vector = " << rb << std::endl;

        START_TIMER("Solve", 0)

        const auto solve_timer = BuiltinTimer();

        SystemSolveWithPhysics(rA, rDx, rb, rModelPart);

        KRATOS_INFO_IF("TrilinosBlockBuilderAndSolver", BaseType::GetEchoLevel() >=1) << "System solve time: " << solve_timer.ElapsedSeconds() << std::endl;

        STOP_TIMER("Solve", 0)

        KRATOS_INFO_IF("TrilinosBlockBuilderAndSolver", BaseType::GetEchoLevel() == 3)
            << "\nAfter the solution of the system"
            << "\nSystem Matrix = " << rA << "\nUnknowns vector = " << rDx
            << "\nRHS vector = " << rb << std::endl;
        KRATOS_CATCH("")
    }

    /**
     * @brief Corresponds to the previous, but the System's matrix is considered
     * already built and only the RHS is built again
     * @param pScheme The integration scheme considered
     * @param rModelPart The model part of the problem to solve
     * @param rA The LHS matrix
     * @param rDx The Unknowns vector
     * @param rb The RHS vector
     */
    void BuildRHSAndSolve(typename TSchemeType::Pointer pScheme,
                          ModelPart& rModelPart,
                          TSystemMatrixType& rA,
                          TSystemVectorType& rDx,
                          TSystemVectorType& rb) override
    {
        KRATOS_TRY

        BuildRHS(pScheme, rModelPart, rb);

        if(rModelPart.MasterSlaveConstraints().size() != 0) {
            START_TIMER("ApplyRHSConstraints", 0)
            ApplyRHSConstraints(pScheme, rModelPart, rb);
            STOP_TIMER("ApplyRHSConstraints", 0)
        }

        const auto solve_timer = BuiltinTimer();
        START_TIMER("Solve", 0)

        SystemSolveWithPhysics(rA, rDx, rb, rModelPart);

        STOP_TIMER("Solve", 0)

        KRATOS_INFO_IF("TrilinosBlockBuilderAndSolver", BaseType::GetEchoLevel() >=1) << "System solve time: " << solve_timer.ElapsedSeconds() << std::endl;

        KRATOS_INFO_IF("TrilinosBlockBuilderAndSolver", ( this->GetEchoLevel() == 3)) << "After the solution of the system" << "\nSystem Matrix = " << rA << "\nUnknowns vector = " << rDx << "\nRHS vector = " << rb << std::endl;

        KRATOS_CATCH("")
    }

    /**
     * @brief Function to perform the build of the RHS.
     * @details The vector could be sized as the total number of dofs or as the
     * number of unrestrained ones
     * @param pScheme The integration scheme considered
     * @param rModelPart The model part of the problem to solve
     */
    void BuildRHS(typename TSchemeType::Pointer pScheme,
                  ModelPart& rModelPart,
                  TSystemVectorType& rb) override
    {
        KRATOS_TRY

        START_TIMER("BuildRHS ", 0)
        // Resetting to zero the vector of reactions
        TSparseSpace::SetToZero(*BaseType::mpReactionsVector);

        // Contributions to the system
        LocalSystemMatrixType LHS_Contribution = LocalSystemMatrixType(0, 0);
        LocalSystemVectorType RHS_Contribution = LocalSystemVectorType(0);

        // Vector containing the localization in the system of the different terms
        Element::EquationIdVectorType equation_ids_vector;
        const ProcessInfo& r_current_process_info = rModelPart.GetProcessInfo();

        // Assemble all elements
        for (auto it = rModelPart.Elements().ptr_begin(); it < rModelPart.Elements().ptr_end(); it++) {
            // Calculate elemental Right Hand Side Contribution
            pScheme->CalculateRHSContribution(**it, RHS_Contribution, equation_ids_vector, r_current_process_info);

            // assemble the elemental contribution
            TSparseSpace::AssembleRHS(rb, RHS_Contribution, equation_ids_vector);
        }

        RHS_Contribution.resize(0, false);

        // Assemble all conditions
        for (auto it = rModelPart.Conditions().ptr_begin(); it < rModelPart.Conditions().ptr_end(); it++) {
            // calculate elemental contribution
            pScheme->CalculateRHSContribution(**it, RHS_Contribution, equation_ids_vector, r_current_process_info);

            // Assemble the elemental contribution
            TSparseSpace::AssembleRHS(rb, RHS_Contribution, equation_ids_vector);
        }

        // Finalizing the assembly
        rb.GlobalAssemble();

        STOP_TIMER("BuildRHS ", 0)

        KRATOS_CATCH("")
    }

    /**
     * @brief Builds the list of the DofSets involved in the problem by "asking"
     * to each element and condition its Dofs.
     * @details The list of dofs is stores inside the BuilderAndSolver as it is
     * closely connected to the way the matrix and RHS are built
     * @param pScheme The integration scheme considered
     * @param rModelPart The model part of the problem to solve
     */
    void SetUpDofSet(typename TSchemeType::Pointer pScheme, ModelPart& rModelPart) override
    {
        KRATOS_TRY

        typedef Element::DofsVectorType DofsVectorType;
        // Gets the array of elements from the modeler
        auto& r_elements_array = rModelPart.GetCommunicator().LocalMesh().Elements();
        DofsVectorType dof_list;
        const ProcessInfo& r_current_process_info = rModelPart.GetProcessInfo();

        DofsArrayType temp_dofs_array;
        IndexType guess_num_dofs = rModelPart.GetCommunicator().LocalMesh().NumberOfNodes() * 3;
        temp_dofs_array.reserve(guess_num_dofs);
        BaseType::mDofSet = DofsArrayType();

        // Taking dofs of elements
        for (auto it_elem = r_elements_array.ptr_begin(); it_elem != r_elements_array.ptr_end(); ++it_elem) {
            pScheme->GetDofList(**it_elem, dof_list, r_current_process_info);
            for (auto i_dof = dof_list.begin(); i_dof != dof_list.end(); ++i_dof)
                temp_dofs_array.push_back(*i_dof);
        }

        // Taking dofs of conditions
        auto& r_conditions_array = rModelPart.Conditions();
        for (auto it_cond = r_conditions_array.ptr_begin(); it_cond != r_conditions_array.ptr_end(); ++it_cond) {
            pScheme->GetDofList(**it_cond, dof_list, r_current_process_info);
            for (auto i_dof = dof_list.begin(); i_dof != dof_list.end(); ++i_dof)
                temp_dofs_array.push_back(*i_dof);
        }

        // TODO: Add constraints

        temp_dofs_array.Unique();
        BaseType::mDofSet = temp_dofs_array;

        // Throws an exception if there are no Degrees of freedom involved in
        // the analysis
        KRATOS_ERROR_IF(rModelPart.GetCommunicator().GetDataCommunicator().SumAll(BaseType::mDofSet.size()) == 0) << "No degrees of freedom!";

#ifdef KRATOS_DEBUG
        // If reactions are to be calculated, we check if all the dofs have
        // reactions defined This is to be done only in debug mode
        if (BaseType::GetCalculateReactionsFlag()) {
            for (auto dof_iterator = BaseType::mDofSet.begin();
                 dof_iterator != BaseType::mDofSet.end(); ++dof_iterator) {
                KRATOS_ERROR_IF_NOT(dof_iterator->HasReaction())
                    << "Reaction variable not set for the following : " << std::endl
                    << "Node : " << dof_iterator->Id() << std::endl
                    << "Dof : " << (*dof_iterator) << std::endl
                    << "Not possible to calculate reactions." << std::endl;
            }
        }
#endif
        BaseType::mDofSetIsInitialized = true;

        KRATOS_CATCH("")
    }

    /**
     * @brief Organises the dofset in order to speed up the building phase
     *          Sets equation id for degrees of freedom
     * @param rModelPart The model part of the problem to solve
     */
    void SetUpSystem(ModelPart& rModelPart) override
    {
        int free_size = 0;
        auto& r_comm = rModelPart.GetCommunicator();
        const auto& r_data_comm = r_comm.GetDataCommunicator();
        int current_rank = r_comm.MyPID();

        // Calculating number of fixed and free dofs
        for (const auto& r_dof : BaseType::mDofSet)
            if (r_dof.GetSolutionStepValue(PARTITION_INDEX) == current_rank)
                free_size++;

        // Calculating the total size and required offset
        int free_offset;
        int global_size;

        // The correspounding offset by the sum of the sizes in thread with
        // inferior current_rank
        free_offset = r_data_comm.ScanSum(free_size);

        // The total size by the sum of all size in all threads
        global_size = r_data_comm.SumAll(free_size);

        // finding the offset for the begining of the partition
        free_offset -= free_size;

        // Now setting the equation id with .
        for (auto& r_dof : BaseType::mDofSet)
            if (r_dof.GetSolutionStepValue(PARTITION_INDEX) == current_rank)
                r_dof.SetEquationId(free_offset++);

        // TODO
        // BaseType::mEquationSystemSize = BaseType::mDofSet.size();

        // IndexPartition<std::size_t>(BaseType::mDofSet.size()).for_each([&, this](std::size_t Index){
        //     typename DofsArrayType::iterator dof_iterator = this->mDofSet.begin() + Index;
        //     dof_iterator->SetEquationId(Index);
        // });

        BaseType::mEquationSystemSize = global_size;
        mLocalSystemSize = free_size;
        KRATOS_INFO_IF_ALL_RANKS("TrilinosBlockBuilderAndSolver", BaseType::GetEchoLevel() > 1)
            << "\n    BaseType::mEquationSystemSize = " << BaseType::mEquationSystemSize
            << "\n    mLocalSystemSize = " << mLocalSystemSize
            << "\n    free_offset = " << free_offset << std::endl;

        // by Riccardo ... it may be wrong!
        mFirstMyId = free_offset - mLocalSystemSize;
        mLastMyId = mFirstMyId + mLocalSystemSize;

        r_comm.SynchronizeDofs();
    }

    /**
     * @brief Resizes the system matrix and the vector according to the number
     * of dos in the current rModelPart. This function also decides on the
     * sparsity pattern and the graph of the trilinos csr matrix
     * @param pScheme The integration scheme considered
     * @param rpA The LHS matrix
     * @param rpDx The Unknowns vector
     * @param rpd The RHS vector
     * @param rModelPart The model part of the problem to solve
     */
    void ResizeAndInitializeVectors(
        typename TSchemeType::Pointer pScheme,
        TSystemMatrixPointerType& rpA,
        TSystemVectorPointerType& rpDx,
        TSystemVectorPointerType& rpb,
        ModelPart& rModelPart
        ) override
    {
        KRATOS_TRY

        // TODO: Move to ConstructMatrixStructure

        // Resizing the system vectors and matrix
        if (rpA == nullptr || TSparseSpace::Size1(*rpA) == 0 ||
            BaseType::GetReshapeMatrixFlag() == true) { // if the matrix is not initialized
            IndexType number_of_local_dofs = mLastMyId - mFirstMyId;
            int temp_size = number_of_local_dofs;
            if (temp_size < 1000) {
                temp_size = 1000;
            }
            std::vector<int> temp(temp_size, 0);

            // TODO: Check if these should be local elements and conditions
            auto& r_elements_array = rModelPart.Elements();
            auto& r_conditions_array = rModelPart.Conditions();

            // Generate map - use the "temp" array here
            for (IndexType i = 0; i != number_of_local_dofs; i++) {
                temp[i] = mFirstMyId + i;
            }
            Epetra_Map my_map(-1, number_of_local_dofs, temp.data(), 0, mrComm);

            // Create and fill the graph of the matrix --> the temp array is
            // reused here with a different meaning
            Epetra_FECrsGraph Agraph(Copy, my_map, mGuessRowSize);
            Element::EquationIdVectorType equation_ids_vector;
            const ProcessInfo& r_current_process_info = rModelPart.GetProcessInfo();

            // Assemble all elements
            for (auto it_elem = r_elements_array.ptr_begin(); it_elem != r_elements_array.ptr_end(); ++it_elem) {
                pScheme->EquationId(**it_elem, equation_ids_vector, r_current_process_info);

                // Filling the list of active global indices (non fixed)
                IndexType num_active_indices = 0;
                for (IndexType i = 0; i < equation_ids_vector.size(); i++) {
                    temp[num_active_indices] = equation_ids_vector[i];
                    num_active_indices += 1;
                }

                if (num_active_indices != 0) {
                    const int ierr = Agraph.InsertGlobalIndices(num_active_indices, temp.data(), num_active_indices, temp.data());
                    KRATOS_ERROR_IF(ierr < 0) << ": Epetra failure in Graph.InsertGlobalIndices. Error code: " << ierr << std::endl;
                }
                std::fill(temp.begin(), temp.end(), 0);
            }

            // Assemble all conditions
            for (auto it_cond = r_conditions_array.ptr_begin(); it_cond != r_conditions_array.ptr_end(); ++it_cond) {
                pScheme->EquationId(**it_cond, equation_ids_vector, r_current_process_info);

                // Filling the list of active global indices (non fixed)
                IndexType num_active_indices = 0;
                for (IndexType i = 0; i < equation_ids_vector.size(); i++) {
                    temp[num_active_indices] = equation_ids_vector[i];
                    num_active_indices += 1;
                }

                if (num_active_indices != 0) {
                    const int ierr = Agraph.InsertGlobalIndices(num_active_indices, temp.data(), num_active_indices, temp.data());
                    KRATOS_ERROR_IF(ierr < 0) << ": Epetra failure in Graph.InsertGlobalIndices. Error code: " << ierr << std::endl;
                }
                std::fill(temp.begin(), temp.end(), 0);
            }

            // Finalizing graph construction
            const int ierr = Agraph.GlobalAssemble();
            KRATOS_ERROR_IF(ierr < 0) << ": Epetra failure in Graph.InsertGlobalIndices. Error code: " << ierr << std::endl;

            // Generate a new matrix pointer according to this graph
            TSystemMatrixPointerType p_new_A = TSystemMatrixPointerType(new TSystemMatrixType(Copy, Agraph));
            rpA.swap(p_new_A);

            // Generate new vector pointers according to the given map
            if (rpb == nullptr || TSparseSpace::Size(*rpb) != BaseType::mEquationSystemSize) {
                TSystemVectorPointerType p_new_b = TSystemVectorPointerType(new TSystemVectorType(my_map));
                rpb.swap(p_new_b);
            }
            if (rpDx == nullptr || TSparseSpace::Size(*rpDx) != BaseType::mEquationSystemSize) {
                TSystemVectorPointerType p_new_Dx = TSystemVectorPointerType(new TSystemVectorType(my_map));
                rpDx.swap(p_new_Dx);
            }
            if (BaseType::mpReactionsVector == nullptr) { // if the pointer is not initialized initialize it to an
                                                          // empty matrix
                TSystemVectorPointerType pNewReactionsVector = TSystemVectorPointerType(new TSystemVectorType(my_map));
                BaseType::mpReactionsVector.swap(pNewReactionsVector);
            }
        } else if (BaseType::mpReactionsVector == nullptr && this->mCalculateReactionsFlag) {
            TSystemVectorPointerType pNewReactionsVector =
                TSystemVectorPointerType(new TSystemVectorType(rpDx->Map()));
            BaseType::mpReactionsVector.swap(pNewReactionsVector);
        } else {
            if (TSparseSpace::Size1(*rpA) == 0 ||
                TSparseSpace::Size1(*rpA) != BaseType::mEquationSystemSize ||
                TSparseSpace::Size2(*rpA) != BaseType::mEquationSystemSize) {
                KRATOS_ERROR << "It should not come here resizing is not allowed this way!!!!!!!! ... " << std::endl;
            }
        }

        ConstructMasterSlaveConstraintsStructure(rModelPart);

        KRATOS_CATCH("")
    }

    //**************************************************************************
    //**************************************************************************

    void CalculateReactions(
        typename TSchemeType::Pointer pScheme,
        ModelPart& rModelPart,
        TSystemMatrixType& rA,
        TSystemVectorType& rDx,
        TSystemVectorType& rb
        ) override
    {
        TSparseSpace::SetToZero(rb);

        // Refresh RHS to have the correct reactions
        BuildRHS(pScheme, rModelPart, rb);

        // Initialize the Epetra importer
        // TODO: this part of the code has been pasted until a better solution
        // is found
        int system_size = TSparseSpace::Size(rb);
        int number_of_dofs = BaseType::mDofSet.size();
        std::vector<int> index_array(number_of_dofs);

        // Filling the array with the global ids
        int counter = 0;
        int id = 0;
        for (const auto& dof : BaseType::mDofSet) {
            id = dof.EquationId();
            if (id < system_size) {
                index_array[counter++] = id;
            }
        }

        std::sort(index_array.begin(), index_array.end());
        std::vector<int>::iterator NewEnd = std::unique(index_array.begin(), index_array.end());
        index_array.resize(NewEnd - index_array.begin());

        int check_size = -1;
        int tot_update_dofs = index_array.size();
        rb.Comm().SumAll(&tot_update_dofs, &check_size, 1);
        KRATOS_ERROR_IF(check_size < system_size)
            << "Dof count is not correct. There are less dofs than expected.\n"
            << "Expected number of active dofs = " << system_size
            << " dofs found = " << check_size << std::endl;

        // Defining a map as needed
        Epetra_Map dof_update_map(-1, index_array.size(),
                                  &(*(index_array.begin())), 0, rb.Comm());

        // Defining the importer class
        Kratos::shared_ptr<Epetra_Import> pDofImporter = Kratos::make_shared<Epetra_Import>(dof_update_map, rb.Map());

        // Defining a temporary vector to gather all of the values needed
        Epetra_Vector temp_RHS(pDofImporter->TargetMap());

        // Importing in the new temp_RHS vector the values
        int ierr = temp_RHS.Import(rb, *pDofImporter, Insert);
        KRATOS_ERROR_IF(ierr != 0) << "Epetra failure found - error code: " << ierr << std::endl;

        double* temp_RHS_values; // DO NOT make delete of this one!!
        temp_RHS.ExtractView(&temp_RHS_values);

        rb.Comm().Barrier();

        const int ndofs = static_cast<int>(BaseType::mDofSet.size());

        // Store the RHS values in the reaction variable
        // NOTE: dofs are assumed to be numbered consecutively in the
        // BlockBuilderAndSolver
        for (int k = 0; k < ndofs; k++) {
            auto dof_iterator = BaseType::mDofSet.begin() + k;

            const int i = (dof_iterator)->EquationId();
            // (dof_iterator)->GetSolutionStepReactionValue() = -(*b[i]);
            const double react_val = temp_RHS[pDofImporter->TargetMap().LID(i)];
            (dof_iterator->GetSolutionStepReactionValue()) = -react_val;
        }
    }

    /**
     * @brief Applies the dirichlet conditions. This operation may be very heavy
     * or completely unexpensive depending on the implementation choosen and on
     * how the System Matrix is built.
     * @details For explanation of how it works for a particular implementation
     * the user should refer to the particular Builder And Solver choosen
     * @param pScheme The integration scheme considered
     * @param rModelPart The model part of the problem to solve
     * @param A The LHS matrix
     * @param Dx The Unknowns vector
     * @param b The RHS vector
     */
    void ApplyDirichletConditions(
        typename TSchemeType::Pointer pScheme,
        ModelPart& rModelPart,
        TSystemMatrixType& rA,
        TSystemVectorType& rDx,
        TSystemVectorType& rb
        ) override
    {
        KRATOS_TRY

        // loop over all dofs to find the fixed ones
        std::vector<int> global_ids(BaseType::mDofSet.size());
        std::vector<int> is_dirichlet(BaseType::mDofSet.size());

        IndexType i = 0;
        for (const auto& dof : BaseType::mDofSet) {
            const int global_id = dof.EquationId();
            global_ids[i] = global_id;
            is_dirichlet[i] = dof.IsFixed();
            ++i;
        }

        // here we construct and fill a vector "fixed local" which cont
        Epetra_Map localmap(-1, global_ids.size(), global_ids.data(), 0, rA.Comm());
        Epetra_IntVector fixed_local(Copy, localmap, is_dirichlet.data());

        Epetra_Import dirichlet_importer(rA.ColMap(), fixed_local.Map());

        // defining a temporary vector to gather all of the values needed
        Epetra_IntVector fixed(rA.ColMap());

        // Detect if there is a line of all zeros and set the diagonal to a 1 if this happens
        const auto& r_process_info = rModelPart.GetProcessInfo();
        const auto& r_data_comm = rModelPart.GetCommunicator().GetDataCommunicator();
        mScaleFactor = TSparseSpace::CheckAndCorrectZeroDiagonalValues(r_process_info, rA, rb, r_data_comm, mScalingDiagonal);

        // Importing in the new temp vector the values
        int ierr = fixed.Import(fixed_local, dirichlet_importer, Insert);
        KRATOS_ERROR_IF(ierr != 0) << "Epetra failure found" << std::endl;

        for (int i = 0; i < rA.NumMyRows(); i++) {
            int numEntries; // Number of non-zero entries
            double* vals;   // Row non-zero values
            int* cols;      // Column indices of row non-zero values
            rA.ExtractMyRowView(i, numEntries, vals, cols);

            const int row_gid = rA.RowMap().GID(i);
            const int row_lid = localmap.LID(row_gid);

            if (fixed_local[row_lid] == 0) { // Not a dirichlet row
                for (int j = 0; j < numEntries; j++) {
                    if (fixed[cols[j]] == true)
                        vals[j] = 0.0;
                }
            } else { // This IS a dirichlet row
                // Set to zero the rhs
                rb[0][i] = 0.0; // note that the index of i is expected to be
                                // coherent with the rows of A

                // Set to zero the whole row
                for (int j = 0; j < numEntries; j++) {
                    int col_gid = rA.ColMap().GID(cols[j]);
                    if (col_gid != row_gid)
                        vals[j] = 0.0;
                }
            }
        }

        KRATOS_CATCH("");
    }

    /**
     * @brief Applies the constraints with master-slave relation matrix (RHS only)
     * @param pScheme The integration scheme considered
     * @param rModelPart The model part of the problem to solve
     * @param rb The RHS vector
     */
    void ApplyRHSConstraints(
        typename TSchemeType::Pointer pScheme,
        ModelPart& rModelPart,
        TSystemVectorType& rb
        ) override
    {
        KRATOS_TRY

        if (rModelPart.MasterSlaveConstraints().size() != 0) {
            BuildMasterSlaveConstraints(rModelPart);

            // // We compute the transposed matrix of the global relation matrix
            // TSystemMatrixType T_transpose_matrix(mpT.size2(), mpT.size1());
            // SparseMatrixMultiplicationUtility::TransposeMatrix<TSystemMatrixType, TSystemMatrixType>(T_transpose_matrix, mpT, 1.0);

            // TSystemVectorType b_modified(rb.size());
            // TSparseSpace::Mult(T_transpose_matrix, rb, b_modified);
            // TSparseSpace::Copy(b_modified, rb);

            // // Apply diagonal values on slaves
            // IndexPartition<std::size_t>(mSlaveIds.size()).for_each([&](std::size_t Index){
            //     const IndexType slave_equation_id = mSlaveIds[Index];
            //     if (mInactiveSlaveDofs.find(slave_equation_id) == mInactiveSlaveDofs.end()) {
            //         rb[slave_equation_id] = 0.0;
            //     }
            // });
        }

        KRATOS_CATCH("")
    }

    /**
     * @brief Applies the constraints with master-slave relation matrix
     * @param pScheme The integration scheme considered
     * @param rModelPart The model part of the problem to solve
     * @param rA The LHS matrix
     * @param rb The RHS vector
     */
    void ApplyConstraints(
        typename TSchemeType::Pointer pScheme,
        ModelPart& rModelPart,
        TSystemMatrixType& rA,
        TSystemVectorType& rb
        ) override
    {
        KRATOS_TRY

        if (rModelPart.MasterSlaveConstraints().size() != 0) {
            BuildMasterSlaveConstraints(rModelPart);

            // Reference to T
            const TSystemMatrixType& r_T = *mpT;

            // Compute T' A T
            const TSystemMatrixType copy_A(rA);
            TSparseSpace::BtDBProductOperation(rA, copy_A, r_T);

            // Compute T b
            const TSystemVectorType copy_b(rb);
            TSparseSpace::Mult(r_T, copy_b, rb);

            // Apply diagonal values on slaves
            IndexPartition<std::size_t>(mSlaveIds.size()).for_each([&](std::size_t Index){
                const IndexType local_slave_equation_id = mSlaveIds[Index]; /// TODO: I am assuming these are local dofs ids, maybe I change it later, please check it!
                if (mInactiveSlaveDofs.find(local_slave_equation_id) == mInactiveSlaveDofs.end()) {
                    int numEntries; // Number of non-zero entries
                    double* vals;   // Row non-zero values
                    int* cols;      // Column indices of row non-zero values
                    rA.ExtractMyRowView(local_slave_equation_id, numEntries, vals, cols);
                    const int row_gid = rA.RowMap().GID(local_slave_equation_id);
                    int j;
                    for (j = 0; j < numEntries; j++) {
                        const int col_gid = rA.ColMap().GID(cols[j]);
                        // Set diagonal value
                        if (col_gid == row_gid) {
                            vals[j] = mScaleFactor; /// NOTE: We may consider the maximum value of the diagonal instead
                            rb[0][local_slave_equation_id] = 0.0;
                        }
                    }
                }
            });
        }

        KRATOS_CATCH("")
    }

    /**
     * @brief This function is intended to be called at the end of the solution step to clean up memory storage not needed
     */
    void Clear() override
    {
        BaseType::Clear();

        mSlaveIds.clear();
        mMasterIds.clear();
        mInactiveSlaveDofs.clear();
        TSparseSpace::Clear(mpT);
        TSparseSpace::Clear(mpConstantVector);
    }

    /**
     * @brief This function is designed to be called once to perform all the checks needed
     * on the input provided. Checks can be "expensive" as the function is designed
     * to catch user's errors.
     * @param rModelPart The model part of the problem to solve
     * @return 0 all ok
     */
    int Check(ModelPart& rModelPart) override
    {
        KRATOS_TRY

        return 0;
        KRATOS_CATCH("");
    }

    /**
     * @brief This method provides the defaults parameters to avoid conflicts between the different constructors
     * @return The default parameters
     */
    Parameters GetDefaultParameters() const override
    {
        Parameters default_parameters = Parameters(R"(
        {
            "name"                                 : "trilinos_block_builder_and_solver",
            "guess_row_size"                       : 45,
            "block_builder"                        : true,
            "diagonal_values_for_dirichlet_dofs"   : "use_max_diagonal",
            "silent_warnings"                      : false
        })");

        // Getting base class default parameters
        const Parameters base_default_parameters = BaseType::GetDefaultParameters();
        default_parameters.RecursivelyAddMissingParameters(base_default_parameters);
        return default_parameters;
    }

    /**
     * @brief Returns the name of the class as used in the settings (snake_case format)
     * @return The name of the class
     */
    static std::string Name()
    {
        return "trilinos_block_builder_and_solver";
    }

    ///@}
    ///@name Access
    ///@{

    ///@}
    ///@name Inquiry
    ///@{

    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    std::string Info() const override
    {
        return "TrilinosBlockBuilderAndSolver";
    }

    /// Print information about this object.
    void PrintInfo(std::ostream& rOStream) const override
    {
        rOStream << Info();
    }

    /// Print object's data.
    void PrintData(std::ostream& rOStream) const override
    {
        rOStream << Info();
    }

    ///@}
    ///@name Friends
    ///@{

    ///@}
protected:
    ///@name Protected static Member Variables
    ///@{

    ///@}
    ///@name Protected member Variables
    ///@{

    /* Base variables */
    EpetraCommunicatorType& mrComm; /// The MPI communicator
    int mGuessRowSize;              /// The guess row size
    IndexType mLocalSystemSize;     /// The local system size
    int mFirstMyId;                 /// Auxiliary Id (I)
    int mLastMyId;                  /// Auxiliary Id (II)

    /* MPC variables */
    TSystemMatrixPointerType mpT =  nullptr;              /// This is matrix containing the global relation for the constraints
    TSystemVectorPointerType mpConstantVector =  nullptr; /// This is vector containing the rigid movement of the constraint
    std::vector<IndexType> mSlaveIds;                     /// The equation ids of the slaves
    std::vector<IndexType> mMasterIds;                    /// The equation ids of the master
    std::unordered_set<IndexType> mInactiveSlaveDofs;     /// The set containing the inactive slave dofs
    double mScaleFactor = 1.0;                            /// The manually set scale factor

    /* Flags */
    SCALING_DIAGONAL mScalingDiagonal = SCALING_DIAGONAL::NO_SCALING;; /// We identify the scaling considered for the dirichlet dofs
    Flags mOptions;                                                    /// Some flags used internally

    ///@}
    ///@name Protected Operators
    ///@{

    ///@}
    ///@name Protected Operations
    ///@{

    virtual void ConstructMasterSlaveConstraintsStructure(ModelPart& rModelPart)
    {
        if (rModelPart.MasterSlaveConstraints().size() > 0) {
            START_TIMER("ConstraintsRelationMatrixStructure", 0)
    //         const ProcessInfo& r_current_process_info = rModelPart.GetProcessInfo();

    //         // Vector containing the localization in the system of the different terms
    //         DofsVectorType slave_dof_list, master_dof_list;

    //         // Constraint initial iterator
    //         const auto it_const_begin = rModelPart.MasterSlaveConstraints().begin();
    //         std::vector<std::unordered_set<IndexType>> indices(BaseType::mDofSet.size());

    //         std::vector<LockObject> lock_array(indices.size());

    //         #pragma omp parallel firstprivate(slave_dof_list, master_dof_list)
    //         {
    //             Element::EquationIdVectorType slave_ids(3);
    //             Element::EquationIdVectorType master_ids(3);
    //             std::unordered_map<IndexType, std::unordered_set<IndexType>> temp_indices;

    //             #pragma omp for schedule(guided, 512) nowait
    //             for (int i_const = 0; i_const < static_cast<int>(rModelPart.MasterSlaveConstraints().size()); ++i_const) {
    //                 auto it_const = it_const_begin + i_const;

    //                 // Detect if the constraint is active or not. If the user did not make any choice the constraint
    //                 // It is active by default
    //                 bool constraint_is_active = true;
    //                 if( it_const->IsDefined(ACTIVE) ) {
    //                     constraint_is_active = it_const->Is(ACTIVE);
    //                 }

    //                 if(constraint_is_active) {
    //                     it_const->EquationIdVector(slave_ids, master_ids, r_current_process_info);

    //                     // Slave DoFs
    //                     for (auto &id_i : slave_ids) {
    //                         temp_indices[id_i].insert(master_ids.begin(), master_ids.end());
    //                     }
    //                 }
    //             }

    //             // Merging all the temporal indexes
    //             for (int i = 0; i < static_cast<int>(temp_indices.size()); ++i) {
    //                 lock_array[i].lock();
    //                 indices[i].insert(temp_indices[i].begin(), temp_indices[i].end());
    //                 lock_array[i].unlock();
    //             }
    //         }

    //         mSlaveIds.clear();
    //         mMasterIds.clear();
    //         for (int i = 0; i < static_cast<int>(indices.size()); ++i) {
    //             if (indices[i].size() == 0) // Master dof!
    //                 mMasterIds.push_back(i);
    //             else // Slave dof
    //                 mSlaveIds.push_back(i);
    //             indices[i].insert(i); // Ensure that the diagonal is there in T
    //         }

    //         // Count the row sizes
    //         std::size_t nnz = 0;
    //         for (IndexType i = 0; i < indices.size(); ++i)
    //             nnz += indices[i].size();

    //         mpT = TSystemMatrixType(indices.size(), indices.size(), nnz);
    //         mpConstantVector.resize(indices.size(), false);

    //         double *Tvalues = mpT.value_data().begin();
    //         IndexType *Trow_indices = mpT.index1_data().begin();
    //         IndexType *Tcol_indices = mpT.index2_data().begin();

    //         // Filling the index1 vector - DO NOT MAKE PARALLEL THE FOLLOWING LOOP!
    //         Trow_indices[0] = 0;
    //         for (int i = 0; i < static_cast<int>(mpT.size1()); i++)
    //             Trow_indices[i + 1] = Trow_indices[i] + indices[i].size();

    //         IndexPartition<std::size_t>(mpT.size1()).for_each([&](std::size_t Index){
    //             const IndexType row_begin = Trow_indices[Index];
    //             const IndexType row_end = Trow_indices[Index + 1];
    //             IndexType k = row_begin;
    //             for (auto it = indices[Index].begin(); it != indices[Index].end(); ++it) {
    //                 Tcol_indices[k] = *it;
    //                 Tvalues[k] = 0.0;
    //                 k++;
    //             }

    //             indices[Index].clear(); //deallocating the memory

    //             std::sort(&Tcol_indices[row_begin], &Tcol_indices[row_end]);
    //         });

    //         mpT.set_filled(indices.size() + 1, nnz);

            STOP_TIMER("ConstraintsRelationMatrixStructure", 0)
        }
    }

    virtual void BuildMasterSlaveConstraints(ModelPart& rModelPart)
    {
        KRATOS_TRY

    //     TSparseSpace::SetToZero(mpT);
    //     TSparseSpace::SetToZero(mpConstantVector);

    //     // The current process info
    //     const ProcessInfo& r_current_process_info = rModelPart.GetProcessInfo();

    //     // Vector containing the localization in the system of the different terms
    //     DofsVectorType slave_dof_list, master_dof_list;

    //     // Contributions to the system
    //     Matrix transformation_matrix = LocalSystemMatrixType(0, 0);
    //     Vector constant_vector = LocalSystemVectorType(0);

    //     // Vector containing the localization in the system of the different terms
    //     Element::EquationIdVectorType slave_equation_ids, master_equation_ids;

    //     const int number_of_constraints = static_cast<int>(rModelPart.MasterSlaveConstraints().size());

    //     // We clear the set
    //     mInactiveSlaveDofs.clear();

    //     #pragma omp parallel firstprivate(transformation_matrix, constant_vector, slave_equation_ids, master_equation_ids)
    //     {
    //         std::unordered_set<IndexType> auxiliar_inactive_slave_dofs;

    //         #pragma omp for schedule(guided, 512)
    //         for (int i_const = 0; i_const < number_of_constraints; ++i_const) {
    //             auto it_const = rModelPart.MasterSlaveConstraints().begin() + i_const;

    //             // Detect if the constraint is active or not. If the user did not make any choice the constraint
    //             // It is active by default
    //             bool constraint_is_active = true;
    //             if (it_const->IsDefined(ACTIVE))
    //                 constraint_is_active = it_const->Is(ACTIVE);

    //             if (constraint_is_active) {
    //                 it_const->CalculateLocalSystem(transformation_matrix, constant_vector, r_current_process_info);
    //                 it_const->EquationIdVector(slave_equation_ids, master_equation_ids, r_current_process_info);

    //                 for (IndexType i = 0; i < slave_equation_ids.size(); ++i) {
    //                     const IndexType i_global = slave_equation_ids[i];

    //                     // Assemble matrix row
    //                     AssembleRowContribution(mpT, transformation_matrix, i_global, i, master_equation_ids);

    //                     // Assemble constant vector
    //                     const double constant_value = constant_vector[i];
    //                     double& r_value = mpConstantVector[i_global];
    //                     AtomicAdd(r_value, constant_value);
    //                 }
    //             } else { // Taking into account inactive constraints
    //                 it_const->EquationIdVector(slave_equation_ids, master_equation_ids, r_current_process_info);
    //                 auxiliar_inactive_slave_dofs.insert(slave_equation_ids.begin(), slave_equation_ids.end());
    //             }
    //         }

    //         // We merge all the sets in one thread
    //         #pragma omp critical
    //         {
    //             mInactiveSlaveDofs.insert(auxiliar_inactive_slave_dofs.begin(), auxiliar_inactive_slave_dofs.end());
    //         }
    //     }

    //     // Setting the master dofs into the T and C system
    //     for (auto eq_id : mMasterIds) {
    //         mpConstantVector[eq_id] = 0.0;
    //         mpT(eq_id, eq_id) = 1.0;
    //     }

    //     // Setting inactive slave dofs in the T and C system
    //     for (auto eq_id : mInactiveSlaveDofs) {
    //         mpConstantVector[eq_id] = 0.0;
    //         mpT(eq_id, eq_id) = 1.0;
    //     }

        KRATOS_CATCH("")
    }

    virtual void ConstructMatrixStructure(
        typename TSchemeType::Pointer pScheme,
        TSystemMatrixType& A,
        ModelPart& rModelPart)
    {
        // Filling with zero the matrix (creating the structure)
        START_TIMER("MatrixStructure", 0)

    //     const ProcessInfo& CurrentProcessInfo = rModelPart.GetProcessInfo();

    //     const std::size_t equation_size = BaseType::mEquationSystemSize;

    //     std::vector< LockObject > lock_array(equation_size);

    //     std::vector<std::unordered_set<std::size_t> > indices(equation_size);

    //     block_for_each(indices, [](std::unordered_set<std::size_t>& rIndices){
    //         rIndices.reserve(40);
    //     });

    //     Element::EquationIdVectorType ids(3, 0);

    //     block_for_each(rModelPart.Elements(), ids, [&](Element& rElem, Element::EquationIdVectorType& rIdsTLS){
    //         pScheme->EquationId(rElem, rIdsTLS, CurrentProcessInfo);
    //         for (std::size_t i = 0; i < rIdsTLS.size(); i++) {
    //             lock_array[rIdsTLS[i]].lock();
    //             auto& row_indices = indices[rIdsTLS[i]];
    //             row_indices.insert(rIdsTLS.begin(), rIdsTLS.end());
    //             lock_array[rIdsTLS[i]].unlock();
    //         }
    //     });

    //     block_for_each(rModelPart.Conditions(), ids, [&](Condition& rCond, Element::EquationIdVectorType& rIdsTLS){
    //         pScheme->EquationId(rCond, rIdsTLS, CurrentProcessInfo);
    //         for (std::size_t i = 0; i < rIdsTLS.size(); i++) {
    //             lock_array[rIdsTLS[i]].lock();
    //             auto& row_indices = indices[rIdsTLS[i]];
    //             row_indices.insert(rIdsTLS.begin(), rIdsTLS.end());
    //             lock_array[rIdsTLS[i]].unlock();
    //         }
    //     });

    //     if (rModelPart.MasterSlaveConstraints().size() != 0) {
    //         struct TLS
    //         {
    //             Element::EquationIdVectorType master_ids = Element::EquationIdVectorType(3,0);
    //             Element::EquationIdVectorType slave_ids = Element::EquationIdVectorType(3,0);
    //         };
    //         TLS tls;

    //         block_for_each(rModelPart.MasterSlaveConstraints(), tls, [&](MasterSlaveConstraint& rConst, TLS& rTls){
    //             rConst.EquationIdVector(rTls.slave_ids, rTls.master_ids, CurrentProcessInfo);

    //             for (std::size_t i = 0; i < rTls.slave_ids.size(); i++) {
    //                 lock_array[rTls.slave_ids[i]].lock();
    //                 auto& row_indices = indices[rTls.slave_ids[i]];
    //                 row_indices.insert(rTls.slave_ids[i]);
    //                 lock_array[rTls.slave_ids[i]].unlock();
    //             }

    //             for (std::size_t i = 0; i < rTls.master_ids.size(); i++) {
    //                 lock_array[rTls.master_ids[i]].lock();
    //                 auto& row_indices = indices[rTls.master_ids[i]];
    //                 row_indices.insert(rTls.master_ids[i]);
    //                 lock_array[rTls.master_ids[i]].unlock();
    //             }
    //         });

    //     }

    //     //destroy locks
    //     lock_array = std::vector< LockObject >();

    //     //count the row sizes
    //     unsigned int nnz = 0;
    //     for (unsigned int i = 0; i < indices.size(); i++) {
    //         nnz += indices[i].size();
    //     }

    //     A = CompressedMatrixType(indices.size(), indices.size(), nnz);

    //     double* Avalues = A.value_data().begin();
    //     std::size_t* Arow_indices = A.index1_data().begin();
    //     std::size_t* Acol_indices = A.index2_data().begin();

    //     //filling the index1 vector - DO NOT MAKE PARALLEL THE FOLLOWING LOOP!
    //     Arow_indices[0] = 0;
    //     for (int i = 0; i < static_cast<int>(A.size1()); i++) {
    //         Arow_indices[i+1] = Arow_indices[i] + indices[i].size();
    //     }

    //     IndexPartition<std::size_t>(A.size1()).for_each([&](std::size_t i){
    //         const unsigned int row_begin = Arow_indices[i];
    //         const unsigned int row_end = Arow_indices[i+1];
    //         unsigned int k = row_begin;
    //         for (auto it = indices[i].begin(); it != indices[i].end(); it++) {
    //             Acol_indices[k] = *it;
    //             Avalues[k] = 0.0;
    //             k++;
    //         }

    //         indices[i].clear(); //deallocating the memory

    //         std::sort(&Acol_indices[row_begin], &Acol_indices[row_end]);

    //     });

    //     A.set_filled(indices.size()+1, nnz);

        STOP_TIMER("MatrixStructure", 0)
    }

    /**
     * @brief This method assigns settings to member variables
     * @param ThisParameters Parameters that are assigned to the member variables
     */
    void AssignSettings(const Parameters ThisParameters) override
    {
        BaseType::AssignSettings(ThisParameters);

        // Get guess row size
        mGuessRowSize = ThisParameters["guess_row_size"].GetInt();

        // Setting flags
        const std::string& r_diagonal_values_for_dirichlet_dofs = ThisParameters["diagonal_values_for_dirichlet_dofs"].GetString();

        const std::set<std::string> available_options_for_diagonal = {"no_scaling","use_max_diagonal","use_diagonal_norm","defined_in_process_info"};

        if (available_options_for_diagonal.find(r_diagonal_values_for_dirichlet_dofs) == available_options_for_diagonal.end()) {
            std::stringstream msg;
            msg << "Currently prescribed diagonal values for dirichlet dofs : " << r_diagonal_values_for_dirichlet_dofs << "\n";
            msg << "Admissible values for the diagonal scaling are : no_scaling, use_max_diagonal, use_diagonal_norm, or defined_in_process_info" << "\n";
            KRATOS_ERROR << msg.str() << std::endl;
        }

        // The first option will not consider any scaling (the diagonal values will be replaced with 1)
        if (r_diagonal_values_for_dirichlet_dofs == "no_scaling") {
            mScalingDiagonal = SCALING_DIAGONAL::NO_SCALING;
        } else if (r_diagonal_values_for_dirichlet_dofs == "use_max_diagonal") {
            mScalingDiagonal = SCALING_DIAGONAL::CONSIDER_MAX_DIAGONAL;
        } else if (r_diagonal_values_for_dirichlet_dofs == "use_diagonal_norm") { // On this case the norm of the diagonal will be considered
            mScalingDiagonal = SCALING_DIAGONAL::CONSIDER_NORM_DIAGONAL;
        } else { // Otherwise we will assume we impose a numerical value
            mScalingDiagonal = SCALING_DIAGONAL::CONSIDER_PRESCRIBED_DIAGONAL;
        }
        mOptions.Set(SILENT_WARNINGS, ThisParameters["silent_warnings"].GetBool());
    }

    ///@}
    ///@name Protected  Access
    ///@{

    ///@}
    ///@name Protected Inquiry
    ///@{

    ///@}
    ///@name Protected LifeCycle
    ///@{

    ///@}
private:
    ///@name Static Member Variables
    ///@{

    ///@}
    ///@name Member Variables
    ///@{

    ///@}
    ///@name Private Operators
    ///@{

    ///@}
    ///@name Private Operations
    ///@{

    void AssembleLHS_CompleteOnFreeRows(TSystemMatrixType& rA,
                                        LocalSystemMatrixType& rLHS_Contribution,
                                        Element::EquationIdVectorType& rEquationId)
    {
        KRATOS_ERROR << "This method is not implemented for Trilinos";
    }

    ///@}
    ///@name Private  Access
    ///@{

    ///@}
    ///@name Private Inquiry
    ///@{

    ///@}
    ///@name Un accessible methods
    ///@{

    ///@}
}; /* Class TrilinosBlockBuilderAndSolver */

///@}

///@name Type Definitions
///@{

// Here one should use the KRATOS_CREATE_LOCAL_FLAG, but it does not play nice with template parameters
template<class TSparseSpace, class TDenseSpace, class TLinearSolver>
const Kratos::Flags TrilinosBlockBuilderAndSolver<TSparseSpace, TDenseSpace, TLinearSolver>::SILENT_WARNINGS(Kratos::Flags::Create(0));

///@}

} /* namespace Kratos.*/