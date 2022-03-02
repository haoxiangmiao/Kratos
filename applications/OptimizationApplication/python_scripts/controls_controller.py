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
import KratosMultiphysics as KM 
# import KratosMultiphysics.OptimizationApplication.controls.shape.explicit_vertex_morphing as evm
import KratosMultiphysics.OptimizationApplication as KOA
import KratosMultiphysics.kratos_utilities as kratos_utilities
import csv, math
import copy

# ==============================================================================
def CreateController(controls_settings,model,model_parts_controller):
    return ControlsController(controls_settings,model,model_parts_controller)

# ==============================================================================
class ControlsController:
    # --------------------------------------------------------------------------
    def __init__(self,controls_settings,model,model_parts_controller):
        
        self.controls_settings = controls_settings
        self.model_parts_controller = model_parts_controller
        self.model = model

        default_settings = KM.Parameters("""
        {
            "name"       : "CONTROL_NAME",
            "type"       : "CONTROL_TYPE",
            "settings"       : {
                "technique"  : "CONTROL_TECHNIQUE",
                "technique_settings"       : {},
                "controlling_objects"  : []             
            }
        }""")


        for itr in range(self.controls_settings.size()):
            for key in default_settings.keys():
                if not self.controls_settings[itr].Has(key):
                    raise RuntimeError("ControlsController: Required setting '{}' missing in 'control Nr.{}'!".format(key,itr+1))  
            self.controls_settings[itr].ValidateAndAssignDefaults(default_settings)
            for key in default_settings["settings"].keys():
                if key == "technique_settings":
                    continue
                if not self.controls_settings[itr]["settings"].Has(key):
                    raise RuntimeError("ControlsController: Required setting '{}' missing in 'settings' of 'control Nr.{}' !".format(key,itr+1))             
            self.controls_settings[itr]["settings"].ValidateAndAssignDefaults(default_settings["settings"])  


        self.supported_control_types_techniques = {"shape":["explicit_vertex_morphing"],"topology":[],"thickness":[]}
        self.supported_control_types_variable_names = {"shape":"CX","thickness":"CT","topology":"CP"}
        # sanity checks
        self.controls_types_vars_dict = {"shape":[],"topology":[],"thickness":[]}
        self.controls = {}
        self.controls_type = {}
        self.controls_controlling_objects = {}
        for itr in range(self.controls_settings.size()):
            control_settings = self.controls_settings[itr]
            
            # check for name
            control_name = control_settings["name"].GetString()
            if  control_name in self.controls.keys():  
                raise RuntimeError("ControlsController: Control name {} already exists.".format(control_name))            
            # check control type
            control_type = control_settings["type"].GetString()
            if not control_type in self.supported_control_types_techniques.keys():
                raise RuntimeError("ControlsController: control type '{}' in control {} is not supported!, we support {} ".format(control_type,control_name,self.supported_control_types_techniques))
            self.controls_type[control_name] = control_type              
            # check control technique
            control_technique = control_settings["settings"]["technique"].GetString()
            if not control_technique in self.supported_control_types_techniques[control_type]:
                raise RuntimeError("ControlsController: control technique '{}' for control type {} in control {} is not supported!, we support {}".format(control_technique,control_type,control_name,self.supported_control_types_techniques))

            # check for repetitious control over variables
            control_controlling_objects_list = control_settings["settings"]["controlling_objects"].GetStringArray()
            control_controlling_objects_list_set = set(control_controlling_objects_list)
            if len(control_controlling_objects_list_set) != len(control_controlling_objects_list):
                raise RuntimeError("ControlsController: control '{}' has duplicated control variables!".format(control_name))
            if control_controlling_objects_list_set.issubset(set(self.controls_types_vars_dict[control_type])):
                raise RuntimeError("ControlsController: there are duplicated {} control over {}!".format(control_type,control_controlling_objects_list))
            if control_name in self.controls.keys():
                raise RuntimeError("ControlsController: control name {} is duplicated !",control_name) 
            self.controls_controlling_objects[control_name] = control_controlling_objects_list

            # now checks passed and create the control
            if control_type == "shape":
                # check if root model parts exist
                self.model_parts_controller.CheckIfRootModelPartsExist(control_controlling_objects_list,True)
                if control_technique == "explicit_vertex_morphing":
                    control = KOA.ExplicitVertexMorphing(control_name,self.model,control_settings["settings"]) 
                    # control = evm.ExplicitVertexMorphing(control_name,model,control_controlling_objects_list,control_settings["settings"]["technique_settings"])                          

            self.controls[control_name] = control
            self.controls_types_vars_dict[control_type].extend(control_controlling_objects_list)
                            
    # --------------------------------------------------------------------------
    def Initialize(self):
        for key,value in self.controls.items():
            value.Initialize()

    # --------------------------------------------------------------------------
    def ControlMapFirstDerivative(self,control_name,derivative_name,mapped_derivative_name,raise_error=True):
        if raise_error:
            if not control_name in self.controls.keys():
                raise RuntimeError("ControlsController:ControlMapFirstDerivative: Control {} does not exist.".format(control_name))

        self.controls[control_name].MapFirstDerivative(derivative_name,mapped_derivative_name)

    # --------------------------------------------------------------------------
    def CheckIfControlExists(self,control_name,raise_error=True):
        if not control_name in self.controls.keys():
            if raise_error:
                raise RuntimeError("ControlsController:CheckIfControlExists: Control {} does not exist.".format(control_name))
            else: 
                return False
        else:
            return True
    # --------------------------------------------------------------------------
    def CheckIfControlsExist(self,controls_name,raise_error=True):
        if type(controls_name) is not list:
            raise RuntimeError("ControlsController:CheckIfControlsExist requires list of control names")
        
        if_exist = True
        for control_name in controls_name:
            if not control_name in self.controls.keys():
                if raise_error:
                    raise RuntimeError("ControlsController:CheckIfControlsExist: Control {} does not exist!".format(control_name))
                else:
                    if_exist = False
                    break
        return if_exist      

    # --------------------------------------------------------------------------
    def GetControlType(self,control_name,raise_error=True):
        if not control_name in self.controls_type.keys():
            if raise_error:
                raise RuntimeError("ControlsController:GetControlType: Control {} does not exist.".format(control_name))
        else:
            return self.controls_type[control_name]           
    # --------------------------------------------------------------------------
    def GetControlControllingObjects(self,control_name,raise_error=True):
        if raise_error:
            if not control_name in self.controls_controlling_objects.keys():
                raise RuntimeError("ControlsController:GetControlControllingObjects: Control {} does not exist.".format(control_name))
       
        return self.controls_controlling_objects[control_name]                   

    # --------------------------------------------------------------------------
    def GetSupportedControlTypesVariablesName(self):
        return self.supported_control_types_variable_names

    # --------------------------------------------------------------------------
    def MapControlFirstDerivative(self, control_name, derivative_variable_name, mapped_derivative_variable_name, raise_error=True):   
        if raise_error:
            if not control_name in self.controls_controlling_objects.keys():
                raise RuntimeError("ControlsController:MapControlFirstDerivative: Control {} does not exist.".format(control_name)) 

        return self.controls[control_name].MapFirstDerivative(derivative_variable_name,mapped_derivative_variable_name)       