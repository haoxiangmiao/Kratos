from typing import Optional

import KratosMultiphysics as Kratos
import KratosMultiphysics.OptimizationApplication as KratosOA
import KratosMultiphysics.DigitalTwinApplication as KratosDT
from KratosMultiphysics.OptimizationApplication.controls.control import Control
from KratosMultiphysics.OptimizationApplication.utilities.union_utilities import ContainerExpressionTypes
from KratosMultiphysics.OptimizationApplication.utilities.union_utilities import SupportedSensitivityFieldVariableTypes
from KratosMultiphysics.OptimizationApplication.utilities.helper_utilities import IsSameContainerExpression
from KratosMultiphysics.OptimizationApplication.utilities.model_part_utilities import ModelPartOperation
from KratosMultiphysics.OptimizationApplication.utilities.optimization_problem import OptimizationProblem
from KratosMultiphysics.OptimizationApplication.utilities.component_data_view import ComponentDataView

def Factory(model: Kratos.Model, parameters: Kratos.Parameters, optimization_problem: OptimizationProblem) -> Control:
    if not parameters.Has("name"):
        raise RuntimeError(f"MaterialPropertiesControl instantiation requires a \"name\" in parameters [ parameters = {parameters}].")
    if not parameters.Has("settings"):
        raise RuntimeError(f"MaterialPropertiesControl instantiation requires a \"settings\" in parameters [ parameters = {parameters}].")

    return MaterialPropertiesControl(parameters["name"].GetString(), model, parameters["settings"], optimization_problem)

