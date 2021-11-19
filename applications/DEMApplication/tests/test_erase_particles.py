import os
import KratosMultiphysics as Kratos
from Kratos import Logger
import KratosMultiphysics.KratosUnittest as KratosUnittest

import KratosMultiphysics.DEMApplication.DEM_analysis_stage as dem_analysis

# This test consists in a system with a single already existing particle and an inlet that injects a few
# particles during the simulation, which consists in letting the particle fall under gravity.
# The bounding box, which has its bottom placed at z=0 is set to mark the particles to be erased when they
# cross this limit. Depending on the delay imposed on the destruction of the particles after they are marked,
# a different number of particles is recovered at the end of the simulation (more delay should lead
# to equal ot greater number of particles at the end).

debug_mode = True


class TestDEMEraseParticles(dem_analysis.DEMAnalysisStage):
    @classmethod
    def GetMainPath(self):
        return os.path.join(os.path.dirname(os.path.realpath(__file__)), "erase_particles_test_files")

    def Finalize(self):
        self.number_of_particles_by_the_end = len(self.spheres_model_part.GetElements())
        parent_return = super().Finalize()

class TestTestDEMEraseParticlesWithNoDelay(KratosUnittest.TestCase):
    @staticmethod
    def GetExpectedNumberOfParticlesByTheEnd():
        return 27

    def setUp(self):
        self.parameters_file_name = 'ProjectParametersDEMWithNoDelay.json'
        self.path = TestDEMEraseParticles.GetMainPath()
        self.analysis = TestDEMEraseParticles

    def test_piecewise_linear_inlet(self):
        print(self.parameters_file_name)
        parameters_file_path = os.path.join(self.path, self.parameters_file_name)
        model = Kratos.Model()

        with open(parameters_file_path, 'r') as parameter_file:
            project_parameters = Kratos.Parameters(parameter_file.read())

        analysis = self.analysis(model, project_parameters)
        analysis.Run()
        self.assertEqual(type(self).GetExpectedNumberOfParticlesByTheEnd(), analysis.number_of_particles_by_the_end)


class TestTestDEMEraseParticlesWithLittleDelay(TestTestDEMEraseParticlesWithNoDelay):
    def setUp(self):
        super().setUp()
        self.parameters_file_name = 'ProjectParametersDEMWithLittleDelay.json'

class TestTestDEMEraseParticlesWithDelay(TestTestDEMEraseParticlesWithNoDelay):
    @staticmethod
    def GetExpectedNumberOfParticlesByTheEnd():
        return 29

    def setUp(self):
        super().setUp()
        self.parameters_file_name = 'ProjectParametersDEMWithDelay.json'

if __name__ == "__main__":
    if debug_mode:
        Logger.GetDefaultOutput().SetSeverity(Logger.Severity.INFO)
    else:
        Logger.GetDefaultOutput().SetSeverity(Logger.Severity.WARNING)

    KratosUnittest.main()
