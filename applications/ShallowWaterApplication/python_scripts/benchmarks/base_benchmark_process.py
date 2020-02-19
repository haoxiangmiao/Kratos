# Importing the Kratos Library
import KratosMultiphysics as KM
import KratosMultiphysics.ShallowWaterApplication as SW

def Factory(settings, model):
    if not isinstance(settings, KM.Parameters):
        raise Exception("expected input shall be a Parameters object, encapsulating a json string")
    return BaseBenchmarkProcess(model, settings["Parameters"])

''' The base process for shallow water benchmarks '''
class BaseBenchmarkProcess(KM.Process):

    def __init__(self, model, settings ):

        default_settings = KM.Parameters("""
            {
                "model_part_name"    : "model_part",
                "error_variable"     : "ERROR_RATIO",
                "degree_of_freedom"  : "HEIGHT",
                "benchmark_settings" : {}
            }
            """
            )

        settings.ValidateAndAssignDefaults(default_settings)

        self.model_part = model[settings["model_part_name"].GetString()]

        self.variable = KM.KratosGlobals.GetVariable(settings["error_variable"].GetString())
        self.degree_of_freedom = settings["degree_of_freedom"].GetString()
        self.benchmark_settings = settings["benchmark_settings"]

    def ExecuteInitialize(self):
        pass

    def ExecuteBeforeSolutionLoop(self):
        pass

    def ExecuteInitializeSolutionStep(self):
        pass

    def ExecuteFinalizeSolutionStep(self):
        time = self.model_part.ProcessInfo[KM.TIME]

        for node in self.model_part.Nodes:
            if self.degree_of_freedom == "HEIGHT":
                node.SetValue(self.variable, node.GetSolutionStepValue(SW.HEIGHT) - self.Height(node, time))

            elif self.degree_of_freedom == "VELOCITY_X":
                node.SetValue(self.variable, node.GetSolutionStepValue(SW.VELOCITY_X) - self.Velocity(node, time)[0])

            elif self.degree_of_freedom == "VELOCITY_Y":
                node.SetValue(self.variable, node.GetSolutionStepValue(SW.VELOCITY_Y) - self.Velocity(node, time)[1])

            elif self.degree_of_freedom == "MOMENTUM_X":
                node.SetValue(self.variable, node.GetSolutionStepValue(SW.MOMENTUM_X) - self.Momentum(node, time)[0])

            elif self.degree_of_freedom == "MOMENTUM_Y":
                node.SetValue(self.variable, node.GetSolutionStepValue(SW.MOMENTUM_Y) - self.Momentum(node, time)[1])

    def ExecuteBeforeOutputStep(self):
        pass

    def ExecuteAfterOutputStep(self):
        pass

    def ExecuteFinalize(self):
        pass

    def Check(self):
        return 1

    def Height(self, coordinates, time):
        raise Exception("Calling the base class of the benchmark. Please, implement the custom benchmark")
        return 0.0

    def Velocity(self, coordinates, time):
        raise Exception("Calling the base class of the benchmark. Please, implement the custom benchmark")
        return [0.0, 0.0, 0.0]

    def Momentum(self, coordinates, time):
        raise Exception("Calling the base class of the benchmark. Please, implement the custom benchmark")
        return [0.0, 0.0, 0.0]
