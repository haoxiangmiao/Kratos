import sys
import importlib

import KratosMultiphysics
from KratosMultiphysics.project import Project
from KratosMultiphysics.model_parameters_factory import KratosModelParametersFactory

class MultistageOrchestrator():

    def __init__(self, settings : KratosMultiphysics.Parameters) -> None:
        """Base class for multistage orchestrators
        
        Member variables:
        settings -- Kratos parameters dictionary encapsulating the multistage settings
        project -- Kratos project instance
        """

        # Store required member variables
        self.settings = settings
        self.project = Project()

        # Add Kratos version and compilation to settings
        kratos_version = f"{KratosMultiphysics.KratosGlobals.Kernel.Version()}-{KratosMultiphysics.KratosGlobals.Kernel.BuildType()}"
        self.settings.AddString("kratos_version", kratos_version)

    def Run(self):
        """Main function that runs the complete multistage simulation."""

        err_msg = "Calling base MultistageOrchestrator Run() method. This must be implemented in derived orchestrators."
        raise NotImplementedError (err_msg)

    def CheckStageSettings(self, stage_name : str):
        """Check the settings for the given stage name
        
        This methods performs the check of the given stage name settings.
        Note that the Check() method of the stage instance is intentionally not called in here.
        This is due to the fact that the Check() of the stage instance is to be called within its Initialize() method.
        """

        if self.settings["stages"][stage_name].Has("stage_postprocess"):
            if self.settings["stages"][stage_name]["stage_postprocess"].Has("modelers"):
                err_msg = f"Found 'modelers' field in 'stage_postprocess' of stage {stage_name}."
                err_msg += " Place the 'modelers' section in the next stage 'stage_preprocess'."
                raise Exception(err_msg)

    def CreateStage(self, stage_name : str):
        """This method creates a stage instance

        Given a stage name, this method creates the corresponding stage instance from the "analysis_stage" registry entry in the settings.
        The stage instance is to be created from the registry. If not registered, it tries to use "analysis_stage" as Python module.
        """

        # Get the current analysis stage class and module names
        input_analysis_stage = self.settings["stages"][stage_name]["analysis_stage"].GetString()

        # Check if the current stage is to be obtained from the Kratos registry
        if input_analysis_stage.split(".")[0] == "Stages":
            # Import current stage main module to populate the registry
            # This is required as the applications Python stuff is registered when the application is imported first
            # Note that in here we are following the convention that the middle strings are the module name
            main_module_name = '.'.join(input_analysis_stage.split(".")[1:-1])
            if main_module_name == "All":
                err_msg = f"Please provide the 'analysis_stage' registry entry with module name (not the 'All' one) in '{stage_name}'."
                raise ValueError(err_msg)
            try:
                importlib.import_module(main_module_name)
            except ImportError:
                err_msg = f"Unbable to import '{main_module_name}'."

            # Once we have populated the registry we retrieve the analysis stage from it
            # Note that in here we are following the registry convention in @python_registry_utilities.py
            if KratosMultiphysics.Registry.HasItem(input_analysis_stage):
                # Get current analysis data from registry
                analysis_stage_reg_entry = KratosMultiphysics.Registry[input_analysis_stage]
                analysis_stage_class_name = analysis_stage_reg_entry["ClassName"]
                if "ModuleName" in analysis_stage_reg_entry:
                    # Standard case in which the analysis stage is registered in a module
                    analysis_stage_module_name = analysis_stage_reg_entry["ModuleName"]
                else:
                    # Alternative in which only the "ClassName" is provided with no module (e.g. custom user-defined stages in MainKratos.py)
                    analysis_stage_module_name = None
            else:
                err_msg = f"Trying to retrieve a non-registered analysis stage '{input_analysis_stage}'"
                raise ValueError(err_msg)
        else:
            # Throw a warning to let the user know that the current analysis stage is not registered
            warn_msg = f"Current analysis stage '{input_analysis_stage}' is not registered. Assuming that provided string contains module and class names."
            KratosMultiphysics.Logger.PrintWarning(warn_msg)

            # Get the Python module name and class name implementing each analysis stage
            # Note that we assume that the class name is the provided module (file) name in CamelCase
            analysis_stage_module_name = input_analysis_stage
            analysis_stage_class_name = analysis_stage_module_name.split('.')[-1]
            analysis_stage_class_name = ''.join(x.title() for x in analysis_stage_class_name.split('_'))

        # Import the stage module
        # Note that for the case of custom analysis stage (i.e. no Kratos module) we check if the class exists in the __main__ module
        if analysis_stage_module_name is not None:
            analysis_stage_module = importlib.import_module(analysis_stage_module_name)
        else:
            analysis_stage_module = sys.modules["__main__"]
        
        # Create the analysis stage instance
        if hasattr(analysis_stage_module, analysis_stage_class_name):
            # First we check for the expected class name
            analysis_stage_class = getattr(analysis_stage_module, analysis_stage_class_name)
            stage_instance = analysis_stage_class(self.project.GetModel(),  KratosMultiphysics.Parameters(self.settings["stages"][stage_name]))
        elif hasattr(analysis_stage_module, "Create"):
            # If Kratos convention is not fulfilled we search for a Create method
            stage_instance = analysis_stage_module.Create(self.project.GetModel(),  KratosMultiphysics.Parameters(self.settings["stages"][stage_name]))
        else:
            err_msg = f"Analysis stage in '{analysis_stage_module_name}' Python module cannot be created. Please check class name or provide a 'Create' method."
            raise Exception(err_msg)
        
        return stage_instance

    def RunCurrentStagePreprocess(self, stage_name : str, data=None):
        """This function executes the preprocess of current stage.

        Note that the stage preprocess involves the execution of modelers and operations.

        Keyword arguments:
        data -- Custom data argument to be used in derived classes
        """

        if self.settings["stages"][stage_name].Has("stage_preprocess"):
            if self.settings["stages"][stage_name]["stage_preprocess"].Has("modelers"):
                modelers_list = self.__GetModelers(stage_name)
                for modeler in modelers_list:
                    modeler.SetupGeometryModel()
                for modeler in modelers_list:
                    modeler.PrepareGeometryModel()
                for modeler in modelers_list:
                    modeler.SetupModelPart()
                del modelers_list

            if self.settings["stages"][stage_name]["stage_preprocess"].Has("operations"):
                operations_list = self.__GetOperations("stage_preprocess", stage_name)
                for operation in operations_list:
                    operation.Execute()
                del operations_list

    def RunCurrentStagePostprocess(self, stage_name : str, data=None):
        """This function executes the postprocessing of current stage.
        
        Note that the stage postprocess deliberately involves operations only.

        Keyword arguments:
        data -- Custom data argument to be used in derived classes
        """

        if self.settings["stages"][stage_name].Has("stage_postprocess"):
            if self.settings["stages"][stage_name]["stage_postprocess"].Has("operations"):
                operations_list = self.__GetOperations("stage_postprocess", stage_name)
                for operation in operations_list:
                    operation.Execute()
                del operations_list

    def GetProject(self):
        """Returns the project."""

        return self.project
   
    def __GetModelers(self, stage_name : str):
        """This method creates the modelers at the preprocess execution point."""

        execution_point_settings = self.settings["stages"][stage_name]["stage_preprocess"]

        factory = KratosModelParametersFactory(self.project.GetModel())
        return factory.ConstructListOfItems(execution_point_settings["modelers"])

    def __GetOperations(self, execution_point : str, stage_name : str):
        """This method creates the operations at any execution point."""

        if execution_point not in ["stage_preprocess","stage_postprocess"]:
            err_msg = f"Wrong execution point '{execution_point}'. Supported ones are 'stage_preprocess' and 'stage_postprocess'."
            raise Exception(err_msg)
        execution_point_settings = self.settings["stages"][stage_name][execution_point]

        factory = KratosModelParametersFactory(self.project.GetModel())
        return factory.ConstructListOfItems(execution_point_settings["operations"])