class MaterialPropertiesControl(Control):
    """Material properties control

    This is a generic material properties control which creates a control
    for the specified control variable. This does not do any filtering.

    TODO: Extend with filtering techniques when they are implemented.

    """
    def __init__(self, name: str, model: Kratos.Model, parameters: Kratos.Parameters, optimization_problem: OptimizationProblem):
        super().__init__(name)

        default_settings = Kratos.Parameters("""{
            "model_part_names": [
                {
                    "primal_model_part_name" : "PLEASE_PROVIDE_MODEL_PART_NAME",
                    "adjoint_model_part_name": "PLEASE_PROVIDE_MODEL_PART_NAME"
                }
            ],
            "control_variable_name": "",
            "control_variable_bounds": [0.0, 0.0],
            "filtering": {
                "filter_type": "vertex_morphing",
                "filter_radius": 5.0,
                "filter_function_type": "linear",
                "fixed_model_part_name": "",
                "damping_function_type": "sigmoidal",
                "max_nodes_in_filter_radius": 1000
            }
        }""")
        parameters.RecursivelyValidateAndAssignDefaults(default_settings)

        self.model = model

        control_variable_name = parameters["control_variable_name"].GetString()
        control_variable_type = Kratos.KratosGlobals.GetVariableType(control_variable_name)
        if control_variable_type != "Double":
            raise RuntimeError(f"{control_variable_name} with {control_variable_type} type is not supported. Only supports double variables")
        self.controlled_physical_variable = Kratos.KratosGlobals.GetVariable(control_variable_name)

        controlled_model_part_names: 'list[Kratos.Parameters]' = parameters["model_part_names"].values()
        if len(controlled_model_part_names) == 0:
            raise RuntimeError(f"No model parts were provided for MaterialPropertiesControl. [ control name = \"{self.GetName()}\"]")

        self.primal_model_part_operation = ModelPartOperation(
                                                self.model,
                                                ModelPartOperation.OperationType.UNION,
                                                f"control_primal_{self.GetName()}",
                                                [param["primal_model_part_name"].GetString() for param in controlled_model_part_names],
                                                False)
        self.adjoint_model_part_operation = ModelPartOperation(
                                                self.model,
                                                ModelPartOperation.OperationType.UNION,
                                                f"control_adjoint_{self.GetName()}",
                                                [param["adjoint_model_part_name"].GetString() for param in controlled_model_part_names],
                                                False)
        self.primal_model_part: Optional[Kratos.ModelPart] = None
        self.adjoint_model_part: Optional[Kratos.ModelPart] = None
        self.filter: Optional[KratosOA.ElementExplicitFilter] = None
        self.parameters = parameters
        self.optimization_problem = optimization_problem
        self.control_variable_bounds = parameters["control_variable_bounds"].GetVector()

    def Initialize(self) -> None:
        self.primal_model_part = self.primal_model_part_operation.GetModelPart()
        self.adjoint_model_part = self.adjoint_model_part_operation.GetModelPart()

        if not KratosOA.ModelPartUtils.CheckModelPartStatus(self.primal_model_part, "element_specific_properties_created"):
            KratosOA.OptimizationUtils.CreateEntitySpecificPropertiesForContainer(self.primal_model_part, self.primal_model_part.Elements)
            KratosOA.ModelPartUtils.LogModelPartStatus(self.primal_model_part, "element_specific_properties_created")

            if self.primal_model_part != self.adjoint_model_part:
                if KratosOA.ModelPartUtils.CheckModelPartStatus(self.adjoint_model_part, "element_specific_properties_created"):
                    raise RuntimeError(f"Trying to create element specific properties for {self.adjoint_model_part.FullName()} which already has element specific properties.")

                # now assign the properties of primal to the adjoint model parts
                KratosDT.ControlUtils.AssignEquivalentProperties(self.primal_model_part.Elements, self.adjoint_model_part.Elements)
                KratosOA.ModelPartUtils.LogModelPartStatus(self.adjoint_model_part, "element_specific_properties_created")

    def Check(self) -> None:
        pass

    def Finalize(self) -> None:
        pass

    def GetPhysicalKratosVariables(self) -> 'list[SupportedSensitivityFieldVariableTypes]':
        return [self.controlled_physical_variable]

    def GetEmptyField(self) -> ContainerExpressionTypes:
        field = Kratos.Expression.ElementExpression(self.adjoint_model_part)
        Kratos.Expression.LiteralExpressionIO.SetData(field, 0.0)
        return field

    def GetControlField(self) -> ContainerExpressionTypes:
        field = self.GetEmptyField()
        KratosOA.PropertiesVariableExpressionIO.Read(field, self.controlled_physical_variable)
        return field

    def MapGradient(self, physical_gradient_variable_container_expression_map: 'dict[SupportedSensitivityFieldVariableTypes, ContainerExpressionTypes]') -> ContainerExpressionTypes:
        keys = physical_gradient_variable_container_expression_map.keys()
        if len(keys) != 1:
            raise RuntimeError(f"Provided more than required gradient fields for control \"{self.GetName()}\". Following are the variables:\n\t" + "\n\t".join([k.Name() for k in keys]))
        if self.controlled_physical_variable not in keys:
            raise RuntimeError(f"The required gradient for control \"{self.GetName()}\" w.r.t. {self.controlled_physical_variable.Name()} not found. Followings are the variables:\n\t" + "\n\t".join([k.Name() for k in keys]))

        physical_gradient = physical_gradient_variable_container_expression_map[self.controlled_physical_variable]
        if not IsSameContainerExpression(physical_gradient, self.GetEmptyField()):
            raise RuntimeError(f"Gradients for the required element container not found for control \"{self.GetName()}\". [ required model part name: {self.adjoint_model_part.FullName()}, given model part name: {physical_gradient.GetModelPart().FullName()} ]")

        return self.__GetFilter().FilterIntegratedField(physical_gradient_variable_container_expression_map[self.controlled_physical_variable])

    def Update(self, control_field: ContainerExpressionTypes) -> bool:
        if not IsSameContainerExpression(control_field, self.GetEmptyField()):
            raise RuntimeError(f"Updates for the required element container not found for control \"{self.GetName()}\". [ required model part name: {self.adjoint_model_part.FullName()}, given model part name: {control_field.GetModelPart().FullName()} ]")

        # first clip the control field to max and mins
        clipped_control_field = control_field.Clone()
        KratosDT.ControlUtils.ClipContainerExpression(clipped_control_field, self.control_variable_bounds[0], self.control_variable_bounds[1])
        filtered_control_field = self.__GetFilter().FilterField(clipped_control_field)
        unbuffered_data = ComponentDataView(self, self.optimization_problem).GetUnBufferedData()
        unbuffered_data.SetValue("filtered_control_field", filtered_control_field.Clone(), overwrite=True)
        KratosOA.PropertiesVariableExpressionIO.Write(filtered_control_field, self.controlled_physical_variable)
        return True

    def __GetFilter(self) -> KratosOA.ElementExplicitFilter:
        if self.filter is None:
            filtering_params = self.parameters["filtering"]
            fixed_model_part_name = filtering_params["fixed_model_part_name"].GetString()
            if fixed_model_part_name != "":
                self.filter = KratosOA.ElementExplicitFilter(
                                        self.adjoint_model_part,
                                        self.model[fixed_model_part_name],
                                        filtering_params["filter_function_type"].GetString(),
                                        filtering_params["damping_function_type"].GetString(),
                                        filtering_params["max_nodes_in_filter_radius"].GetInt())
            else:
                self.filter = KratosOA.ElementExplicitFilter(
                                        self.adjoint_model_part,
                                        filtering_params["filter_function_type"].GetString(),
                                        filtering_params["max_nodes_in_filter_radius"].GetInt())

            filter_radius_expression = Kratos.Expression.ElementExpression(self.adjoint_model_part)
            Kratos.Expression.LiteralExpressionIO.SetData(filter_radius_expression, filtering_params["filter_radius"].GetDouble())
            self.filter.SetFilterRadius(filter_radius_expression)

        return self.filter

    def __str__(self) -> str:
        return f"Control [type = {self.__class__.__name__}, name = {self.GetName()}, model part name = {self.adjoint_model_part_operation.GetModelPartFullName()}, control variable = {self.controlled_physical_variable.Name()}"
