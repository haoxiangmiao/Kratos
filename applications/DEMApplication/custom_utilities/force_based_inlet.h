// Author: Guillermo Casas (gcasas@cimne.upc.edu)

#if !defined(DEM_FORCE_BASED_INLET_H)
#define DEM_FORCE_BASED_INLET_H

// Project includes
#include "inlet.h"

namespace Kratos {

    class KRATOS_API(DEM_APPLICATION) DEM_Force_Based_Inlet: public DEM_Inlet
    {
    public:
        typedef GlobalPointersVector<Element >::iterator ParticleWeakIteratorType;
        typedef GlobalPointersVector<Element> ParticleWeakVectorType;
        typedef ModelPart::ElementsContainerType ElementsArrayType;

        KRATOS_CLASS_POINTER_DEFINITION(DEM_Force_Based_Inlet);

        /// Constructor:
        DEM_Force_Based_Inlet(ModelPart& inlet_modelpart, array_1d<double, 3> injection_force, const int seed=42);

        /// Destructor.
        virtual ~DEM_Force_Based_Inlet(){}

    private:
        array_1d<double, 3> mInjectionForce;
        void FixInjectionConditions(Element* p_element, Element* p_injector_element) override;
        void FixInjectorConditions(Element* p_element) override;
        void RemoveInjectionConditions(Element &element, const int dimension) override;
        virtual array_1d<double, 3> GetInjectionForce(Element* p_element);
    };
}// namespace Kratos.

#endif // DEM_FORCE_BASED_INLET_H defined
