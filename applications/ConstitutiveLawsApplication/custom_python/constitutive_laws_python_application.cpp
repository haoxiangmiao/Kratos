// KRATOS ___                _   _ _         _   _             __                       _
//       / __\___  _ __  ___| |_(_) |_ _   _| |_(_)_   _____  / /  __ ___      _____   /_\  _ __  _ __
//      / /  / _ \| '_ \/ __| __| | __| | | | __| \ \ / / _ \/ /  / _` \ \ /\ / / __| //_\\| '_ \| '_  |
//     / /__| (_) | | | \__ \ |_| | |_| |_| | |_| |\ V /  __/ /__| (_| |\ V  V /\__ \/  _  \ |_) | |_) |
//     \____/\___/|_| |_|___/\__|_|\__|\__,_|\__|_| \_/ \___\____/\__,_| \_/\_/ |___/\_/ \_/ .__/| .__/
//                                                                                         |_|   |_|
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Alejandro Cornejo Velazquez
//                   Riccardo Rossi
//


// System includes

#if defined(KRATOS_PYTHON)
// External includes
#include <pybind11/pybind11.h>


// Project includes
#include "includes/define_python.h"
#include "constitutive_laws_application.h"
#include "constitutive_laws_application_variables.h"

#include "custom_python/add_custom_constitutive_laws_to_python.h"
#include "custom_python/add_custom_processes_to_python.h"


