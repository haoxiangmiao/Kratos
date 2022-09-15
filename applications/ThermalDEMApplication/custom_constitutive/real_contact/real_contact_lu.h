//  Kratos Multi-Physics - ThermalDEM Application
//
//  License:       BSD License
//                 Kratos default license: kratos/license.txt
//
//  Main authors:  Rafael Rangel (rrangel@cimne.upc.edu)
//

#if !defined(REAL_CONTACT_MODEL_LU_H_INCLUDED)
#define REAL_CONTACT_MODEL_LU_H_INCLUDED

// System includes

// External includes

// Project includes
#include "real_contact_model.h"
#include "custom_elements/thermal_spheric_particle.h"

namespace Kratos
{
  class KRATOS_API(THERMAL_DEM_APPLICATION) RealContactLu : public RealContactModel
  {
    public:

      // Pointer definition
      KRATOS_CLASS_POINTER_DEFINITION(RealContactLu);

      // Constructor / Destructor
      RealContactLu();
      virtual ~RealContactLu();

      // Public methods
      void AdjustContact(const ProcessInfo& r_process_info, ThermalSphericParticle* particle) override;

      // Clone
      RealContactModel* CloneRaw() const override {
        RealContactModel* cloned_model(new RealContactLu(*this));
        return cloned_model;
      }

      RealContactModel::Pointer CloneShared() const override {
        RealContactModel::Pointer cloned_model(new RealContactLu(*this));
        return cloned_model;
      }

      // Turn back information as a string
      virtual std::string Info() const override {
        std::stringstream buffer;
        buffer << "RealContactLu";
        return buffer.str();
      }

      // Print object information
      virtual void PrintInfo(std::ostream& rOStream) const override { rOStream << "RealContactLu"; }
      virtual void PrintData(std::ostream& rOStream) const override {}

    private:

      // Assignment operator / Copy constructor
      RealContactLu& operator=(RealContactLu const& rOther) {return *this;}
      RealContactLu(RealContactLu const& rOther) {*this = rOther;}

  }; // Class RealContactLu

  // input stream function
  inline std::istream& operator>>(std::istream& rIStream,
    RealContactLu& rThis) {
    return rIStream;
  }

  // output stream function
  inline std::ostream& operator<<(std::ostream& rOStream,
    const RealContactLu& rThis) {
    rThis.PrintInfo(rOStream);
    rOStream << std::endl;
    rThis.PrintData(rOStream);
    return rOStream;
  }

} // namespace Kratos

#endif // REAL_CONTACT_MODEL_LU_H_INCLUDED
