//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    , KratosAppGenerator
//

// System includes

// External includes

// Include Base h
#include "rans_evm_k_adjoint.h"

#include "custom_elements/evm_k_epsilon/evm_k_epsilon_adjoint_utilities.h"
#include "custom_elements/evm_k_epsilon/evm_k_epsilon_utilities.h"
#include "custom_utilities/rans_variable_utils.h"
#include "includes/cfd_variables.h"
#include "rans_modelling_application_variables.h"

namespace Kratos
{
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
 * Constructor.
 */
template <unsigned int TDim, unsigned int TNumNodes>
RansEvmKAdjoint<TDim, TNumNodes>::RansEvmKAdjoint(IndexType NewId)
    : BaseType(NewId)
{
}

/**
 * Constructor using an array of nodes
 */
template <unsigned int TDim, unsigned int TNumNodes>
RansEvmKAdjoint<TDim, TNumNodes>::RansEvmKAdjoint(IndexType NewId, const NodesArrayType& ThisNodes)
    : BaseType(NewId, ThisNodes)
{
}

/**
 * Constructor using Geometry
 */
template <unsigned int TDim, unsigned int TNumNodes>
RansEvmKAdjoint<TDim, TNumNodes>::RansEvmKAdjoint(IndexType NewId, GeometryType::Pointer pGeometry)
    : BaseType(NewId, pGeometry)
{
}

/**
 * Constructor using Properties
 */
template <unsigned int TDim, unsigned int TNumNodes>
RansEvmKAdjoint<TDim, TNumNodes>::RansEvmKAdjoint(IndexType NewId,
                                                  GeometryType::Pointer pGeometry,
                                                  PropertiesType::Pointer pProperties)
    : BaseType(NewId, pGeometry, pProperties)
{
}

/**
 * Copy Constructor
 */
template <unsigned int TDim, unsigned int TNumNodes>
RansEvmKAdjoint<TDim, TNumNodes>::RansEvmKAdjoint(RansEvmKAdjoint<TDim, TNumNodes> const& rOther)
    : BaseType(rOther)
{
}

/**
 * Destructor
 */
template <unsigned int TDim, unsigned int TNumNodes>
RansEvmKAdjoint<TDim, TNumNodes>::~RansEvmKAdjoint()
{
}

///@}
///@name Operators
///@{

/// Assignment operator.
template <unsigned int TDim, unsigned int TNumNodes>
RansEvmKAdjoint<TDim, TNumNodes>& RansEvmKAdjoint<TDim, TNumNodes>::operator=(
    RansEvmKAdjoint<TDim, TNumNodes> const& rOther)
{
    BaseType::operator=(rOther);
    Flags::operator=(rOther);
    // mpProperties = rOther.mpProperties;
    return *this;
}

///@}
///@name Operations
///@{

/**
 * ELEMENTS inherited from this class have to implement next
 * Create and Clone methods: MANDATORY
 */

/**
 * creates a new element pointer
 * @param NewId: the ID of the new element
 * @param ThisNodes: the nodes of the new element
 * @param pProperties: the properties assigned to the new element
 * @return a Pointer to the new element
 */
template <unsigned int TDim, unsigned int TNumNodes>
Element::Pointer RansEvmKAdjoint<TDim, TNumNodes>::Create(IndexType NewId,
                                                          NodesArrayType const& ThisNodes,
                                                          PropertiesType::Pointer pProperties) const
{
    KRATOS_TRY
    return Kratos::make_intrusive<RansEvmKAdjoint>(
        NewId, Element::GetGeometry().Create(ThisNodes), pProperties);
    KRATOS_CATCH("");
}

/**
 * creates a new element pointer
 * @param NewId: the ID of the new element
 * @param pGeom: the geometry to be employed
 * @param pProperties: the properties assigned to the new element
 * @return a Pointer to the new element
 */
template <unsigned int TDim, unsigned int TNumNodes>
Element::Pointer RansEvmKAdjoint<TDim, TNumNodes>::Create(IndexType NewId,
                                                          GeometryType::Pointer pGeom,
                                                          PropertiesType::Pointer pProperties) const
{
    KRATOS_TRY
    return Kratos::make_intrusive<RansEvmKAdjoint>(NewId, pGeom, pProperties);
    KRATOS_CATCH("");
}

/**
 * creates a new element pointer and clones the previous element data
 * @param NewId: the ID of the new element
 * @param ThisNodes: the nodes of the new element
 * @param pProperties: the properties assigned to the new element
 * @return a Pointer to the new element
 */
template <unsigned int TDim, unsigned int TNumNodes>
Element::Pointer RansEvmKAdjoint<TDim, TNumNodes>::Clone(IndexType NewId,
                                                         NodesArrayType const& ThisNodes) const
{
    KRATOS_TRY
    return Kratos::make_intrusive<RansEvmKAdjoint>(
        NewId, Element::GetGeometry().Create(ThisNodes), Element::pGetProperties());
    KRATOS_CATCH("");
}

template <unsigned int TDim, unsigned int TNumNodes>
GeometryData::IntegrationMethod RansEvmKAdjoint<TDim, TNumNodes>::GetIntegrationMethod() const
{
    return GeometryData::GI_GAUSS_2;
}

/**
 * This method provides the place to perform checks on the completeness of the
 * input and the compatibility with the problem options as well as the
 * contitutive laws selected It is designed to be called only once (or anyway,
 * not often) typically at the beginning of the calculations, so to verify that
 * nothing is missing from the input or that no common error is found.
 * @param rCurrentProcessInfo
 * this method is: MANDATORY
 */
template <unsigned int TDim, unsigned int TNumNodes>
int RansEvmKAdjoint<TDim, TNumNodes>::Check(const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY

    BaseType::Check(rCurrentProcessInfo);

    KRATOS_CHECK_VARIABLE_KEY(TURBULENCE_RANS_C_MU);
    KRATOS_CHECK_VARIABLE_KEY(TURBULENT_ENERGY_DISSIPATION_RATE_SIGMA);
    KRATOS_CHECK_VARIABLE_KEY(TURBULENT_KINETIC_ENERGY_SIGMA);
    KRATOS_CHECK_VARIABLE_KEY(TURBULENT_VISCOSITY);
    KRATOS_CHECK_VARIABLE_KEY(TURBULENT_KINETIC_ENERGY);
    KRATOS_CHECK_VARIABLE_KEY(KINEMATIC_VISCOSITY);
    KRATOS_CHECK_VARIABLE_KEY(DISTANCE);
    KRATOS_CHECK_VARIABLE_KEY(RANS_Y_PLUS);
    KRATOS_CHECK_VARIABLE_KEY(TURBULENT_ENERGY_DISSIPATION_RATE);
    KRATOS_CHECK_VARIABLE_KEY(VELOCITY);
    KRATOS_CHECK_VARIABLE_KEY(RANS_AUXILIARY_VARIABLE_1);
    KRATOS_CHECK_VARIABLE_KEY(RANS_TURBULENT_ENERGY_DISSIPATION_RATE_PARTIAL_DERIVATIVE);
    KRATOS_CHECK_VARIABLE_KEY(RANS_Y_PLUS_VELOCITY_DERIVATIVES);
    KRATOS_CHECK_VARIABLE_KEY(RANS_SCALAR_1_ADJOINT_1);
    KRATOS_CHECK_VARIABLE_KEY(RANS_SCALAR_1_ADJOINT_3);

    KRATOS_ERROR_IF(this->Id() < 1) << "RansEvmKAdjoint"
                                       "found with Id 0 "
                                       "or negative"
                                    << std::endl;

    KRATOS_ERROR_IF(this->GetGeometry().Area() <= 0)
        << "On RansEvmKAdjoint -> " << this->Id()
        << "; Area cannot be less than or equal to 0" << std::endl;

    for (IndexType iNode = 0; iNode < this->GetGeometry().size(); ++iNode)
    {
        NodeType& r_node = this->GetGeometry()[iNode];
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(TURBULENT_VISCOSITY, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(TURBULENT_KINETIC_ENERGY, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(KINEMATIC_VISCOSITY, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(DISTANCE, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(RANS_Y_PLUS, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(TURBULENT_ENERGY_DISSIPATION_RATE, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(VELOCITY, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(RANS_AUXILIARY_VARIABLE_1, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(RANS_SCALAR_1_ADJOINT_1, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(RANS_SCALAR_1_ADJOINT_3, r_node);

        KRATOS_CHECK_DOF_IN_NODE(RANS_SCALAR_1_ADJOINT_1, r_node);
    }

    return 0;

    KRATOS_CATCH("");
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

template <unsigned int TDim, unsigned int TNumNodes>
std::string RansEvmKAdjoint<TDim, TNumNodes>::Info() const
{
    std::stringstream buffer;
    buffer << "RansEvmKAdjoint #" << Element::Id();
    return buffer.str();
}

/// Print information about this object.

template <unsigned int TDim, unsigned int TNumNodes>
void RansEvmKAdjoint<TDim, TNumNodes>::PrintInfo(std::ostream& rOStream) const
{
    rOStream << "RansEvmKAdjoint #" << Element::Id();
}

/// Print object's data.

template <unsigned int TDim, unsigned int TNumNodes>
void RansEvmKAdjoint<TDim, TNumNodes>::PrintData(std::ostream& rOStream) const
{
    Element::pGetGeometry()->PrintData(rOStream);
}

///@}
///@name Friends
///@{

///@}

///@name Protected static Member Variables
///@{

///@}
///@name Protected member Variables
///@{

///@}
///@name Protected Operators
///@{

///@}
///@name Protected Operations
///@{

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
template <unsigned int TDim, unsigned int TNumNodes>
const Variable<double>& RansEvmKAdjoint<TDim, TNumNodes>::GetPrimalVariable() const
{
    return TURBULENT_KINETIC_ENERGY;
}

template <unsigned int TDim, unsigned int TNumNodes>
const Variable<double>& RansEvmKAdjoint<TDim, TNumNodes>::GetPrimalRelaxedRateVariable() const
{
    return RANS_AUXILIARY_VARIABLE_1;
}

template <unsigned int TDim, unsigned int TNumNodes>
const Variable<double>& RansEvmKAdjoint<TDim, TNumNodes>::GetAdjointVariable() const
{
    return RANS_SCALAR_1_ADJOINT_1;
}

template <unsigned int TDim, unsigned int TNumNodes>
const Variable<double>& RansEvmKAdjoint<TDim, TNumNodes>::GetAdjointSecondVariable() const
{
    return RANS_SCALAR_1_ADJOINT_3;
}

template <unsigned int TDim, unsigned int TNumNodes>
void RansEvmKAdjoint<TDim, TNumNodes>::CalculateElementData(RansEvmKAdjointData& rData,
                                                            const Vector& rShapeFunctions,
                                                            const Matrix& rShapeFunctionDerivatives,
                                                            const ProcessInfo& rCurrentProcessInfo) const
{
    rData.ShapeFunctionDerivatives = rShapeFunctionDerivatives;
    rData.ShapeFunctions = rShapeFunctions;

    const double& c_mu = rCurrentProcessInfo[TURBULENCE_RANS_C_MU];
    const double& tke_sigma = rCurrentProcessInfo[TURBULENT_KINETIC_ENERGY_SIGMA];

    const double& nu_t = this->EvaluateInPoint(TURBULENT_VISCOSITY, rShapeFunctions);
    const double& tke = this->EvaluateInPoint(TURBULENT_KINETIC_ENERGY, rShapeFunctions);
    const double& nu = this->EvaluateInPoint(KINEMATIC_VISCOSITY, rShapeFunctions);
    const double& wall_distance = this->EvaluateInPoint(DISTANCE, rShapeFunctions);
    const double& gamma = EvmKepsilonModelUtilities::CalculateGamma(c_mu, 1.0, tke, nu_t);

    rData.TurbulentKinematicViscositySensitivitiesK.resize(TNumNodes);
    rData.TurbulentKinematicViscositySensitivitiesEpsilon.resize(TNumNodes);

    for (unsigned int i_node = 0; i_node < TNumNodes; ++i_node)
    {
        const NodeType& r_node = this->GetGeometry()[i_node];
        const Vector& turbulent_kinematic_viscosity_sensitivities =
            r_node.GetValue(RANS_NUT_SCALAR_PARTIAL_DERIVATIVES);

        KRATOS_ERROR_IF(turbulent_kinematic_viscosity_sensitivities.size() != 2) << "RANS_NUT_SCALAR_PARTIAL_DERIVATIVES variable is not specified for node "
                                                                                 << r_node
                                                                                        .Info()
                                                                                 << "\n Please use available NutKEpsilonHighReSensitivitiesProcess to calculate RANS_NUT_SCALAR_PARTIAL_DERIVATIVES.\n";

        rData.TurbulentKinematicViscositySensitivitiesK[i_node] =
            turbulent_kinematic_viscosity_sensitivities[0];
        rData.TurbulentKinematicViscositySensitivitiesEpsilon[i_node] =
            turbulent_kinematic_viscosity_sensitivities[1];
    }

    rData.TurbulentKinematicViscosity = nu_t;
    rData.TurbulentKineticEnergy = tke;
    rData.KinematicViscosity = nu;
    rData.WallDistance = wall_distance;
    rData.Gamma = gamma;
    rData.EffectiveKinematicViscosity = nu + nu_t / tke_sigma;
    rData.VelocityDivergence =
        this->GetDivergenceOperator(VELOCITY, rShapeFunctionDerivatives);

    RansVariableUtils rans_variable_utils;

    rans_variable_utils.GetNodalArray(rData.NodalTurbulentKineticEnergy, *this,
                                      TURBULENT_KINETIC_ENERGY);
    rans_variable_utils.GetNodalArray(rData.NodalTurbulentEnergyDissipationRate,
                                      *this, TURBULENT_ENERGY_DISSIPATION_RATE);
    rans_variable_utils.GetNodalArray(rData.NodalYPlus, *this, RANS_Y_PLUS);

    std::size_t number_of_nodes = rData.NodalYPlus.size();

    if (rData.NodalFmu.size() != number_of_nodes)
        rData.NodalFmu.resize(rData.NodalYPlus.size());

    if (rData.NodalVelocity.size1() != TNumNodes || rData.NodalVelocity.size2() != TDim)
        rData.NodalVelocity.resize(TNumNodes, TDim);

    for (std::size_t i_node = 0; i_node < number_of_nodes; ++i_node)
    {
        rData.NodalFmu[i_node] = 1.0;
        const array_1d<double, 3>& rVelocity =
            this->GetGeometry()[i_node].FastGetSolutionStepValue(VELOCITY);
        for (unsigned int i_dim = 0; i_dim < TDim; ++i_dim)
        {
            rData.NodalVelocity(i_node, i_dim) = rVelocity[i_dim];
        }
    }
}

template <unsigned int TDim, unsigned int TNumNodes>
double RansEvmKAdjoint<TDim, TNumNodes>::CalculateEffectiveKinematicViscosity(
    const RansEvmKAdjointData& rCurrentData, const ProcessInfo& rCurrentProcessInfo) const
{
    return rCurrentData.EffectiveKinematicViscosity;
}

template <unsigned int TDim, unsigned int TNumNodes>
double RansEvmKAdjoint<TDim, TNumNodes>::CalculateReactionTerm(
    const RansEvmKAdjointData& rData, const ProcessInfo& rCurrentProcessInfo) const
{
    return rData.Gamma + 2 * rData.VelocityDivergence / 3.0;
}

template <unsigned int TDim, unsigned int TNumNodes>
double RansEvmKAdjoint<TDim, TNumNodes>::CalculateSourceTerm(
    const RansEvmKAdjointData& rData, const ProcessInfo& rCurrentProcessInfo) const
{
    BoundedMatrix<double, TDim, TDim> velocity_gradient_matrix;
    this->CalculateGradient(velocity_gradient_matrix, VELOCITY, rData.ShapeFunctionDerivatives);

    const double tke_production = EvmKepsilonModelUtilities::CalculateSourceTerm<TDim>(
        velocity_gradient_matrix, rData.TurbulentKinematicViscosity, rData.TurbulentKineticEnergy);

    return tke_production;
}

template <unsigned int TDim, unsigned int TNumNodes>
void RansEvmKAdjoint<TDim, TNumNodes>::CalculateEffectiveKinematicViscosityScalarDerivatives(
    Vector& rOutput,
    const Variable<double>& rDerivativeVariable,
    const RansEvmKAdjointData& rCurrentData,
    const ProcessInfo& rCurrentProcessInfo) const
{
    KRATOS_TRY

    if (rDerivativeVariable == TURBULENT_KINETIC_ENERGY)
    {
        const double tke_sigma = rCurrentProcessInfo[TURBULENT_KINETIC_ENERGY_SIGMA];

        EvmKepsilonModelAdjointUtilities::CalculateGaussSensitivities(
            rOutput, rCurrentData.TurbulentKinematicViscositySensitivitiesK,
            rCurrentData.ShapeFunctions);

        noalias(rOutput) = rOutput / tke_sigma;
    }
    else if (rDerivativeVariable == TURBULENT_ENERGY_DISSIPATION_RATE)
    {
        const double tke_sigma = rCurrentProcessInfo[TURBULENT_KINETIC_ENERGY_SIGMA];

        EvmKepsilonModelAdjointUtilities::CalculateGaussSensitivities(
            rOutput, rCurrentData.TurbulentKinematicViscositySensitivitiesEpsilon,
            rCurrentData.ShapeFunctions);

        noalias(rOutput) = rOutput / tke_sigma;
    }
    else
    {
        KRATOS_ERROR << "Unsupported partial derivative variable "
                     << rDerivativeVariable.Name() << " used in RansEvmKAdjoint::CalculateEffectiveKinematicViscosityDerivatives method.";
    }

    KRATOS_CATCH("");
}

template <unsigned int TDim, unsigned int TNumNodes>
void RansEvmKAdjoint<TDim, TNumNodes>::CalculateReactionTermScalarDerivatives(
    Vector& rOutput,
    const Variable<double>& rDerivativeVariable,
    const RansEvmKAdjointData& rCurrentData,
    const ProcessInfo& rCurrentProcessInfo) const
{
    if (rDerivativeVariable == TURBULENT_KINETIC_ENERGY)
    {
        const double c_mu = rCurrentProcessInfo[TURBULENCE_RANS_C_MU];

        EvmKepsilonModelAdjointUtilities::CalculateGaussSensitivities(
            rOutput, rCurrentData.TurbulentKinematicViscositySensitivitiesK,
            rCurrentData.ShapeFunctions);

        Vector theta_sensitivities(rOutput.size());
        EvmKepsilonModelAdjointUtilities::CalculateThetaTKESensitivity(
            theta_sensitivities, c_mu, 1.0, rCurrentData.TurbulentKineticEnergy,
            rCurrentData.TurbulentKinematicViscosity, rOutput, rCurrentData.ShapeFunctions);

        noalias(rOutput) = theta_sensitivities;
    }
    else if (rDerivativeVariable == TURBULENT_ENERGY_DISSIPATION_RATE)
    {
        const double c_mu = rCurrentProcessInfo[TURBULENCE_RANS_C_MU];

        EvmKepsilonModelAdjointUtilities::CalculateGaussSensitivities(
            rOutput, rCurrentData.TurbulentKinematicViscositySensitivitiesEpsilon,
            rCurrentData.ShapeFunctions);

        Vector theta_sensitivities(rOutput.size());
        EvmKepsilonModelAdjointUtilities::CalculateThetaEpsilonSensitivity(
            theta_sensitivities, c_mu, 1.0, rCurrentData.TurbulentKineticEnergy,
            rCurrentData.TurbulentKinematicViscosity, rOutput);

        noalias(rOutput) = theta_sensitivities;
    }
    else
    {
        KRATOS_ERROR << "Unsupported partial derivative variable "
                     << rDerivativeVariable.Name() << " used in RansEvmKAdjoint::CalculateReactionTermScalarDerivatives method.";
    }
}

template <unsigned int TDim, unsigned int TNumNodes>
void RansEvmKAdjoint<TDim, TNumNodes>::CalculateSourceTermScalarDerivatives(
    Vector& rOutput,
    const Variable<double>& rDerivativeVariable,
    const RansEvmKAdjointData& rCurrentData,
    const ProcessInfo& rCurrentProcessInfo) const
{
    if (rDerivativeVariable == TURBULENT_KINETIC_ENERGY)
    {
        EvmKepsilonModelAdjointUtilities::CalculateGaussSensitivities(
            rOutput, rCurrentData.TurbulentKinematicViscositySensitivitiesK,
            rCurrentData.ShapeFunctions);

        BoundedMatrix<double, TDim, TDim> velocity_gradient;
        this->CalculateGradient(velocity_gradient, VELOCITY,
                                rCurrentData.ShapeFunctionDerivatives);

        EvmKepsilonModelAdjointUtilities::CalculateProductionScalarSensitivities<TDim>(
            rOutput, rOutput, velocity_gradient);
    }
    else if (rDerivativeVariable == TURBULENT_ENERGY_DISSIPATION_RATE)
    {
        EvmKepsilonModelAdjointUtilities::CalculateGaussSensitivities(
            rOutput, rCurrentData.TurbulentKinematicViscositySensitivitiesEpsilon,
            rCurrentData.ShapeFunctions);

        BoundedMatrix<double, TDim, TDim> velocity_gradient;
        this->CalculateGradient(velocity_gradient, VELOCITY,
                                rCurrentData.ShapeFunctionDerivatives);

        EvmKepsilonModelAdjointUtilities::CalculateProductionScalarSensitivities<TDim>(
            rOutput, rOutput, velocity_gradient);
    }
    else
    {
        KRATOS_ERROR << "Unsupported partial derivative variable "
                     << rDerivativeVariable.Name() << " used in RansEvmKAdjoint::CalculateSourceTermScalarDerivatives method.";
    }
}

template <unsigned int TDim, unsigned int TNumNodes>
void RansEvmKAdjoint<TDim, TNumNodes>::Calculate(const Variable<Matrix>& rVariable,
                                                 Matrix& rOutput,
                                                 const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY

    if (rVariable == RANS_TURBULENT_ENERGY_DISSIPATION_RATE_PARTIAL_DERIVATIVE)
    {
        BoundedMatrix<double, TNumNodes, TNumNodes> local_matrix;
        this->CalculateResidualScalarDerivatives(
            TURBULENT_ENERGY_DISSIPATION_RATE, local_matrix, rCurrentProcessInfo);
        if (rOutput.size1() != local_matrix.size1() ||
            rOutput.size2() != local_matrix.size2())
            rOutput.resize(local_matrix.size1(), local_matrix.size2(), false);

        noalias(rOutput) = local_matrix;
    }
    else
    {
        BaseType::Calculate(rVariable, rOutput, rCurrentProcessInfo);
    }

    KRATOS_CATCH("");
}

template <unsigned int TDim, unsigned int TNumNodes>
void RansEvmKAdjoint<TDim, TNumNodes>::CalculateEffectiveKinematicViscosityVelocityDerivatives(
    Matrix& rOutput, const RansEvmKAdjointData& rCurrentData, const ProcessInfo& rCurrentProcessInfo) const
{
    rOutput.clear();
}

template <unsigned int TDim, unsigned int TNumNodes>
void RansEvmKAdjoint<TDim, TNumNodes>::CalculateReactionTermVelocityDerivatives(
    Matrix& rOutput, const RansEvmKAdjointData& rCurrentData, const ProcessInfo& rCurrentProcessInfo) const
{
    noalias(rOutput) = rCurrentData.ShapeFunctionDerivatives * (2.0 / 3.0);
}

template <unsigned int TDim, unsigned int TNumNodes>
void RansEvmKAdjoint<TDim, TNumNodes>::CalculateSourceTermVelocityDerivatives(
    Matrix& rOutput, const RansEvmKAdjointData& rCurrentData, const ProcessInfo& rCurrentProcessInfo) const
{
    const double c_mu = rCurrentProcessInfo[TURBULENCE_RANS_C_MU];

    Matrix nu_t_sensitivities;
    EvmKepsilonModelAdjointUtilities::CalculateNodalTurbulentViscosityVectorSensitivities(
        nu_t_sensitivities, c_mu, rCurrentData.NodalTurbulentKineticEnergy,
        rCurrentData.NodalTurbulentEnergyDissipationRate,
        ZeroMatrix(rOutput.size1(), rOutput.size2()));
    EvmKepsilonModelAdjointUtilities::CalculateGaussSensitivities(
        nu_t_sensitivities, nu_t_sensitivities, rCurrentData.ShapeFunctions);

    BoundedMatrix<double, TDim, TDim> velocity_gradient;
    this->CalculateGradient(velocity_gradient, VELOCITY, rCurrentData.ShapeFunctionDerivatives);

    EvmKepsilonModelAdjointUtilities::CalculateProductionVelocitySensitivities<TDim>(
        rOutput, rCurrentData.TurbulentKinematicViscosity, nu_t_sensitivities,
        velocity_gradient, rCurrentData.ShapeFunctionDerivatives);
}

template <unsigned int TDim, unsigned int TNumNodes>
double RansEvmKAdjoint<TDim, TNumNodes>::CalculateEffectiveKinematicViscosityShapeSensitivity(
    const RansEvmKAdjointData& rCurrentData,
    const ShapeParameter& rShapeDerivative,
    const double detJ_deriv,
    const GeometricalSensitivityUtility::ShapeFunctionsGradientType& rDN_Dx_deriv,
    const ProcessInfo& rCurrentProcessInfo) const
{
    return 0.0;
}

template <unsigned int TDim, unsigned int TNumNodes>
double RansEvmKAdjoint<TDim, TNumNodes>::CalculateReactionTermShapeSensitivity(
    const RansEvmKAdjointData& rCurrentData,
    const ShapeParameter& rShapeDerivative,
    const double detJ_deriv,
    const GeometricalSensitivityUtility::ShapeFunctionsGradientType& rDN_Dx_deriv,
    const ProcessInfo& rCurrentProcessInfo) const
{
    return (2.0 / 3.0) * this->GetDivergenceOperator(VELOCITY, rDN_Dx_deriv);
}

template <unsigned int TDim, unsigned int TNumNodes>
double RansEvmKAdjoint<TDim, TNumNodes>::CalculateSourceTermShapeSensitivity(
    const RansEvmKAdjointData& rCurrentData,
    const ShapeParameter& rShapeDerivative,
    const double detJ_deriv,
    const GeometricalSensitivityUtility::ShapeFunctionsGradientType& rDN_Dx_deriv,
    const ProcessInfo& rCurrentProcessInfo) const
{
    double value;

    BoundedMatrix<double, TDim, TDim> velocity_gradient;
    this->CalculateGradient(velocity_gradient, VELOCITY, rCurrentData.ShapeFunctionDerivatives);

    EvmKepsilonModelAdjointUtilities::CalculateProductionShapeSensitivities<TDim>(
        value, rCurrentData.TurbulentKinematicViscosity, 0.0, rCurrentData.NodalVelocity,
        velocity_gradient, rCurrentData.ShapeFunctionDerivatives, rDN_Dx_deriv);

    return value;
}

///@}
///@name Serialization
///@{

template <unsigned int TDim, unsigned int TNumNodes>
void RansEvmKAdjoint<TDim, TNumNodes>::save(Serializer& rSerializer) const
{
    KRATOS_SERIALIZE_SAVE_BASE_CLASS(rSerializer, Element);

    // List
    // To be completed with the class member list
}

template <unsigned int TDim, unsigned int TNumNodes>
void RansEvmKAdjoint<TDim, TNumNodes>::load(Serializer& rSerializer)
{
    KRATOS_SERIALIZE_LOAD_BASE_CLASS(rSerializer, Element);

    // List
    // To be completed with the class member list
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
///@name Type Definitions
///@{

///@}
///@name Input and output
///@{

/// input stream function

template <unsigned int TDim, unsigned int TNumNodes>
inline std::istream& operator>>(std::istream& rIStream,
                                RansEvmKAdjoint<TDim, TNumNodes>& rThis);

/// output stream function

template <unsigned int TDim, unsigned int TNumNodes>
inline std::ostream& operator<<(std::ostream& rOStream,
                                const RansEvmKAdjoint<TDim, TNumNodes>& rThis)
{
    rThis.PrintInfo(rOStream);
    rOStream << " : " << std::endl;
    rThis.PrintData(rOStream);
    return rOStream;
}

// Class template instantiation

// K only
template class RansEvmKAdjoint<2, 3>;
template class RansEvmKAdjoint<3, 4>;
template class RansEvmKAdjoint<2, 4>;
template class RansEvmKAdjoint<3, 8>;

} // namespace Kratos.
