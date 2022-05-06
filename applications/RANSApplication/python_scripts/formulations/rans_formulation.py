"""!@package RANSApplication"""

import KratosMultiphysics as Kratos
import KratosMultiphysics.RANSApplication as KratosRANS

##!@addtogroup RANSApplication
##!@{
##!@name Kratos classes
##!@{
class RansFormulation:
    """!@brief RansFormulation base class
    @details This class is the base class for formulations used in RANSApplication. A single leaf formulation
    is responsible for solving for one variable only. RansFormulations can be added to another RansFormulation
    creating coupled hierarchical formulation. Then this base RansFormulations solves them recursively according
    to the addition order.
    """

    def __init__(self, base_computing_model_part: Kratos.ModelPart, settings: Kratos.Parameters):
        """!
        @param base_computing_model_part: Base model part, which is copied to create solvers for given formulation
        @param settings: Settings to be used in this formulation
        """
        self.__settings = settings
        self.__base_computing_model_part = base_computing_model_part
        self.__list_of_formulations = []
        self.__list_of_processes = []
        self.__move_mesh = False

    def GetParameters(self) -> Kratos.Parameters:
        """!@brief Returns parameters used in this formulation
        @return Parameters of this formulation
        """
        return self.__settings

    def GetDomainSize(self) -> int:
        """!@brief Returns domain size
        """
        return self.__base_computing_model_part.ProcessInfo[Kratos.DOMAIN_SIZE]

    def AddRansFormulation(self, formulation: "RansFormulation") -> None:
        """!Adds another RansFormulation to current formulation creating a list of formulations.
        @param formulation: Formulation to be added
        """
        if (isinstance(formulation, RansFormulation)):
            self.__list_of_formulations.append(formulation)
        else:
            msg = str(formulation).rstrip() + " is not a RansFormulation. Please use only RansFormulation objects."
            raise Exception(msg)

    def AddProcess(self, process: KratosRANS.RansFormulationProcess) -> None:
        """!Adds a RansFormulationProcess to the current RansFormulation
        @param process: RansFormulationProcess to be added to current formulation
        """
        if (isinstance(process, KratosRANS.RansFormulationProcess)):
            self.__list_of_processes.append(process)
        else:
            msg = str(process).rstrip() + " is not a RansFormulationProcess. Please use only RansFormulationProcess objects."
            raise Exception(msg)

    def AddVariables(self) -> None:
        """Recursively calls AddVariables methods of existing formulations in this formulaton
        """
        self.__ExecuteRansFormulationMethods("AddVariables")

    def AddDofs(self) -> None:
        """Recursively calls AddDofs methods of existing formulations in this formulaton
        """
        self.__ExecuteRansFormulationMethods("AddDofs")

    def PrepareModelPart(self) -> None:
        """Recursively calls PrepareModelPart methods of existing formulations in this formulaton
        """
        self.__ExecuteRansFormulationMethods("PrepareModelPart")

    def Clear(self) -> None:
        """Recursively calls Clear methods of existing formulations in this formulaton and clears strategy
        """
        if (self.GetStrategy() is not None):
            self.GetStrategy().Clear()

        self.__ExecuteRansFormulationMethods("Clear")

    def Check(self) -> None:
        """Recursively calls Check methods of existing formulations, processes and strategy in this formulaton
        """
        self.__ExecuteProcessMethods("Check")
        self.__ExecuteRansFormulationMethods("Check")

        if (self.GetStrategy() is not None):
            self.GetStrategy().Check()

    def Initialize(self) -> None:
        """Recursively calls Initialize methods of existing formulations, processes and strategy in this formulaton
        """
        self.__ExecuteProcessMethods("ExecuteInitialize")
        self.__ExecuteRansFormulationMethods("Initialize")

        if (self.GetStrategy() is not None):
            self.GetStrategy().Initialize()

    def InitializeSolutionStep(self) -> None:
        """Recursively calls InitializeSolutionStep methods of existing formulations, processes and strategy in this formulaton
        """
        self.__ExecuteProcessMethods("ExecuteInitializeSolutionStep")
        self.__ExecuteRansFormulationMethods("InitializeSolutionStep")

        if (self.GetStrategy() is not None):
            self.GetStrategy().InitializeSolutionStep()

    def SolveCouplingStep(self) -> None:
        """!@brief Solves current formulation
        @details This method recursively solves each formulation in the list of formulations.
        @return True if solve is successfull, False if not
        """
        max_iterations = self.GetMaxCouplingIterations()
        for iteration in range(max_iterations):
            self.ExecuteBeforeCouplingSolveStep()
            for formulation in self.__list_of_formulations:
                if (not formulation.SolveCouplingStep()):
                    return False
            self.ExecuteAfterCouplingSolveStep()
            Kratos.Logger.PrintInfo(self.__class__.__name__, "Solved coupling itr. " + str(iteration + 1) + "/" + str(max_iterations) + ".")

        return True

    def ExecuteBeforeCouplingSolveStep(self) -> None:
        """Recursively calls ExecuteBeforeCouplingSolveStep methods of existing formulations in this formulaton
        """
        self.__ExecuteProcessMethods("ExecuteBeforeCouplingSolveStep")

    def ExecuteAfterCouplingSolveStep(self) -> None:
        """Recursively calls ExecuteAfterCouplingSolveStep methods of existing formulations in this formulaton
        """
        self.__ExecuteProcessMethods("ExecuteAfterCouplingSolveStep")

    def FinalizeSolutionStep(self) -> None:
        """Recursively calls FinalizeSolutionStep methods of existing formulations, processes and strategy in this formulaton
        """
        if (self.GetStrategy() is not None):
            self.GetStrategy().FinalizeSolutionStep()

        self.__ExecuteRansFormulationMethods("FinalizeSolutionStep")
        self.__ExecuteProcessMethods("ExecuteFinalizeSolutionStep")

    def Finalize(self) -> None:
        """Recursively calls Finalize methods of existing formulations and processes in this formulaton
        """
        self.__ExecuteRansFormulationMethods("Finalize")
        self.__ExecuteProcessMethods("ExecuteFinalize")

        if (self.GetStrategy() is not None):
            self.GetStrategy().Clear()

    def GetMinimumBufferSize(self) -> int:
        """Recursively calculate minimum buffer size required by all formulations
        @return Minimum buffer size
        """
        min_buffer_size = 0
        for formulation in self.__list_of_formulations:
            if (min_buffer_size < formulation.GetMinimumBufferSize()):
                min_buffer_size = formulation.GetMinimumBufferSize()
        return min_buffer_size

    def IsBufferInitialized(self) -> bool:
        """Check whether enough buffer is initialized to solve current formulation and its child formulations
        @return True if enough steps are initialized, False otherwise
        """
        return (self.GetBaseModelPart().ProcessInfo[Kratos.STEP] + 1 >=
                self.GetMinimumBufferSize())

    def IsConverged(self) -> bool:
        """Recursively checks whether all formulations are converged.
        @return True if all of them have converged, False if not
        """
        for formulation in self.__list_of_formulations:
            if (not formulation.IsConverged()):
                return False

        if (self.GetStrategy() is not None):
            is_converged = self.GetStrategy().IsConverged()
            if (is_converged):
                Kratos.Logger.PrintInfo(self.__class__.__name__, " *** CONVERGENCE ACHIEVED ***")
            return is_converged

        return True

    def GetMoveMeshFlag(self) -> bool:
        """Returns move mesh flag
        @return True if mesh move, False if not
        """
        return self.__move_mesh

    def SetCommunicator(self, communicator: Kratos.Communicator) -> None:
        """Sets the communicator for MPI use
        """
        self.__ExecuteRansFormulationMethods("SetCommunicator", [communicator])
        self.__communicator = communicator

    def GetCommunicator(self) -> Kratos.Communicator:
        """Get the communicator for MPI use
        @return Communicator used in the model part
        """
        if hasattr(self, "_RansFormulation__communicator"):
            return self.__communicator
        else:
            raise Exception(self.__class__.__name__ + " needs to use \"SetCommunicator\" first before retrieving.")

    def IsPeriodic(self) -> bool:
        """Checks whether current formulations are solved with periodic conditions
        @return True if Periodic, False if not
        """
        if hasattr(self, "_RansFormulation__is_periodic"):
            return self.__is_periodic
        else:
            raise Exception(self.__class__.__name__ + " needs to use \"SetIsPeriodic\" first before checking.")

    def SetIsPeriodic(self, value: bool) -> None:
        """Sets periodicity recursively for all formulations
        @param value True if formulations need to be Periodic, False otherwise
        """
        self.__ExecuteRansFormulationMethods("SetIsPeriodic", [value])
        self.__is_periodic = value

    def SetTimeSchemeSettings(self, settings: Kratos.Parameters) -> None:
        """Sets time scheme settings recursively for all formulations
        @param settings: Time scheme settings
        """
        self.__ExecuteRansFormulationMethods("SetTimeSchemeSettings", [settings])
        self.__time_scheme_settings = settings

    def GetTimeSchemeSettings(self) -> Kratos.Parameters:
        """Returns time scheme settings
        @return Time scheme settings used for formulations
        """
        if (hasattr(self, "_RansFormulation__time_scheme_settings")):
            return self.__time_scheme_settings
        else:
            raise Exception(self.__class__.__name__ + " needs to use \"SetTimeSchemeSettings\" first before calling \"GetTimeSchemeSettings\".")

    def SetWallFunctionSettings(self, settings: Kratos.Parameters) -> None:
        self.__ExecuteRansFormulationMethods("SetWallFunctionSettings", [settings])
        self.__wall_function_settings = settings

    def GetWallFunctionSettings(self) -> Kratos.Parameters:
        """Returns wall function settings
        @return Wall function settings used for formulations
        """
        if (hasattr(self, "_RansFormulation__wall_function_settings")):
            return self.__wall_function_settings
        else:
            raise Exception(self.__class__.__name__ + " needs to use \"SetWallFunctionSettings\" first before calling \"GetWallFunctionSettings\".")

    def GetBaseModelPart(self) -> Kratos.ModelPart:
        """Returns base model part used in the formulation
        @return Base model part used in the formulation
        """
        return self.__base_computing_model_part

    def GetComputingModelPart(self) -> Kratos.ModelPart:
        """Returns computing model part used in the formulation
        @return Computing model part used in the formulation
        """
        if (self.__base_computing_model_part.HasSubModelPart("fluid_computational_model_part")):
            return self.__base_computing_model_part.GetSubModelPart("fluid_computational_model_part")
        else:
            raise Exception("Computing model part \"fluid_computational_model_part\" not found as a submodel part in " + self.__base_computing_model_part.Name() + ".")

    def SetMaxCouplingIterations(self, max_iterations: int) -> None:
        """!@brief Sets max coupling iterations
        @details This is not done recursively because, there are some formulations which doesn't have coupling iterations.
        Each formulation needs to set this seperately if base class SolveCouplingStep is used.
        @param max_iterations: Maximum number of coupling iterations to be done in the child formulations
        """
        self.__max_coupling_iterations = max_iterations

    def GetMaxCouplingIterations(self) -> int:
        """Returns maxmum number of coupling iterations used in this formulation
        @return Maximum number of coupling iterations
        """
        if (hasattr(self, "_RansFormulation__max_coupling_iterations")):
            return self.__max_coupling_iterations
        else:
            raise Exception(self.__class__.__name__ + " needs to use \"SetMaxCouplingIterations\" first before calling \"GetMaxCouplingIterations\".")

    def SetConstants(self, settings: Kratos.Parameters) -> None:
        """Recursively sets constants in the formulations
        @param settings: Constants settings
        """
        self.__ExecuteRansFormulationMethods("SetConstants", [settings])

    def GetRansFormulationsList(self) -> list["RansFormulation"]:
        """Returns list of formulations in this formulation
        @return List of formulations in this formulation
        """
        return self.__list_of_formulations

    def GetProcessList(self) -> list[KratosRANS.RansFormulationProcess]:
        """Returns list of processes used in this formulation
        @return List of rans formulation processes in this formulation
        """
        return self.__list_of_processes

    def GetModelPart(self) -> Kratos.ModelPart:
        """Returns the model part used for solving current formulation (if a strategy is used only.)
        @return Model part used for solving current formulation
        """

        if (self.GetStrategy() is not None):
            return self.GetStrategy().GetModelPart()
        return None

    def GetStrategy(self) -> None:
        """Returns strategy used in this formulation, if used any.
        @return Strategy used in this formulation, None if not used.
        """
        return None

    def GetInfo(self) -> str:
        """Recursively identify formulations being used.
        @return Information of all the formulations
        """
        info = "\n" + self.__class__.__name__
        if (self.GetModelPart() is not None):
            info += "\n   Model part    : " + str(self.GetModelPart().Name)

        if (self.GetMaxCouplingIterations() != 0):
            info += "\n   Max iterations: " + str(self.GetMaxCouplingIterations())

        if (len(self.GetProcessList()) > 0):
            info += "\n   Process list:"
            for process in self.GetProcessList():
                info += "\n      " + str(process).strip()

        for formulation in self.GetRansFormulationsList():
            info += str(formulation.GetInfo()).replace("\n", "\n   ")
        return info

    def __ExecuteRansFormulationMethods(self, method_name, args = []):
        for formulation in self.__list_of_formulations:
            getattr(formulation, method_name)(*args)

    def __ExecuteProcessMethods(self, method_name):
        for process in self.__list_of_processes:
            getattr(process, method_name)()
##!@}
##!@}
