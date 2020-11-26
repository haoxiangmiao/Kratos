//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Jordi Cotela
//                   Ruben Zorrilla
//
//

#ifndef KRATOS_ESTIMATE_DT_UTILITIES_H
#define	KRATOS_ESTIMATE_DT_UTILITIES_H

// System includes


// External includes


// Project includes
#include "includes/define.h"
#include "includes/model_part.h"
#include "includes/kratos_parameters.h"
#include "includes/serializer.h"

// Application includes


namespace Kratos
{

///@addtogroup FluidDynamicsApplication
///@{

///@name Kratos Classes
///@{

/// Estimate the time step in a fluid problem to obtain a given Courant number.
template< unsigned int TDim >
class EstimateDtUtility
{
public:

    ///@name Life Cycle
    ///@{

    /// Constructor
    /**
     * @param ModelPart The model part containing the problem mesh
     * @param CFL The user-defined Courant-Friedrichs-Lewy number
     * @param DtMin user-defined minimum time increment allowed
     * @param DtMax user-defined maximum time increment allowed
     */
    EstimateDtUtility(
        ModelPart &ModelPart,
        const double CFL,
        const double DtMin,
        const double DtMax)
        : mrModelPart(ModelPart)
    {
        mCFL = CFL;
        mDtMin = DtMin;
        mDtMax = DtMax;
    }

    /// Constructor with Kratos parameters
    /**
     * @param ModelPart The model part containing the problem mesh
     * @param rParameters Kratos parameters containing the CFL number and max time step
     */
    EstimateDtUtility(
        ModelPart& ModelPart,
        Parameters& rParameters)
        : mrModelPart(ModelPart)
    {
        Parameters defaultParameters(R"({
            "automatic_time_step"   : true,
            "CFL_number"            : 1.0,
            "minimum_delta_time"    : 1e-4,
            "maximum_delta_time"    : 0.1
        })");

        rParameters.ValidateAndAssignDefaults(defaultParameters);

        mCFL = rParameters["CFL_number"].GetDouble();
        mDtMin = rParameters["minimum_delta_time"].GetDouble();
        mDtMax = rParameters["maximum_delta_time"].GetDouble();
    }

    /// Destructor
    ~EstimateDtUtility()
    {}

    ///@}
    ///@name Operations
    ///@{

    /// Set the CFL value.
    /**
    * @param CFL the user-defined CFL number used in the automatic time step computation
    */
    void SetCFL(const double CFL);

    /// Set the maximum time step allowed value.
    /**
    * @param CFL the user-defined CFL number used in the automatic time step computation
    */
    void SetDtMin(const double DtMin);

    /// Set the maximum time step allowed value.
    /**
    * @param CFL the user-defined CFL number used in the automatic time step computation
    */
    void SetDtMax(const double DtMax);

    /// Calculate the maximum time step that satisfies the Courant-Friedrichs-Lewy (CFL) condition.
    /**
     * @return A time step value that satisfies the CFL condition for the current mesh and velocity field
     */
    double EstimateDt() const;

    /// Calculate each element's CFL for the current time step for the given ModelPart.
    /**
     * The elemental CFL is stored in the CFL_NUMBER elemental variable.
     * To view it in the post-process file, remember to print CFL_NUMBER as a Gauss Point result.
     */
    static void CalculateLocalCFL(ModelPart& rModelPart);

    /// Calculate each element's CFL for the current time step.
    /**
     * The elemental CFL is stored in the CFL_NUMBER elemental variable.
     * To view it in the post-process file, remember to print CFL_NUMBER as a Gauss Point result.
     */
    void CalculateLocalCFL();

    ///@} // Operators

private:

    ///@name Auxiliary Data types
    ///@{

    struct GeometryDataContainer {
        double Area;
        array_1d<double, TDim+1> N;
        BoundedMatrix<double, TDim+1, TDim> DN_DX;
    };

    ///@}
    ///@name Member Variables
    ///@{

    double    mCFL;         // User-defined CFL number
    double    mDtMax;       // User-defined maximum time increment allowed
    double    mDtMin;       // User-defined minimum time increment allowed
    ModelPart &mrModelPart; // The problem's model part

    ///@}
    ///@name Private Operations
    ///@{

    static double CalculateElementCFL(
        Element &rElement,
        GeometryDataContainer& rGeometryInfo,
        double Dt);

    ///@} // Private Operations
};

///@} // Kratos classes

///@} // FluidDynamicsApplication group

} // namespace Kratos.


#endif	/* KRATOS_ESTIMATE_DT_UTILITIES_H */
