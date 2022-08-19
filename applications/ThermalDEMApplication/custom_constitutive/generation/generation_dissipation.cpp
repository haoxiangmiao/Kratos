//  Kratos Multi-Physics - ThermalDEM Application
//
//  License:       BSD License
//                 Kratos default license: kratos/license.txt
//
//  Main authors:  Rafael Rangel (rrangel@cimne.upc.edu)
//

// System includes

// External includes

// Project includes
#include "generation_dissipation.h"

namespace Kratos {
  //-----------------------------------------------------------------------------------------------------------------------
  GenerationDissipation::GenerationDissipation() {}
  GenerationDissipation::~GenerationDissipation() {}

  //------------------------------------------------------------------------------------------------------------
  double GenerationDissipation::ComputeHeatGeneration(const ProcessInfo& r_process_info, ThermalSphericParticle* particle) {
    KRATOS_TRY

    // Check for contact
    if (!particle->mNeighborInContact)
      return 0.0;

    // Get contact info with current neighbor
    typename ThermalSphericParticle::ContactParams contact_params = particle->GetContactParameters();

    // Time passed since last thermal solution
    double time = particle->mNumStepsEval * r_process_info[DELTA_TIME];

    // Conversion and partition coefficients
    const double conversion = r_process_info[HEAT_GENERATION_RATIO];
    const double partition  = ComputePartitionCoeff(particle);

    // Initialize contribution from different sources of energy dissipation
    double heat_gen_sliding = 0.0;
    double heat_gen_damping = 0.0;

    // Convert dissipated power (energy/time) to heat
    if (r_process_info[GENERATION_SLIDING_OPTION]) {
      if (particle->mNeighborType & PARTICLE_NEIGHBOR) {
        // Energy multiplied by 2 as it was calculated by equally splitting the energy with neighbor
        heat_gen_sliding = 2.0 * partition * conversion * contact_params.frictional_energy / time;
        particle->mGenerationHeatFlux_slid_particle += heat_gen_sliding;
      }
      else if (particle->mNeighborType & WALL_NEIGHBOR) {
        // Energy was calculated by assuming that it goes entirely to the particle
        heat_gen_sliding = partition * conversion * contact_params.frictional_energy / time;
        particle->mGenerationHeatFlux_slid_wall += heat_gen_sliding;
      }
    }

    if (r_process_info[GENERATION_DAMPING_OPTION]) {
      if (particle->mNeighborType & PARTICLE_NEIGHBOR) {
        // Energy multiplied by 2 as it was calculated by equally splitting the energy with neighbor
        heat_gen_damping = 2.0 * partition * conversion * contact_params.viscodamping_energy / time;
        particle->mGenerationHeatFlux_damp_particle += heat_gen_damping;
      }
      else if (particle->mNeighborType & WALL_NEIGHBOR) {
        // Energy was calculated by assuming that it goes entirely to the particle
        heat_gen_damping = partition * conversion * contact_params.viscodamping_energy / time;
        particle->mGenerationHeatFlux_damp_wall += heat_gen_damping;
      }
    }

    return heat_gen_sliding + heat_gen_damping;

    KRATOS_CATCH("")
  }

  //------------------------------------------------------------------------------------------------------------
  double GenerationDissipation::ComputePartitionCoeff(ThermalSphericParticle* particle) {
    KRATOS_TRY

    const double k1 = particle->GetParticleConductivity();
    const double k2 = particle->GetNeighborConductivity();
    return k1 / (k1 + k2);

    KRATOS_CATCH("")
  }

} // namespace Kratos