namespace Kratos {
namespace Python {

PYBIND11_MODULE(KratosConstitutiveLawsApplication,m)
{
    namespace py = pybind11;

    py::class_<KratosConstitutiveLawsApplication,
        KratosConstitutiveLawsApplication::Pointer,
        KratosApplication>(m, "KratosConstitutiveLawsApplication")
        .def(py::init<>())
        ;

    AddCustomConstitutiveLawsToPython(m);
    AddCustomProcessesToPython(m);

    // Constitutive laws variables
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, IS_PRESTRESSED)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, FIBER_VOLUMETRIC_PARTICIPATION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SERIAL_PARALLEL_IMPOSED_STRAIN)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, CRACK_RECLOSING)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DELAMINATION_DAMAGE_VECTOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DELAMINATION_DAMAGE_VECTOR_MODE_ONE)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DELAMINATION_DAMAGE_VECTOR_MODE_TWO)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, HIGH_CYCLE_FATIGUE_COEFFICIENTS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, STRESS_LIMITS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, HARDENING_PARAMETERS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, FATIGUE_REDUCTION_FACTOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, LOCAL_NUMBER_OF_CYCLES)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, WOHLER_STRESS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, REVERSION_FACTOR_RELATIVE_ERROR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, MAX_STRESS_RELATIVE_ERROR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, MAX_STRESS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, THRESHOLD_STRESS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, CYCLE_INDICATOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, CYCLES_TO_FAILURE)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TIME_INCREMENT)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DAMAGE_ACTIVATION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PREVIOUS_CYCLE);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, CYCLE_PERIOD)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, ADVANCE_STRATEGY_APPLIED)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SYMMETRIZE_TANGENT_OPERATOR)

    // Constitutive laws variables
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, INELASTIC_FLAG)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, INFINITY_YIELD_STRESS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, YIELD_STRESS_TENSION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PLASTIC_STRAIN_VECTOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PLASTIC_DEFORMATION_GRADIENT)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, YIELD_STRESS_COMPRESSION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DILATANCY_ANGLE)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SOFTENING_TYPE)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SOFTENING_TYPE_COMPRESSION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, HARDENING_CURVE)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, MAX_NUMBER_NL_CL_ITERATIONS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, VISCOUS_PARAMETER)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DELAY_TIME)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, UNIAXIAL_STRESS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PLASTIC_DISSIPATION_LIMIT_LINEAR_SOFTENING)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, FRICTION_ANGLE)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DISSIPATION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DAMAGE)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DAMAGE_MATRIX)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DAMAGE_FIBER)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, THRESHOLD)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, COHESION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, INTEGRATED_STRESS_TENSOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PLASTIC_STRAIN_TENSOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DAMAGE_TENSION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DAMAGE_COMPRESSION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, THRESHOLD_TENSION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, THRESHOLD_COMPRESSION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, UNIAXIAL_STRESS_TENSION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, UNIAXIAL_STRESS_COMPRESSION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, FRACTURE_ENERGY_COMPRESSION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, CURVE_FITTING_PARAMETERS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TANGENCY_REGION2)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PLASTIC_STRAIN_INDICATORS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, EQUIVALENT_PLASTIC_STRAIN)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, KINEMATIC_PLASTICITY_PARAMETERS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, KINEMATIC_HARDENING_TYPE)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, CONSIDER_PERTURBATION_THRESHOLD)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TANGENT_OPERATOR_ESTIMATION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TENSION_STRESS_TENSOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, COMPRESSION_STRESS_TENSOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TENSION_STRESS_VECTOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, COMPRESSION_STRESS_VECTOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, EFFECTIVE_TENSION_STRESS_VECTOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, EFFECTIVE_COMPRESSION_STRESS_VECTOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, CAUCHY_STRESS_TENSOR_FIBER)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, CAUCHY_STRESS_VECTOR_FIBER)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, CAUCHY_STRESS_TENSOR_MATRIX)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, CAUCHY_STRESS_VECTOR_MATRIX)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, GREEN_LAGRANGE_STRAIN_TENSOR_MATRIX)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, GREEN_LAGRANGE_STRAIN_VECTOR_MATRIX)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, GREEN_LAGRANGE_STRAIN_TENSOR_FIBER)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, GREEN_LAGRANGE_STRAIN_VECTOR_FIBER)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, EXPONENTIAL_SATURATION_YIELD_STRESS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, ACCUMULATED_PLASTIC_STRAIN)
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, EULER_ANGLES)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, LAYER_EULER_ANGLES)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, BACK_STRESS_VECTOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, BACK_STRESS_TENSOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, FRACTURE_ENERGY_DAMAGE_PROCESS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, OGDEN_BETA_1)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, OGDEN_BETA_2)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, MULTI_LINEAR_ELASTICITY_MODULI)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, MULTI_LINEAR_ELASTICITY_STRAINS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PLASTIC_DAMAGE_PROPORTION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, STRAIN_DAMAGE_CURVE)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, STRESS_DAMAGE_CURVE)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, EQUIVALENT_STRESS_VECTOR_PLASTICITY_POINT_CURVE)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PLASTIC_STRAIN_VECTOR_PLASTICITY_POINT_CURVE)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, UNIAXIAL_STRESS_FIBER)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, UNIAXIAL_STRESS_MATRIX)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, INTERFACIAL_NORMAL_STRENGTH)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, INTERFACIAL_SHEAR_STRENGTH)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, MODE_ONE_FRACTURE_ENERGY)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, MODE_TWO_FRACTURE_ENERGY)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, BK_COEFFICIENT)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TENSILE_INTERAFCE_MODULUS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHEAR_INTERAFCE_MODULUS)


    // D+D- Damage Constitutive laws variables, additional Masonry 2D & 3D
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, BACK_STRESS_VECTOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, BACK_STRESS_TENSOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DAMAGE_ONSET_STRESS_COMPRESSION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, BIAXIAL_COMPRESSION_MULTIPLIER)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, FRACTURE_ENERGY_TENSION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, RESIDUAL_STRESS_COMPRESSION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, BEZIER_CONTROLLER_C1)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, BEZIER_CONTROLLER_C2)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, BEZIER_CONTROLLER_C3)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, YIELD_STRAIN_COMPRESSION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TRIAXIAL_COMPRESSION_COEFFICIENT)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, INTEGRATION_IMPLEX)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TENSION_YIELD_MODEL)

    // The ratios between the yield strength in the isotropic space and the anisotropic space
    // at each direction in local coordinates ratio_x = ft / ft,x
    KRATOS_REGISTER_IN_PYTHON_SYMMETRIC_3D_TENSOR_VARIABLE_WITH_COMPONENTS(m, ISOTROPIC_ANISOTROPIC_YIELD_RATIO );
    KRATOS_REGISTER_IN_PYTHON_SYMMETRIC_3D_TENSOR_VARIABLE_WITH_COMPONENTS(m, ORTHOTROPIC_ELASTIC_CONSTANTS );
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SERIAL_PARALLEL_EQUILIBRIUM_TOLERANCE);


}

} // namespace Python.
} // namespace Kratos.

#endif // KRATOS_PYTHON defined
