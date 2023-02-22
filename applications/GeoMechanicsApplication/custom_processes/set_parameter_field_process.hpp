//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Aron Noordam
//

#pragma once

// System includes

// External includes

// Project includes
#include "includes/define.h"
#include "processes/process.h"
#include "includes/model_part.h"
#include "includes/kratos_parameters.h"

#include "utilities/function_parser_utility.h"
#include "utilities/mortar_utilities.h"
namespace Kratos {

///@name Kratos Globals
///@{

///@name Kratos Classes
///@{


/**
 * @class SetParameterFieldProcess
 * @ingroup GeoMechanicsApplication
 * @brief Process to set a parameter field
 * @details This process sets values from a custom parameter field per individual element within the model part. the possibilities are: setting a parameter field from an
 * input function; setting a parameter field with a user defined python script; setting a parameter field from a json file.
 * @author Aron Noordam
*/
class KRATOS_API(GEO_MECHANICS_APPLICATION) SetParameterFieldProcess : public Process
{
public:
    ///@name Type Definitions
    ///@{

    ///@}
    ///@name Pointer Definitions
    /// Pointer definition of SetMovingLoadProcess
    KRATOS_CLASS_POINTER_DEFINITION(SetParameterFieldProcess);

    typedef std::size_t SizeType;

    ///@}
    ///@name Life Cycle
    ///@{

    SetParameterFieldProcess(ModelPart& rModelPart,
                                  Parameters Parameters);

    ///@}
    ///@name Operations
    ///@{

    /**
     * \brief  Initializes the set parameter field process. Checks what type of input field is given,
     */
    void ExecuteInitialize() override;

    
    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    virtual std::string Info() const override {
        return "SetParameterFieldProcess";
    }

    /// Print information about this object.
    void PrintInfo(std::ostream& rOStream) const override {
        rOStream << "SetParameterFieldProcess";
    }

    /// Print object's data.
    void PrintData(std::ostream& rOStream) const override {
    }

    ///@}

private:
    ///@name Member Variables
    ///@{

    ModelPart& mrModelPart;
    Parameters mParameters;


    ///@}
    ///@name Private Operations
    ///@{

    /**
     * \brief Sets the value of a certain variable for only 1 element
     * \param rElement element for which a variable should be altered
     * \param rVar variable type which is to be altered
     * \param Value new value for the to be altered variable
     */
	void SetValueAtElement(Element& rElement, const Variable<double>& rVar, double Value);

    ///@}
    ///@name Serialization
    ///@{

    ///@}

}; // Class SetParameterFieldProcess

///@}

}  // namespace Kratos.
