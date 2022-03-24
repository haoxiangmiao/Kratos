//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ \
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics ThermalDEM Application
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Rafael Rangel (rrangel@cimne.upc.edu)
//

#if !defined(DIRECT_CONDUCTION_MODEL_BOB_MODIFIED_H_INCLUDED)
#define DIRECT_CONDUCTION_MODEL_BOB_MODIFIED_H_INCLUDED

// System includes

// External includes

// Project includes
#include "direct_conduction_bob_complete.h"

namespace Kratos
{
  class KRATOS_API(THERMAL_DEM_APPLICATION) DirectConductionBOBModified : public DirectConductionBOBComplete
  {
    public:

      // Pointer definition
      KRATOS_CLASS_POINTER_DEFINITION(DirectConductionBOBModified);

      // Constructor / Destructor
      DirectConductionBOBModified();
      virtual ~DirectConductionBOBModified();

      // Public methods
      double ComputeHeatFlux             (const ProcessInfo& r_process_info, ThermalSphericParticle* particle) override;
      double ComputeInnerResistance      (const ProcessInfo& r_process_info, ThermalSphericParticle* particle);
      double InnerResistanceWithParticle (const ProcessInfo& r_process_info, ThermalSphericParticle* particle);
      double InnerResistanceWithWall     (const ProcessInfo& r_process_info, ThermalSphericParticle* particle);

      // Clone
      HeatExchangeMechanism* CloneRaw() const override {
        HeatExchangeMechanism* cloned_model(new DirectConductionBOBModified(*this));
        return cloned_model;
      }

      HeatExchangeMechanism::Pointer CloneShared() const override {
        HeatExchangeMechanism::Pointer cloned_model(new DirectConductionBOBModified(*this));
        return cloned_model;
      }

      // Turn back information as a string
      virtual std::string Info() const override {
        std::stringstream buffer;
        buffer << "DirectConductionBOBModified";
        return buffer.str();
      }

      // Print object information
      virtual void PrintInfo(std::ostream& rOStream) const override { rOStream << "DirectConductionBOBModified"; }
      virtual void PrintData(std::ostream& rOStream) const override {}

    private:

      // Assignment operator / Copy constructor
      DirectConductionBOBModified& operator=(DirectConductionBOBModified const& rOther) {return *this;}
      DirectConductionBOBModified(DirectConductionBOBModified const& rOther) {*this = rOther;}

  }; // Class DirectConductionBOBModified

  // input stream function
  inline std::istream& operator>>(std::istream& rIStream,
    DirectConductionBOBModified& rThis) {
    return rIStream;
  }

  // output stream function
  inline std::ostream& operator<<(std::ostream& rOStream,
    const DirectConductionBOBModified& rThis) {
    rThis.PrintInfo(rOStream);
    rOStream << std::endl;
    rThis.PrintData(rOStream);
    return rOStream;
  }

} // namespace Kratos

#endif // DIRECT_CONDUCTION_MODEL_BOB_MODIFIED_H_INCLUDED
