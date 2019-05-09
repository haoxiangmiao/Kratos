from __future__ import print_function, absolute_import, division  # makes KratosMultiphysics backward compatible with python 2.6 and 2.7

# Importing the Kratos Library
import KratosMultiphysics as KM

# Import applications
import KratosMultiphysics.MeshMovingApplication as KratosMeshMoving

# Import baseclass
from KratosMultiphysics.MeshMovingApplication.mesh_solver_base import MeshSolverBase


def CreateSolver(model, custom_settings):
    return MeshSolverLaplacian(model, custom_settings)


class MeshSolverLaplacian(MeshSolverBase):
    def __init__(self, model, custom_settings):
        super(MeshSolverLaplacian, self).__init__(model, custom_settings)
        if custom_settings["buffer_size"].GetInt() < 2:
            raise Exception("A buffer_size of at least 2 is required!")
        KM.Logger.PrintInfo("::[MeshSolverLaplacian]:: Construction finished")

    @classmethod
    def GetDefaultSettings(cls):
        this_defaults = KM.Parameters("""{
            "buffer_size"           : 2
        }""")
        this_defaults.AddMissingParameters(super(MeshSolverLaplacian, cls).GetDefaultSettings())
        return this_defaults

    def _create_mesh_motion_solving_strategy(self):
        linear_solver = self.get_linear_solver()
        time_order = self.settings["time_order"].GetInt()
        reform_dofs_each_step = self.settings["reform_dofs_each_step"].GetBool()
        compute_reactions = self.settings["compute_reactions"].GetBool()
        calculate_mesh_velocities = self.settings["calculate_mesh_velocities"].GetBool()
        echo_level = self.settings["echo_level"].GetInt()
        solving_strategy = KratosMeshMoving.LaplacianMeshMovingStrategy(self.mesh_model_part,
                                                            linear_solver,
                                                            time_order,
                                                            reform_dofs_each_step,
                                                            compute_reactions,
                                                             calculate_mesh_velocities,
                                                            echo_level)
        return solving_strategy



