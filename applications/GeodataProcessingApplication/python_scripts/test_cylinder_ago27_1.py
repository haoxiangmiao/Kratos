""" cylindrical domain
	# create:	27 August 2019

	[ITA]
	# fare il punto della situazione sull'applicazione
	# verranno fatti vari test per vedere cosa funziona e cosa no
	# """


import KratosMultiphysics as Kratos
import KratosMultiphysics.GeodataProcessingApplication as KratosGeo
import KratosMultiphysics.FluidDynamicsApplication

from geo_importer import GeoImporter
from geo_mesher import GeoMesher
from geo_preprocessor import GeoPreprocessor
from geo_building import GeoBuilding

import time
import os

start_time = time.time()
num_test = "august_01"
print("\n\nTEST ", num_test, "\n\n")

# we create a new folders for this test
if not os.path.exists("cfd_data/test_{}".format(num_test)):
	os.mkdir("cfd_data/test_{}".format(num_test))
else:
	# to overwrite the folder or not
	if (input("Overwrite the folder? (True / False): ")):
		import shutil
		shutil.rmtree("cfd_data/test_{}".format(num_test))
		os.mkdir("cfd_data/test_{}".format(num_test))
		print("\n*** Folder test_{} overwriten! ***\n".format(num_test))

if not os.path.exists("cfd_data/test_{}/gid_file".format(num_test)):
	os.mkdir("cfd_data/test_{}/gid_file".format(num_test))
if not os.path.exists("cfd_data/test_{}/stl_file".format(num_test)):
	os.mkdir("cfd_data/test_{}/stl_file".format(num_test))
# if not os.path.exists("cfd_data/test_{}/analysis_file".format(num_test)):
# 	os.mkdir("cfd_data/test_{}/analysis_file".format(num_test))
if not os.path.exists("cfd_data/test_{}/mdpa_file".format(num_test)):
	os.mkdir("cfd_data/test_{}/mdpa_file".format(num_test))

# import terrain
preprocessor = GeoPreprocessor()
preprocessor.ReadSTL("data/terrain/terrain_2.stl")

X = []; Y = []
for coord in preprocessor.point_list:
	X.append(coord[0])
	Y.append(coord[1])

x_shift = max(X)/2
y_shift = max(Y)/2
preprocessor.Shift(-x_shift, -y_shift)

# we extract the surface in the top of the terrain
preprocessor.ExtractMountain(1.0)

STL_name = "cfd_data/test_{}/stl_file/terrain_shift.stl".format(num_test)
preprocessor.WriteSTL(STL_name)

importer = GeoImporter()
importer.StlImport(STL_name)
terrain_model_part = importer.GetGeoModelPart()

mesher = GeoMesher()
mesher.SetGeoModelPart(terrain_model_part)

# we cut a circular portion of the terrain, we perform smoothing procedure and we compute the volume mesh
height = 100.0		# height = 80.0
elem_list = mesher.MeshCircleWithTerrainPoints(height, 60, True)	# if the value is True we can save a list with the elements that are inside r_buildings

##################################################################################################
STL_center_name = "cfd_data/test_{}/stl_file/terrain_center.stl".format(num_test)
preprocessor.WriteSTL(STL_center_name, elem_list)
importer.StlImport(STL_center_name)
importer.CreateGidControlOutput("cfd_data/test_{}/gid_file/terrain_center".format(num_test))
# input(importer.ModelPart)
##################################################################################################

terrain_model_part = mesher.GetGeoModelPart()
mesher.CreateGidControlOutput("cfd_data/test_{}/gid_file/01_Mesh_cylinder".format(num_test))


