//  Kratos Multi-Physics - ThermalDEM Application
//
//  License:       BSD License
//                 Kratos default license: kratos/license.txt
//
//  Main authors:  Chengshun Shang (cshang@cimne.upc.edu)
//

#if !defined(DEM_ROLLING_FRICTION_MODEL_CONSTANT_TORQUE_H_INCLUDED)
#define DEM_ROLLING_FRICTION_MODEL_CONSTANT_TORQUE_H_INCLUDED

/* Project includes */
#include "../custom_elements/spheric_particle.h"
#include "DEM_rolling_friction_model.h"


namespace Kratos{

    class KRATOS_API(DEM_APPLICATION) DEMRollingFrictionModelConstantTorque : public DEMRollingFrictionModel{
    public:

        KRATOS_CLASS_POINTER_DEFINITION(DEMRollingFrictionModelConstantTorque);

        DEMRollingFrictionModelConstantTorque() {}

        void Check(Properties::Pointer pProp) const override;

        ~DEMRollingFrictionModelConstantTorque() {}

        DEMRollingFrictionModel::Pointer Clone() const override;

        std::unique_ptr<DEMRollingFrictionModel> CloneUnique() override;

        void ComputeRollingFriction(SphericParticle* p_element, SphericParticle* p_neighbor, double LocalCoordSystem_2[3], double LocalContactForce[3], array_1d<double, 3>& mContactMoment) override;
        
        void ComputeRollingFrictionWithWall(SphericParticle* p_element, Condition* const wall, double LocalCoordSystem_2[3], double LocalContactForce[3], array_1d<double, 3>& mContactMoment) override;
    
    private:

        friend class Serializer;

        void save(Serializer& rSerializer) const override {
                    //rSerializer.save("MyMemberName",myMember);
        }

        void load(Serializer& rSerializer) override {
                    //rSerializer.load("MyMemberName",myMember);
        }

    };

    
} /* namespace Kratos.*/

#endif /*DEM_ROLLING_FRICTION_MODEL_CONSTANT_TORQUE_H_INCLUDED defined*/