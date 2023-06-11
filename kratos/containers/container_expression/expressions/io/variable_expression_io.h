//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Suneth Warnakulasuriya
//

#pragma once

// System incldues
#include <variant>

// Project includes
#include "containers/array_1d.h"
#include "containers/container_expression/container_expression.h"
#include "containers/container_expression/container_data_io.h"
#include "containers/variable.h"
#include "expression_io.h"
#include "includes/define.h"
#include "includes/model_part.h"

namespace Kratos {


class KRATOS_API(KRATOS_CORE) VariableExpressionIO
{
public:
    ///@name Type definitions
    ///@{

    using VariableType = std::variant<
                                const Variable<int>*,
                                const Variable<double>*,
                                const Variable<array_1d<double, 3>>*,
                                const Variable<array_1d<double, 4>>*,
                                const Variable<array_1d<double, 6>>*,
                                const Variable<array_1d<double, 9>>*,
                                const Variable<Vector>*,
                                const Variable<Matrix>*>;

    ///}
    ///@name Public enums
    ///@{

    enum ContainerType
    {
        NodalHistorical,
        NodalNonHistorical,
        ConditionNonHistorical,
        ElementNonHistorical
    };

    ///@}
    ///@name Public classes
    ///@{

    class KRATOS_API(KRATOS_CORE) VariableExpressionInput : public ExpressionInput
    {
    public:
        ///@name Type definitions
        ///@{

        KRATOS_CLASS_POINTER_DEFINITION(VariableExpressionInput);

        ///@}
        ///@name Life cycle
        ///@{

        template<class TDataType>
        VariableExpressionInput(
            const ModelPart& rModelPart,
            const Variable<TDataType>& rVariable,
            const ContainerType& rContainerType,
            MeshType rMeshType = MeshType::Local);

        VariableExpressionInput(
            const ModelPart& rModelPart,
            const VariableType& rVariable,
            const ContainerType& rContainerType,
            MeshType rMeshType = MeshType::Local);

        template <class TDataType, MeshType TMeshType = Kratos::MeshType::Local>
        VariableExpressionInput(
            const ContainerExpression<ModelPart::NodesContainerType, TMeshType>& rContainer,
            const Variable<TDataType>& rVariable,
            const bool IsHistorical = false);

        template <MeshType TMeshType = Kratos::MeshType::Local>
        VariableExpressionInput(
            const ContainerExpression<ModelPart::NodesContainerType, TMeshType>& rContainer,
            const VariableType& rVariable,
            const bool IsHistorical = false);

        template <class TContainerType, class TDataType, MeshType TMeshType = Kratos::MeshType::Local>
        VariableExpressionInput(
            const ContainerExpression<TContainerType, TMeshType>& rContainer,
            const Variable<TDataType>& rVariable);

        template <class TContainerType, MeshType TMeshType = Kratos::MeshType::Local>
        VariableExpressionInput(
            const ContainerExpression<TContainerType, TMeshType>& rContainer,
            const VariableType& rVariable);

        ~VariableExpressionInput() override = default;

        ///@}
        ///@name Public operations
        ///@{

        Expression::Pointer Execute() const override;

        ///@}

    private:
        ///@name Private member variables
        ///@{

        const ModelPart& mrModelPart;

        const VariableType mpVariable;

        const ContainerType mContainerType;

        const MeshType mMeshType;

        ///@}

    };

    class KRATOS_API(KRATOS_CORE) VariableExpressionOutput : public ExpressionOutput
    {
    public:
        ///@name Type definitions
        ///@{

        KRATOS_CLASS_POINTER_DEFINITION(VariableExpressionOutput);

        ///@}
        ///@name Life cycle
        ///@{

        template<class TDataType>
        VariableExpressionOutput(
            ModelPart& rModelPart,
            const Variable<TDataType>& rVariable,
            const ContainerType& rContainerType,
            MeshType rMeshType = MeshType::Local);

        VariableExpressionOutput(
            ModelPart& rModelPart,
            const VariableType& rVariable,
            const ContainerType& rContainerType,
            MeshType rMeshType = MeshType::Local);

        template <class TDataType, MeshType TMeshType = Kratos::MeshType::Local>
        VariableExpressionOutput(
            ContainerExpression<ModelPart::NodesContainerType, TMeshType>& rContainer,
            const Variable<TDataType>& rVariable,
            const bool IsHistorical = false);

        template <MeshType TMeshType = Kratos::MeshType::Local>
        VariableExpressionOutput(
            ContainerExpression<ModelPart::NodesContainerType, TMeshType>& rContainer,
            const VariableType& rVariable,
            const bool IsHistorical = false);

        template <class TContainerType, class TDataType, MeshType TMeshType = Kratos::MeshType::Local>
        VariableExpressionOutput(
            ContainerExpression<TContainerType, TMeshType>& rContainer,
            const Variable<TDataType>& rVariable);

        template <class TContainerType, MeshType TMeshType = Kratos::MeshType::Local>
        VariableExpressionOutput(
            ContainerExpression<TContainerType, TMeshType>& rContainer,
            const VariableType& rVariable);

        ~VariableExpressionOutput() override = default;

        ///@}
        ///@name Public operations
        ///@{

        void Execute(const Expression& rExpression) override;

        ///@}

    private:
        ///@name Private member variables
        ///@{

        ModelPart& mrModelPart;

        const VariableType mpVariable;

        const ContainerType mContainerType;

        const MeshType mMeshType;

        ///@}

    };

    ///@}
    ///@name Public static operations
    ///@{

    static ModelPart::MeshType& GetMesh(
        Communicator& rCommunicator,
        MeshType rMeshType);

    static const ModelPart::MeshType& GetMesh(
        const Communicator& rCommunicator,
        MeshType rMeshType);

    ///@}

}; // class ExpressionIO


} // namespace Kratos