### MMG ###
# 1st ground refinement
print("\n\n***** 1st ground refinement *****\n")
# distance field from ground
mesher.ComputeDistanceFieldFromGround()
mesher.CreateGidControlOutput("cfd_data/test_{}/gid_file/02_Mesh_cylinder_distance_field_1".format(num_test))
# writing file mdpa
mdpa_out_name = "cfd_data/test_{}/mdpa_file/02_Mesh_cylinder_distance_field_1".format(num_test)
mesher.WriteMdpaOutput(mdpa_out_name)
# refine mesh
# mesher.RefineMesh_test(5.0, 10.0)
mesher.RefineMesh_test(10.0, 100.0)
mesher.CreateGidControlOutput("cfd_data/test_{}/gid_file/03_Mesh_cylinder_refinement_1".format(num_test))
# writing file mdpa
mdpa_out_name = "cfd_data/test_{}/mdpa_file/03_Mesh_cylinder_refinement_1".format(num_test)
mesher.WriteMdpaOutput(mdpa_out_name)
print("BOX REFINEMENT 1 DONE!")

input("PAUSE")


''' vecchia parte '''
# import terrain
preprocessor = GeoPreprocessor()
importer = GeoImporter()
importer.CreateBoxTetra("Volume", -10, 10, -10, 10, 0.0, 20)

main_model = importer.GetGeoModelPart()
bottom_model_part = main_model.CreateSubModelPart("BottomModelPart")
bottom_model_part.AddNode(main_model.GetNode(1), 0)
bottom_model_part.AddNode(main_model.GetNode(2), 0)
bottom_model_part.AddNode(main_model.GetNode(3), 0)
bottom_model_part.AddNode(main_model.GetNode(4), 0)
bottom_model_part.AddCondition(main_model.GetCondition(1))
bottom_model_part.AddCondition(main_model.GetCondition(2))

# we fill point_list
for node in importer.GetGeoModelPart().Nodes:
	preprocessor.point_list.append([node.X, node.Y, node.Z])

# we extract the surface in the top of the terrain
preprocessor.ExtractMountain(1.0)

STL_name = "cfd_data/test_{}/stl_file/terrain_shift.stl".format(num_test)
preprocessor.WriteSTL(STL_name)

importer.StlImport(STL_name)
terrain_model_part = importer.GetGeoModelPart()

mesher = GeoMesher()
mesher.SetGeoModelPart(terrain_model_part)

# we cut a circular portion of the terrain, we perform smoothing procedure and we compute the volume mesh
height = 20.0		# height = 80.0
elem_list = mesher.MeshCircleWithTerrainPoints(height, 10, True)	# if the value is True we can save a list with the elements that are inside r_buildings

terrain_model_part = mesher.GetGeoModelPart()
mesher.CreateGidControlOutput("cfd_data/test_{}/gid_file/01_Mesh_cylinder".format(num_test))

input("01_Mesh_cylinder DONE!")

### MMG ###
# 1st ground refinement
print("\n\n***** 1st ground refinement *****\n")
# distance field from ground
mesher.ComputeDistanceFieldFromGround()
mesher.CreateGidControlOutput("cfd_data/test_{}/gid_file/02_Mesh_cylinder_distance_field_1".format(num_test))
# writing file mdpa
mdpa_out_name = "cfd_data/test_{}/mdpa_file/02_Mesh_cylinder_distance_field_1".format(num_test)
mesher.WriteMdpaOutput(mdpa_out_name)

# refine mesh
# mesher.RefineMeshNearGround(10.0)
# mesher.CreateGidControlOutput("cfd_data/test_{}/gid_file/03_Mesh_cylinder_refinement_1".format(num_test))
# mdpa_out_name = "cfd_data/test_{}/mdpa_file/03_Mesh_cylinder_refinement_1".format(num_test)

mesher.RefineMesh_test(1.0, 10.0)
mesher.CreateGidControlOutput("cfd_data/test_{}/gid_file/04_Mesh_cylinder_refinement_1".format(num_test))
mdpa_out_name = "cfd_data/test_{}/mdpa_file/04_Mesh_cylinder_refinement_1".format(num_test)

mesher.WriteMdpaOutput(mdpa_out_name)
print("BOX REFINEMENT 1 DONE!")

input("Mesh_cylinder_refinement_1 DONE!")


print("*** Time: ", time.time() - start_time)
print("\nTEST ", num_test, "END\n\n")