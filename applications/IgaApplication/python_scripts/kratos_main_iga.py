import KratosMultiphysics
from KratosMultiphysics.IgaApplication.iga_analysis import IgaAnalysis

"""
For user-scripting it is intended that a new class is derived
from IgaAnalysis to do modifications
"""

if __name__ == "__main__":

    with open("ProjectParameters.json",'r') as parameter_file:
        parameters = KratosMultiphysics.Parameters(parameter_file.read())

    model = KratosMultiphysics.Model()
    simulation = IgaAnalysis(model,parameters)
    simulation.Run()
