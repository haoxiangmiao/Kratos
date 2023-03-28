// KRATOS___
//     //   ) )
//    //         ___      ___
//   //  ____  //___) ) //   ) )
//  //    / / //       //   / /
// ((____/ / ((____   ((___/ /  MECHANICS
//
//  License:         geo_mechanics_application/license.txt
//
//  Main authors:    Vahid Galavi
//

#if !defined(KRATOS_GEO_U_PW_UPDATED_LAGRANGIAN_DIFFERENT_ORDER_AXISYMMETRIC_ELEMENT_H_INCLUDED )
#define  KRATOS_GEO_U_PW_UPDATED_LAGRANGIAN_DIFFERENT_ORDER_AXISYMMETRIC_ELEMENT_H_INCLUDED

// Project includes
#include "includes/serializer.h"

// Application includes
#include "custom_utilities/stress_strain_utilities.hpp"
#include "custom_elements/updated_lagrangian_U_Pw_diff_order_element.hpp"
#include "custom_utilities/element_utilities.hpp"
#include "geo_mechanics_application_variables.h"

namespace Kratos
{

class KRATOS_API(GEO_MECHANICS_APPLICATION) UpdatedLagrangianUPwDiffOrderAxisymmetricElement :
    public UpdatedLagrangianUPwDiffOrderElement
{

public:

    KRATOS_CLASS_INTRUSIVE_POINTER_DEFINITION( UpdatedLagrangianUPwDiffOrderAxisymmetricElement );

    using IndexType = std::size_t;
    using PropertiesType = Properties;
    using NodeType = Node<3>;
    using GeometryType = Geometry<NodeType>;
    using NodesArrayType = GeometryType::PointsArrayType;
    using VectorType = Vector;
    using MatrixType = Matrix;
    /// The definition of the sizetype
    using SizeType = std::size_t;
    using UpdatedLagrangianUPwDiffOrderElement::mConstitutiveLawVector;

///----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    /// Default Constructor
    UpdatedLagrangianUPwDiffOrderAxisymmetricElement()
        : UpdatedLagrangianUPwDiffOrderElement() {}

    /// Constructor using Geometry
    UpdatedLagrangianUPwDiffOrderAxisymmetricElement(IndexType NewId,
                                               GeometryType::Pointer pGeometry)
        : UpdatedLagrangianUPwDiffOrderElement(NewId, pGeometry) {}

    /// Constructor using Properties
    UpdatedLagrangianUPwDiffOrderAxisymmetricElement(IndexType NewId,
                                               GeometryType::Pointer pGeometry,
                                               PropertiesType::Pointer pProperties)
        : UpdatedLagrangianUPwDiffOrderElement( NewId, pGeometry, pProperties ) {}

    /// Destructor
    ~UpdatedLagrangianUPwDiffOrderAxisymmetricElement() override {}

///----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    Element::Pointer Create( IndexType NewId,
                             NodesArrayType const& ThisNodes,
                             PropertiesType::Pointer pProperties ) const override;

    Element::Pointer Create( IndexType NewId,
                             GeometryType::Pointer pGeom,
                             PropertiesType::Pointer pProperties ) const override;

///----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    // Turn back information as a string.
    std::string Info() const override
    {
        std::stringstream buffer;
        buffer << "Updated-Lagrangian axisymmetric U-Pw element with different order #"
               << this->Id()
               << "\nConstitutive law: "
               << mConstitutiveLawVector[0]->Info();
        return buffer.str();
    }

    // Print information about this object.
    void PrintInfo(std::ostream& rOStream) const override
    {
        rOStream << "Updated-Lagrangian axisymmetric U-Pw element with different order #"
                 << this->Id()
                 << "\nConstitutive law: "
                 << mConstitutiveLawVector[0]->Info();
    }

///----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

protected:

    /// Member Variables

///----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    void CalculateBMatrix( Matrix &rB,
                           const Matrix &GradNpT,
                           const Vector &Np ) override;

    double CalculateIntegrationCoefficient( const GeometryType::IntegrationPointsArrayType& IntegrationPoints,
                                            unsigned int PointNumber,
                                            double detJ) override;

///----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

private:

    /// Member Variables

///----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    /// Serialization

    friend class Serializer;

    void save(Serializer& rSerializer) const override
    {
        KRATOS_SERIALIZE_SAVE_BASE_CLASS( rSerializer, Element )
    }

    void load(Serializer& rSerializer) override
    {
        KRATOS_SERIALIZE_LOAD_BASE_CLASS( rSerializer, Element )
    }

    // Assignment operator.
    UpdatedLagrangianUPwDiffOrderAxisymmetricElement & operator=(UpdatedLagrangianUPwDiffOrderAxisymmetricElement const& rOther);

    // Copy constructor.
    UpdatedLagrangianUPwDiffOrderAxisymmetricElement(UpdatedLagrangianUPwDiffOrderAxisymmetricElement const& rOther);

    // Private Operations

    template < class TValueType >
    inline void ThreadSafeNodeWrite(NodeType& rNode, const Variable<TValueType> &Var, const TValueType Value)
    {
        rNode.SetLock();
        rNode.FastGetSolutionStepValue(Var) = Value;
        rNode.UnSetLock();
    }


}; // Class UpdatedLagrangianUPwDiffOrderAxisymmetricElement

} // namespace Kratos

#endif // KRATOS_GEO_U_PW_UPDATED_LAGRANGIAN_DIFFERENT_ORDER_AXISYMMETRIC_ELEMENT_H_INCLUDED  defined
