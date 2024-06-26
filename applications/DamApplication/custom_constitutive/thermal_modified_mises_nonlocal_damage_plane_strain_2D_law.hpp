//
//   Project Name:                  KratosDamApplication $
//   Last Modified by:    $Author:    Ignasi de Pouplana $
//   Date:                $Date:           February 2017 $
//   Revision:            $Revision:                 1.0 $
//

#if !defined (KRATOS_THERMAL_MODIFIED_MISES_NONLOCAL_DAMAGE_PLANE_STRAIN_2D_LAW_H_INCLUDED)
#define  KRATOS_THERMAL_MODIFIED_MISES_NONLOCAL_DAMAGE_PLANE_STRAIN_2D_LAW_H_INCLUDED

// Project includes
#include "includes/serializer.h"

// Application includes
#include "custom_constitutive/thermal_nonlocal_damage_plane_strain_2D_law.hpp"
#include "custom_constitutive/continuum_laws/custom_hardening_laws/modified_exponential_damage_hardening_law.hpp"
#include "custom_constitutive/continuum_laws/custom_yield_criteria/modified_mises_yield_criterion.hpp"
#include "custom_constitutive/continuum_laws/custom_flow_rules/nonlocal_damage_flow_rule.hpp"
#include "dam_application_variables.h"

namespace Kratos
{

class KRATOS_API(DAM_APPLICATION) ThermalModifiedMisesNonlocalDamagePlaneStrain2DLaw : public ThermalNonlocalDamagePlaneStrain2DLaw
{

public:

    KRATOS_CLASS_POINTER_DEFINITION(ThermalModifiedMisesNonlocalDamagePlaneStrain2DLaw);

    typedef FlowRule::Pointer FlowRulePointer;
    typedef YieldCriterion::Pointer YieldCriterionPointer;
    typedef HardeningLaw::Pointer HardeningLawPointer;

///----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    /// Default Constructor
    ThermalModifiedMisesNonlocalDamagePlaneStrain2DLaw();

    /// Second Constructor
    ThermalModifiedMisesNonlocalDamagePlaneStrain2DLaw(FlowRulePointer pFlowRule, YieldCriterionPointer pYieldCriterion, HardeningLawPointer pHardeningLaw);

    /// Copy Constructor
    ThermalModifiedMisesNonlocalDamagePlaneStrain2DLaw (const ThermalModifiedMisesNonlocalDamagePlaneStrain2DLaw& rOther);

    /// Destructor
    virtual ~ThermalModifiedMisesNonlocalDamagePlaneStrain2DLaw();

///----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    int Check(const Properties& rMaterialProperties, const GeometryType& rElementGeometry, const ProcessInfo& rCurrentProcessInfo) const override;

    ConstitutiveLaw::Pointer Clone() const override;

///----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

protected:

    /// Member Variables

///----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

private:

    /// Serialization

    friend class Serializer;

    void save(Serializer& rSerializer) const override
    {
        KRATOS_SERIALIZE_SAVE_BASE_CLASS( rSerializer, ConstitutiveLaw )
    }

    void load(Serializer& rSerializer) override
    {
        KRATOS_SERIALIZE_LOAD_BASE_CLASS( rSerializer, ConstitutiveLaw )
    }

}; // Class ThermalModifiedMisesNonlocalDamagePlaneStrain2DLaw
}  // namespace Kratos.
#endif // KRATOS_THERMAL_MODIFIED_MISES_NONLOCAL_DAMAGE_PLANE_STRAIN_2D_LAW_H_INCLUDED  defined