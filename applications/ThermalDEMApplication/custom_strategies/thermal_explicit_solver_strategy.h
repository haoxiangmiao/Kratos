//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ \.
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics ThermalDEM Application
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Rafael Rangel (rrangel@cimne.upc.edu)
//

#if !defined(KRATOS_THERMAL_EXPLICIT_SOLVER_STRATEGY)
#define KRATOS_THERMAL_EXPLICIT_SOLVER_STRATEGY

// System includes

// External includes

// Project includes
#include "custom_strategies/strategies/explicit_solver_strategy.h"
#include "custom_elements/thermal_spheric_particle.h"

namespace Kratos
{

class KRATOS_API(THERMAL_DEM_APPLICATION) ThermalExplicitSolverStrategy : public ExplicitSolverStrategy
{
  public:

    // Pointer definition of ThermalExplicitSolverStrategy
    KRATOS_CLASS_POINTER_DEFINITION(ThermalExplicitSolverStrategy);

    using ExplicitSolverStrategy::mListOfSphericParticles;

    // Constructor
    ThermalExplicitSolverStrategy();

    ThermalExplicitSolverStrategy(ExplicitSolverSettings&            settings,
                                  const double                       max_delta_time,
                                  const int                          n_step_search,
                                  const double                       safety_factor,
                                  const int                          delta_option,
                                  ParticleCreatorDestructor::Pointer p_creator_destructor,
                                  DEM_FEM_Search::Pointer            p_dem_fem_search,
                                  SpatialSearch::Pointer             pSpSearch,
                                  Parameters                         strategy_parameters);

    // Destructor
    virtual ~ThermalExplicitSolverStrategy();

    // Public derived methods
    void SetSearchRadiiOnAllParticles        (ModelPart& r_model_part, double added_search_distance = 0.0, double amplification = 1.0) override;
    void SetSearchRadiiWithFemOnAllParticles (ModelPart& r_model_part, double added_search_distance = 0.0, double amplification = 1.0) override;

    // Public particular methods
    double SolveSolutionStepStatic();

  protected:
    // Protected particular methods
    void SetSearchRadii(ModelPart & r_model_part, double added_search_distance, double amplification);

}; // Class ThermalExplicitSolverStrategy
} // namespace Kratos

#endif // KRATOS_THERMAL_EXPLICIT_SOLVER_STRATEGY defined
