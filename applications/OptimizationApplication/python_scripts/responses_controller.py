# ==============================================================================
#  KratosOptimizationApplication
#
#  License:         BSD License
#                   license: OptimizationApplication/license.txt
#
#  Main authors:    Reza Najian Asl, https://github.com/RezaNajian
#
# ==============================================================================

# Making KratosMultiphysics backward compatible with python 2.6 and 2.7
from __future__ import print_function, absolute_import, division

# additional imports
# Kratos Core and Apps
import KratosMultiphysics as KM
from KratosMultiphysics import Parameters, Logger
# Importing the analysis base class
from KratosMultiphysics.analysis_stage import AnalysisStage

# Additional imports
import KratosMultiphysics.OptimizationApplication.responses.response_function_factory as response_function_factory

import time as timer

# ==============================================================================
def CreateController(reponses_settings,model,analyses_controller):
    return ResponsesController(reponses_settings,model,analyses_controller)

# ==============================================================================
class ResponsesController:
    # --------------------------------------------------------------------------
    def __init__(self,reponses_settings,model,analyses_controller):
        
        self.reponses_settings = reponses_settings
        self.analyses_controller = analyses_controller
        self.model = model

        default_settings = KM.Parameters("""
        {
            "name"                : "REPONSE_NAME",
            "type"                : "RESPONSE_TYPE",
            "settings"                : {
                "analysis_name"   : "RESPONSE_ANALYZER_NAME",
                "evaluate_model_parts": [],
                "design_types": [],
                "design_model_parts": [],
                "gradient_settings" : {}
            }
        }""")

        for itr in range(self.reponses_settings.size()):
            for key in default_settings.keys():
                if not self.reponses_settings[itr].Has(key):
                    raise RuntimeError("ResponsesController: Required entry '{}' missing in 'response Nr.{}'!".format(key,itr+1))  
            self.reponses_settings[itr].ValidateAndAssignDefaults(default_settings)

            for key in default_settings["settings"].keys():
                if key == "gradient_settings" or key == "analysis_name":
                    continue
                if not self.reponses_settings[itr]["settings"].Has(key):
                    raise RuntimeError("ResponsesController: Required entry '{}' missing in settings of 'response Nr.{}' !".format(key,itr+1))             
            self.reponses_settings[itr]["settings"].ValidateAndAssignDefaults(default_settings["settings"])  


        self.responses = {}

        for itr in range(self.reponses_settings.size()):
            response_settings = self.reponses_settings[itr]
            response_name = response_settings["name"].GetString()            
            response_type = response_settings["type"].GetString()
            response_analysis = None
            response_analysis_model_part = None
            # check for analysis
            if response_settings["settings"].Has("analysis_name"):
                response_analysis_name = response_settings["settings"]["analysis_name"].GetString()
                response_analysis = analyses_controller.GetAnalysis(response_analysis_name)
                response_analysis_model_part = analyses_controller.GetAnalysisModelPart(response_analysis_name)  

            # check for analysis                      

            response = response_function_factory.CreateResponseFunction(response_type,response_settings["settings"],response_analysis,response_analysis_model_part,model)
            self.responses[response_name] = response


    # --------------------------------------------------------------------------
    def Initialize(self):
        for response in self.responses.values():
            response.Initialize()

    # --------------------------------------------------------------------------
    def CalculateResponse(self,response_name):
        if not response_name in self.responses.keys():
            raise RuntimeError("ResponsesController: Try to calculate response {} which does not exist.".format(response_name))
        else:
            Logger.PrintInfo("ResponsesController", " Starting calculate response ", response_name)
            startTime = timer.time()
            response_value = self.responses[response_name].CalculateValue()
            Logger.PrintInfo("ResponsesController", "Time needed for response calculation ",round(timer.time() - startTime,2),"s") 
        return response_value

    # --------------------------------------------------------------------------
    def CalculateAll(self):
        responses_values = []
        for name,response in self.responses.items():
            Logger.PrintInfo("ResponsesController", " Starting calculate response ", name)
            startTime = timer.time()            
            response_value = response.CalculateValue()
            responses_values.append(response_value)
            Logger.PrintInfo("ResponsesController", "Time needed for response calculation ",round(timer.time() - startTime,2),"s")
        return responses_values





            

               

