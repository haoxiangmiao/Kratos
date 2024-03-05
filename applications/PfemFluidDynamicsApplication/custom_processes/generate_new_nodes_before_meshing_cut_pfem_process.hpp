//
//   Project Name:        KratosPfemFluidDynamicsApplication $
//   Created by:          $Author:                   AFranci $
//   Last modified by:    $Co-Author:                        $
//   Date:                $Date:                October 2016 $
//   Revision:            $Revision:                     0.0 $
//
//

#pragma once

// External includes

// System includes

// Project includes
#include "containers/variables_list_data_value_container.h"
#include "spatial_containers/spatial_containers.h"

#include "includes/model_part.h"
#include "custom_utilities/mesh_error_calculation_utilities.hpp"
#include "custom_utilities/mesher_utilities.hpp"
#include "custom_processes/mesher_process.hpp"
#include "custom_processes/generate_new_nodes_before_meshing_process.hpp"

/// VARIABLES used:
// Data:
// Flags:    (checked)
//           (set)
//           (modified)
//           (reset)
//(set):=(set in this process)

namespace Kratos
{

	///@name Kratos Classes
	///@{

	/// Refine Mesh Elements Process 2D and 3D
	/** The process labels the nodes to be refined (TO_REFINE)
	if the ThresholdVariable  is larger than a ReferenceThreshold
*/

	class GenerateNewNodesBeforeMeshingCutPfemProcess
		: public GenerateNewNodesBeforeMeshingProcess
	{
	public:
		///@name Type Definitions
		///@{

		/// Pointer definition of Process
		KRATOS_CLASS_POINTER_DEFINITION(GenerateNewNodesBeforeMeshingCutPfemProcess);

		typedef ModelPart::NodeType NodeType;
		typedef ModelPart::ConditionType ConditionType;
		typedef ModelPart::PropertiesType PropertiesType;
		typedef ConditionType::GeometryType GeometryType;
		typedef std::size_t SizeType;

		typedef GenerateNewNodesBeforeMeshingProcess BaseType;

		///@}
		///@name Life Cycle
		///@{

		/// Default constructor.
		GenerateNewNodesBeforeMeshingCutPfemProcess(
			ModelPart &rModelPart,
			MesherUtilities::MeshingParameters &rRemeshingParameters,
			int EchoLevel)
			: BaseType(rModelPart, rRemeshingParameters, EchoLevel)
		{
		}

		/// Destructor.
		virtual ~GenerateNewNodesBeforeMeshingCutPfemProcess() = default;

		///@}
		///@name Operators
		///@{

		/// This operator is provided to call the process as a function and simply calls the Execute method.
		void operator()()
		{
			Execute();
		}

		///@}
		///@name Operations
		///@{

		/// Execute method is used to execute the Process algorithms.
		void Execute() override
		{
			KRATOS_TRY

			KRATOS_INFO_IF("GenerateNewNodesBeforeMeshingCutPfemProcess", mEchoLevel > 1)
				<< " [ GENERATE NEW NODES for homogeneous mesh: " << std::endl;

			KRATOS_WARNING_IF("GenerateNewNodesBeforeMeshingCutPfemProcess", mrModelPart.Name() != mrRemesh.SubModelPartName)
				<< " ModelPart Supplied do not corresponds to the meshing domain: " << mrModelPart.FullName() << " != " << mrRemesh.SubModelPartName << "." << std::endl;

			const auto &r_current_process_info = mrModelPart.GetProcessInfo();
			double currentTime = r_current_process_info[TIME];
			double timeInterval = r_current_process_info[DELTA_TIME];
			const SizeType dimension = mrModelPart.ElementsBegin()->GetGeometry().WorkingSpaceDimension();

			bool refiningBox = false;
			for (SizeType index = 0; index < mrRemesh.UseRefiningBox.size(); index++)
			{
				if (mrRemesh.UseRefiningBox[index] == true && currentTime > mrRemesh.RefiningBoxInitialTime[index] && currentTime < mrRemesh.RefiningBoxFinalTime[index])
				{
					refiningBox = true;
				}
			}

			if (currentTime < 2 * timeInterval)
			{
				mrRemesh.Info->RemovedNodes = 0;
				mrRemesh.Info->BalancePrincipalSecondaryPartsNodes = 0;

				KRATOS_INFO_IF("GenerateNewNodesBeforeMeshingCutPfemProcess", mEchoLevel > 1)
					<< " First meshes: I repare the mesh without adding new nodes" << std::endl;
				mrRemesh.Info->InitialNumberOfNodes = mrRemesh.Info->NumberOfNodes;
			}

			int ElementsToRefine = mrRemesh.Info->RemovedNodes;
			SizeType eulerianInletNodes = mrRemesh.Info->NumberOfEulerianInletNodes;

			int initialNumberOfNodes = mrRemesh.Info->InitialNumberOfNodes;
			int numberOfNodes = mrRemesh.Info->NumberOfNodes;
			int extraNodes = initialNumberOfNodes - numberOfNodes;
			int toleredExtraNodes = int(0.05 * mrRemesh.Info->InitialNumberOfNodes);

			if (mrRemesh.ExecutionOptions.Is(MesherUtilities::REFINE_WALL_CORNER))
			{
				if ((ElementsToRefine - extraNodes) > toleredExtraNodes && refiningBox == false)
				{
					ElementsToRefine = toleredExtraNodes + extraNodes;
					if (ElementsToRefine < 0)
					{
						ElementsToRefine = 0;
					}
				}
			}

			KRATOS_INFO_IF("GenerateNewNodesBeforeMeshingCutPfemProcess", ElementsToRefine > 0 && mEchoLevel > 1)
				<< " I will look for " << ElementsToRefine << " new nodes" << std::endl;

			if (refiningBox == false)
			{

				if (ElementsToRefine > 0 || eulerianInletNodes > 0)
				{
					std::vector<array_1d<double, 3>> new_positions;
					std::vector<double> biggest_volumes;
					std::vector<array_1d<SizeType, 4>> nodes_id_to_interpolate;

					int CountNodes = 0;

					new_positions.resize(ElementsToRefine);
					biggest_volumes.resize(ElementsToRefine, false);
					nodes_id_to_interpolate.resize(ElementsToRefine);

					for (int nn = 0; nn < ElementsToRefine; nn++)
					{
						biggest_volumes[nn] = -1.0;
					}

					// std::vector<array_1d<double, 3>> CornerWallNewPositions;
					// std::vector<array_1d<SizeType, 4>> CornerWallNodesIDToInterpolate;
					// std::vector<Node::DofsContainerType> CornerWallNewDofs;
					// int cornerWallNewNodes = 0;
					// int maxOfNewWallNodes = toleredExtraNodes;
					// if (mrRemesh.ExecutionOptions.Is(MesherUtilities::REFINE_WALL_CORNER))
					// {
					// 	CornerWallNewPositions.resize(maxOfNewWallNodes, false);
					// 	CornerWallNodesIDToInterpolate.resize(maxOfNewWallNodes, false);
					// 	CornerWallNewDofs.resize(maxOfNewWallNodes, false);
					// }
					SizeType addedNodesAtEulerianInlet = 0;
					ModelPart::ElementsContainerType::iterator element_begin = mrModelPart.ElementsBegin();
					for (ModelPart::ElementsContainerType::const_iterator ie = element_begin; ie != mrModelPart.ElementsEnd(); ie++)
					{

						//////// choose the right (big and safe) elements to refine and compute the new node position and variables ////////
						if (dimension == 2)
						{
							SelectEdgeToRefine2D(ie->GetGeometry(), new_positions, biggest_volumes, nodes_id_to_interpolate, CountNodes, ElementsToRefine, addedNodesAtEulerianInlet);
						}
						else if (dimension == 3)
						{
							SelectEdgeToRefine3D(ie->GetGeometry(), new_positions, biggest_volumes, nodes_id_to_interpolate, CountNodes, ElementsToRefine, addedNodesAtEulerianInlet);
						}

					} // elements loop
					ElementsToRefine += addedNodesAtEulerianInlet;

					mrRemesh.Info->RemovedNodes -= ElementsToRefine;
					if (CountNodes < ElementsToRefine)
					{
						mrRemesh.Info->RemovedNodes += ElementsToRefine - CountNodes;
						new_positions.resize(CountNodes);
						biggest_volumes.resize(CountNodes, false);
						nodes_id_to_interpolate.resize(CountNodes);
					}
					SizeType maxId = 0;
					this->CreateAndAddNewNodes(new_positions, nodes_id_to_interpolate, ElementsToRefine, maxId);
				}
			}
			else
			{

				std::vector<array_1d<double, 3>> new_positions;
				std::vector<array_1d<SizeType, 4>> nodes_id_to_interpolate;

				int CountNodes = 0;

				new_positions.resize(0);
				nodes_id_to_interpolate.resize(0);

				ModelPart::ElementsContainerType::iterator element_begin = mrModelPart.ElementsBegin();
				int nodesInTransitionZone = 0;
				for (ModelPart::ElementsContainerType::const_iterator ie = element_begin; ie != mrModelPart.ElementsEnd(); ie++)
				{
					const SizeType dimension = ie->GetGeometry().WorkingSpaceDimension();
					//////// choose the right (big and safe) elements to refine and compute the new node position and variables ////////
					if (dimension == 2)
					{
						SelectEdgeToRefine2DWithRefinement(ie->GetGeometry(), new_positions, nodes_id_to_interpolate, CountNodes, ElementsToRefine, nodesInTransitionZone);
					}
					else if (dimension == 3)
					{
						SelectEdgeToRefine3DWithRefinement(ie->GetGeometry(), new_positions, nodes_id_to_interpolate, CountNodes, ElementsToRefine, nodesInTransitionZone);
					}

				} // elements loop

				mrRemesh.Info->RemovedNodes -= CountNodes;
				if (CountNodes < ElementsToRefine)
				{
					mrRemesh.Info->RemovedNodes += ElementsToRefine - CountNodes;
					new_positions.resize(CountNodes);
					nodes_id_to_interpolate.resize(CountNodes);
				}
				SizeType maxId = 0;
				this->CreateAndAddNewNodes(new_positions, nodes_id_to_interpolate, CountNodes, maxId);
			}

			mrRemesh.InputInitializedFlag = false;

			KRATOS_INFO_IF("GenerateNewNodesBeforeMeshingCutPfemProcess", mEchoLevel > 1)
				<< "   GENERATE NEW NODES ]; " << std::endl;

			KRATOS_CATCH(" ")
		}

		///@}
		///@name Access
		///@{

		///@}
		///@name Inquiry
		///@{

		///@}
		///@name Input and output
		///@{

		/// Turn back information as a string.
		std::string Info() const override
		{
			return "GenerateNewNodesBeforeMeshingCutPfemProcess";
		}

		/// Print information about this object.
		void PrintInfo(std::ostream &rOStream) const override
		{
			rOStream << "GenerateNewNodesBeforeMeshingCutPfemProcess";
		}

		/// Print object's data.s
		void PrintData(std::ostream &rOStream) const override
		{
		}

		///@}
		///@name Friends
		///@{

		///@}

	private:
		///@}
		///@name Private Operators
		///@{

		///@}
		///@name Private Operations
		///@{

		void SelectEdgeToRefine2D(
			Element::GeometryType &Element,
			std::vector<array_1d<double, 3>> &new_positions,
			std::vector<double> &biggest_volumes,
			std::vector<array_1d<SizeType, 4>> &nodes_id_to_interpolate,
			int &CountNodes,
			int &ElementsToRefine,
			SizeType &addedNodesAtEulerianInlet)
		{
			KRATOS_TRY

			const SizeType nds = Element.size();
			double meanMeshSize = mrRemesh.Refine->CriticalRadius;
			const double distance_tolerance = 0.05 * meanMeshSize;
			SizeType rigidNodes = 0;
			SizeType freesurfaceNodes = 0;
			SizeType lagrangianInletNodes = 0;
			SizeType eulerianInletNodes = 0;
			SizeType negativeDistanceNodes = 0;
			bool toEraseNodeFound = false;
			double rigidNodeLocalMeshSize = 0;
			double rigidNodeMeshCounter = 0;
			bool suitableElementForSecondAdd = true;

			for (SizeType pn = 0; pn < nds; ++pn)
			{
				if (Element[pn].Is(RIGID))
				{
					rigidNodes++;
					rigidNodeLocalMeshSize += Element[pn].FastGetSolutionStepValue(NODAL_H_WALL);
					rigidNodeMeshCounter += 1.0;
				}
				if (Element[pn].Is(TO_ERASE))
				{
					toEraseNodeFound = true;
				}
				if (Element[pn].Is(FREE_SURFACE))
				{
					freesurfaceNodes++;
				}
				if (Element[pn].Is(PFEMFlags::LAGRANGIAN_INLET))
				{
					lagrangianInletNodes++;
				}
				if (Element[pn].Is(PFEMFlags::EULERIAN_INLET))
				{
					eulerianInletNodes++;
				}
				if (Element[pn].GetSolutionStepValue(DISTANCE) < distance_tolerance)
				{
					negativeDistanceNodes++;
				}
			}

			if (rigidNodeMeshCounter > 0)
			{
				const double rigidWallMeshSize = rigidNodeLocalMeshSize / rigidNodeMeshCounter;
				const double ratio = rigidWallMeshSize / meanMeshSize;
				const double tolerance = 1.8;
				if (ratio > tolerance)
				{
					meanMeshSize *= 0.5;
					meanMeshSize += 0.5 * rigidWallMeshSize;
				}
			}

			const double limitEdgeLength = 1.4 * meanMeshSize;
			const double safetyCoefficient2D = 1.5;
			double penalization = 1.0; // to penalize adding node, penalization here should be smaller than 1
			if (rigidNodes > 1)
			{
				penalization = 0.8;
				if (lagrangianInletNodes > 0)
				{
					penalization = 0.9;
				}
			}
			else if (rigidNodes > 0 && freesurfaceNodes > 0 && eulerianInletNodes == 0)
			{
				penalization = 0;
			}
			else if (negativeDistanceNodes > 0) // to penalize insertion of nodes near the boundary
			{
				penalization = 0.9;
			}
			// else if (freesurfaceNodes > 0)
			// {
			// 	penalization = 0.875;
			// }

			double ElementalVolume = Element.Area();

			array_1d<double, 3> Edges(3, 0.0);
			array_1d<SizeType, 3> FirstEdgeNode(3, 0);
			array_1d<SizeType, 3> SecondEdgeNode(3, 0);
			double WallCharacteristicDistance = 0;
			array_1d<double, 3> CoorDifference = Element[1].Coordinates() - Element[0].Coordinates();
			double SquaredLength = CoorDifference[0] * CoorDifference[0] + CoorDifference[1] * CoorDifference[1];
			Edges[0] = std::sqrt(SquaredLength);
			FirstEdgeNode[0] = 0;
			SecondEdgeNode[0] = 1;
			if ((Element[0].Is(RIGID) && Element[1].Is(RIGID)) || (Element[0].Is(INLET) && Element[1].Is(INLET)))
			{
				WallCharacteristicDistance = Edges[0];
			}
			SizeType Counter = 0;
			for (SizeType i = 2; i < nds; i++)
			{
				for (SizeType j = 0; j < i; j++)
				{
					noalias(CoorDifference) = Element[i].Coordinates() - Element[j].Coordinates();
					SquaredLength = CoorDifference[0] * CoorDifference[0] + CoorDifference[1] * CoorDifference[1];
					Counter += 1;
					Edges[Counter] = std::sqrt(SquaredLength);
					FirstEdgeNode[Counter] = j;
					SecondEdgeNode[Counter] = i;
					if (((Element[i].Is(RIGID) && Element[j].Is(RIGID)) || (Element[i].Is(INLET) && Element[j].Is(INLET))) && Edges[Counter] > WallCharacteristicDistance)
					{
						WallCharacteristicDistance = Edges[Counter];
					}
				}
			}

			bool dangerousElement = false;

			for (SizeType i = 0; i < 3; i++)
			{
				if (rigidNodes > 1)
				{
					if ((Edges[i] < WallCharacteristicDistance * safetyCoefficient2D && (Element[FirstEdgeNode[i]].Is(RIGID) || Element[SecondEdgeNode[i]].Is(RIGID))) ||
						((Element[FirstEdgeNode[i]].Is(RIGID) && Element[SecondEdgeNode[i]].Is(RIGID)) || (Element[FirstEdgeNode[i]].Is(INLET) && Element[SecondEdgeNode[i]].Is(INLET))))
					{
						Edges[i] = 0;
					}
					if ((Element[FirstEdgeNode[i]].Is(FREE_SURFACE) || Element[FirstEdgeNode[i]].Is(RIGID)) &&
						(Element[SecondEdgeNode[i]].Is(FREE_SURFACE) || Element[SecondEdgeNode[i]].Is(RIGID)) &&
						(Element[FirstEdgeNode[i]].IsNot(PFEMFlags::EULERIAN_INLET) && Element[SecondEdgeNode[i]].IsNot(PFEMFlags::EULERIAN_INLET)))
					{
						Edges[i] = 0;
					}
				}
				else if (rigidNodes == 0)
				{
					SizeType propertyIdFirstNode = Element[FirstEdgeNode[i]].FastGetSolutionStepValue(PROPERTY_ID);
					SizeType propertyIdSecondNode = Element[SecondEdgeNode[i]].FastGetSolutionStepValue(PROPERTY_ID);
					if (propertyIdFirstNode != propertyIdSecondNode)
					{
						penalization = 0.9; // 10% less than normal nodes
					}
				}
			}
			if ((Edges[0] == 0 && Edges[1] == 0 && Edges[2] == 0) || rigidNodes == 3)
			{
				dangerousElement = true;
			}

			if (dangerousElement == false && toEraseNodeFound == false)
			{
				SizeType maxCount = 3;
				double LargestEdge = 0;

				for (SizeType i = 0; i < 3; i++)
				{
					if (Edges[i] > LargestEdge)
					{
						maxCount = i;
						LargestEdge = Edges[i];
					}
				}

				if (CountNodes < ElementsToRefine && LargestEdge > limitEdgeLength && eulerianInletNodes == 0)
				{

					array_1d<double, 3> new_position = (Element[FirstEdgeNode[maxCount]].Coordinates() + Element[SecondEdgeNode[maxCount]].Coordinates()) * 0.5;

					bool suitableElement = true;
					for (int j = 0; j < CountNodes; j++)
					{
						const double diffX = std::abs(new_positions[j][0] - new_position[0]) - meanMeshSize * 0.5;
						const double diffY = std::abs(new_positions[j][1] - new_position[1]) - meanMeshSize * 0.5;
						if (diffX < 0 && diffY < 0) //  the node is in the same zone of a previously inserted node
						{
							suitableElement = false;
						}
					}

					if (suitableElement)
					{
						nodes_id_to_interpolate[CountNodes][0] = Element[FirstEdgeNode[maxCount]].GetId();
						nodes_id_to_interpolate[CountNodes][1] = Element[SecondEdgeNode[maxCount]].GetId();
						biggest_volumes[CountNodes] = ElementalVolume;
						new_positions[CountNodes] = new_position;
						CountNodes++;
					}
				}
				else if (freesurfaceNodes < 3 && rigidNodes < 3 && eulerianInletNodes == 0)
				{
					ElementalVolume *= penalization;
					for (int nn = 0; nn < ElementsToRefine; nn++)
					{
						if (ElementalVolume > biggest_volumes[nn])
						{

							if (maxCount < 3 && LargestEdge > limitEdgeLength)
							{
								array_1d<double, 3> new_position = (Element[FirstEdgeNode[maxCount]].Coordinates() + Element[SecondEdgeNode[maxCount]].Coordinates()) * 0.5;
								if (ElementsToRefine > 1 && CountNodes > 0)
								{
									for (int j = 0; j < ElementsToRefine; j++)
									{
										const double diffX = std::abs(new_positions[j][0] - new_position[0]) - meanMeshSize * 0.5;
										const double diffY = std::abs(new_positions[j][1] - new_position[1]) - meanMeshSize * 0.5;
										if (diffX < 0 && diffY < 0) // the node is in the same zone of a previously inserted node
										{
											suitableElementForSecondAdd = false;
										}
									}
								}

								if (suitableElementForSecondAdd)
								{
									nodes_id_to_interpolate[nn][0] = Element[FirstEdgeNode[maxCount]].GetId();
									nodes_id_to_interpolate[nn][1] = Element[SecondEdgeNode[maxCount]].GetId();
									biggest_volumes[nn] = ElementalVolume;
									new_positions[nn] = new_position;
								}
							}

							break;
						}
					}
				}

				if (eulerianInletNodes > 0 && LargestEdge > (2.0 * meanMeshSize))
				{

					array_1d<double, 3> new_position = (Element[FirstEdgeNode[maxCount]].Coordinates() + Element[SecondEdgeNode[maxCount]].Coordinates()) * 0.5;

					bool suitableElement = true;
					for (int j = 0; j < CountNodes; j++)
					{
						const double diffX = std::abs(new_positions[j][0] - new_position[0]) - meanMeshSize * 0.5;
						const double diffY = std::abs(new_positions[j][1] - new_position[1]) - meanMeshSize * 0.5;
						// if (diffX < 0 && diffY < 0)																															//  the node is in the same zone of a previously inserted node
						if ((diffX < 0 && diffY < 0) || (Element[FirstEdgeNode[maxCount]].IsNot(INLET) && Element[SecondEdgeNode[maxCount]].IsNot(INLET))) //  the node is in the same zone of a previously inserted node

						{
							suitableElement = false;
						}
					}

					if (suitableElement)
					{
						if (CountNodes >= ElementsToRefine)
						{
							new_positions.resize(CountNodes + 1);
							biggest_volumes.resize(CountNodes + 1, false);
							nodes_id_to_interpolate.resize(CountNodes + 1);
							addedNodesAtEulerianInlet++;
						}
						nodes_id_to_interpolate[CountNodes][0] = Element[FirstEdgeNode[maxCount]].GetId();
						nodes_id_to_interpolate[CountNodes][1] = Element[SecondEdgeNode[maxCount]].GetId();
						biggest_volumes[CountNodes] = ElementalVolume;
						new_positions[CountNodes] = new_position;
						CountNodes++;
					}
				}
			}

			KRATOS_CATCH("")
		}

		void SelectEdgeToRefine3D(Element::GeometryType &Element,
								  std::vector<array_1d<double, 3>> &new_positions,
								  std::vector<double> &biggest_volumes,
								  std::vector<array_1d<SizeType, 4>> &nodes_id_to_interpolate,
								  int &CountNodes,
								  int &ElementsToRefine,
								  SizeType &addedNodesAtEulerianInlet)
		{
			KRATOS_TRY

			const SizeType nds = Element.size();
			double meanMeshSize = mrRemesh.Refine->CriticalRadius;
			const double distance_tolerance = 0.05 * meanMeshSize;
			SizeType rigidNodes = 0;
			SizeType freesurfaceNodes = 0;
			SizeType lagrangianInletNodes = 0;
			SizeType eulerianInletNodes = 0;
			SizeType negativeDistanceNodes = 0;
			bool toEraseNodeFound = false;
			double rigidNodeLocalMeshSize = 0;
			double rigidNodeMeshCounter = 0;
			bool suitableElementForSecondAdd = true;

			for (SizeType pn = 0; pn < nds; ++pn)
			{
				if (Element[pn].Is(RIGID))
				{
					rigidNodes++;
					rigidNodeLocalMeshSize += Element[pn].FastGetSolutionStepValue(NODAL_H_WALL);
					rigidNodeMeshCounter += 1.0;
				}
				if (Element[pn].Is(TO_ERASE))
				{
					toEraseNodeFound = true;
				}
				if (Element[pn].Is(FREE_SURFACE))
				{
					freesurfaceNodes++;
				}
				if (Element[pn].Is(PFEMFlags::LAGRANGIAN_INLET))
				{
					lagrangianInletNodes++;
				}
				if (Element[pn].Is(PFEMFlags::EULERIAN_INLET))
				{
					eulerianInletNodes++;
				}
				if (Element[pn].GetSolutionStepValue(DISTANCE) < distance_tolerance)
				{
					negativeDistanceNodes++;
				}
			}

			if (rigidNodeMeshCounter > 0)
			{
				const double rigidWallMeshSize = rigidNodeLocalMeshSize / rigidNodeMeshCounter;
				const double ratio = rigidWallMeshSize / meanMeshSize;
				const double tolerance = 1.8;
				if (ratio > tolerance)
				{
					meanMeshSize *= 0.5;
					meanMeshSize += 0.5 * rigidWallMeshSize;
				}
			}

			const double limitEdgeLength = 1.25 * meanMeshSize;
			const double safetyCoefficient3D = 1.6;
			double penalization = 1.0; // penalization here should be smaller than 1
			if (rigidNodes > 2)
			{
				penalization = 0.7;
				if (lagrangianInletNodes > 0)
				{
					penalization = 0.9;
				}
			}
			else if (rigidNodes > 0 && freesurfaceNodes > 0 && eulerianInletNodes == 0)
			{
				penalization = 0;
			}
			else if (freesurfaceNodes > 0)
			{
				penalization = 0.95;
			}
			else if (negativeDistanceNodes > 0) // to penalize insertion of nodes near the boundary
			{
				penalization = 0.9;
			}

			double ElementalVolume = Element.Volume();

			array_1d<double, 6> Edges(6, 0.0);
			array_1d<SizeType, 6> FirstEdgeNode(6, 0);
			array_1d<SizeType, 6> SecondEdgeNode(6, 0);
			double WallCharacteristicDistance = 0;
			array_1d<double, 3> CoorDifference = Element[1].Coordinates() - Element[0].Coordinates();
			double SquaredLength = CoorDifference[0] * CoorDifference[0] + CoorDifference[1] * CoorDifference[1] + CoorDifference[2] * CoorDifference[2];
			Edges[0] = std::sqrt(SquaredLength);
			FirstEdgeNode[0] = 0;
			SecondEdgeNode[0] = 1;
			if ((Element[0].Is(RIGID) && Element[1].Is(RIGID)) || (Element[0].Is(INLET) && Element[1].Is(INLET)))
			{
				WallCharacteristicDistance = Edges[0];
			}
			SizeType Counter = 0;
			for (SizeType i = 2; i < nds; i++)
			{
				for (SizeType j = 0; j < i; j++)
				{
					noalias(CoorDifference) = Element[i].Coordinates() - Element[j].Coordinates();
					SquaredLength = CoorDifference[0] * CoorDifference[0] + CoorDifference[1] * CoorDifference[1] + CoorDifference[2] * CoorDifference[2];
					Counter += 1;
					Edges[Counter] = std::sqrt(SquaredLength);
					FirstEdgeNode[Counter] = j;
					SecondEdgeNode[Counter] = i;
					if (((Element[i].Is(RIGID) && Element[j].Is(RIGID)) || (Element[i].Is(INLET) && Element[j].Is(INLET))) && Edges[Counter] > WallCharacteristicDistance)
					{
						WallCharacteristicDistance = Edges[Counter];
					}
				}
			}
			// Edges connectivity: Edges[0]=d01, Edges[1]=d20, Edges[2]=d21, Edges[3]=d30, Edges[4]=d31, Edges[5]=d32
			bool dangerousElement = false;

			for (SizeType i = 0; i < 6; i++)
			{
				if (rigidNodes > 1)
				{
					if ((Edges[i] < WallCharacteristicDistance * safetyCoefficient3D && (Element[FirstEdgeNode[i]].Is(RIGID) || Element[SecondEdgeNode[i]].Is(RIGID))) ||
						((Element[FirstEdgeNode[i]].Is(RIGID) && Element[SecondEdgeNode[i]].Is(RIGID)) || (Element[FirstEdgeNode[i]].Is(INLET) && Element[SecondEdgeNode[i]].Is(INLET))))
					{
						Edges[i] = 0;
					}
					if ((Element[FirstEdgeNode[i]].Is(FREE_SURFACE) || Element[FirstEdgeNode[i]].Is(RIGID)) &&
						(Element[SecondEdgeNode[i]].Is(FREE_SURFACE) || Element[SecondEdgeNode[i]].Is(RIGID)) &&
						(Element[FirstEdgeNode[i]].IsNot(PFEMFlags::EULERIAN_INLET) && Element[SecondEdgeNode[i]].IsNot(PFEMFlags::EULERIAN_INLET)))
					{
						Edges[i] = 0;
					}
				}
				else if (rigidNodes == 0)
				{
					SizeType propertyIdFirstNode = Element[FirstEdgeNode[i]].FastGetSolutionStepValue(PROPERTY_ID);
					SizeType propertyIdSecondNode = Element[SecondEdgeNode[i]].FastGetSolutionStepValue(PROPERTY_ID);
					if (propertyIdFirstNode != propertyIdSecondNode)
					{
						penalization = 0.8; // 20% less than normal nodes
					}
				}
			}
			if (rigidNodes == 1)
			{
				if (Element[0].Is(RIGID))
				{
					Edges[0] = 0;
					Edges[1] = 0;
					Edges[3] = 0;
				}
				if (Element[1].Is(RIGID))
				{
					Edges[0] = 0;
					Edges[2] = 0;
					Edges[4] = 0;
				}
				if (Element[2].Is(RIGID))
				{
					Edges[1] = 0;
					Edges[2] = 0;
					Edges[5] = 0;
				}
				if (Element[3].Is(RIGID))
				{
					Edges[3] = 0;
					Edges[4] = 0;
					Edges[5] = 0;
				}
			}

			if ((Edges[0] == 0 && Edges[1] == 0 && Edges[2] == 0 && Edges[3] == 0 && Edges[4] == 0 && Edges[5] == 0) || rigidNodes > 2)
			{
				dangerousElement = true;
			}

			// just to fill the vector
			if (dangerousElement == false && toEraseNodeFound == false)
			{
				SizeType maxCount = 6;
				double LargestEdge = 0;

				for (SizeType i = 0; i < 6; i++)
				{
					if (Edges[i] > LargestEdge)
					{
						maxCount = i;
						LargestEdge = Edges[i];
					}
				}
				if ((CountNodes < ElementsToRefine && LargestEdge > limitEdgeLength && eulerianInletNodes == 0))
				{

					array_1d<double, 3> new_position = (Element[FirstEdgeNode[maxCount]].Coordinates() + Element[SecondEdgeNode[maxCount]].Coordinates()) * 0.5;

					bool suitableElement = true;
					for (int j = 0; j < CountNodes; j++)
					{
						const double diffX = std::abs(new_positions[j][0] - new_position[0]) - meanMeshSize * 0.5;
						const double diffY = std::abs(new_positions[j][1] - new_position[1]) - meanMeshSize * 0.5;
						const double diffZ = std::abs(new_positions[j][2] - new_position[2]) - meanMeshSize * 0.5;
						if (diffX < 0 && diffY < 0 && diffZ < 0) //  the node is in the same zone of a previously inserted node
						{
							suitableElement = false;
						}
					}

					if (suitableElement)
					{
						nodes_id_to_interpolate[CountNodes][0] = Element[FirstEdgeNode[maxCount]].GetId();
						nodes_id_to_interpolate[CountNodes][1] = Element[SecondEdgeNode[maxCount]].GetId();
						biggest_volumes[CountNodes] = ElementalVolume;
						new_positions[CountNodes] = new_position;
						CountNodes++;
					}
				}
				else if (freesurfaceNodes < 4 && rigidNodes < 4 && eulerianInletNodes == 0)
				{

					ElementalVolume *= penalization;
					for (int nn = 0; nn < ElementsToRefine; nn++)
					{
						if (ElementalVolume > biggest_volumes[nn])
						{
							if (maxCount < 6 && LargestEdge > limitEdgeLength)
							{
								array_1d<double, 3> new_position = (Element[FirstEdgeNode[maxCount]].Coordinates() + Element[SecondEdgeNode[maxCount]].Coordinates()) * 0.5;

								if (ElementsToRefine > 1 && CountNodes > 0)
								{
									for (int j = 0; j < ElementsToRefine; j++)
									{
										const double diffX = std::abs(new_positions[j][0] - new_position[0]) - meanMeshSize * 0.5;
										const double diffY = std::abs(new_positions[j][1] - new_position[1]) - meanMeshSize * 0.5;
										const double diffZ = std::abs(new_positions[j][2] - new_position[2]) - meanMeshSize * 0.5;
										if (diffX < 0 && diffY < 0 && diffZ < 0) // the node is in the same zone of a previously inserted node
										{
											suitableElementForSecondAdd = false;
										}
									}
								}

								if (suitableElementForSecondAdd)
								{
									nodes_id_to_interpolate[nn][0] = Element[FirstEdgeNode[maxCount]].GetId();
									nodes_id_to_interpolate[nn][1] = Element[SecondEdgeNode[maxCount]].GetId();
									biggest_volumes[nn] = ElementalVolume;
									new_positions[nn] = new_position;
								}
							}

							break;
						}
					}
				}

				if (eulerianInletNodes > 0 && LargestEdge > (1.7 * meanMeshSize))
				{

					array_1d<double, 3> new_position = (Element[FirstEdgeNode[maxCount]].Coordinates() + Element[SecondEdgeNode[maxCount]].Coordinates()) * 0.5;

					bool suitableElement = true;
					for (int j = 0; j < CountNodes; j++)
					{
						const double diffX = std::abs(new_positions[j][0] - new_position[0]) - meanMeshSize * 0.5;
						const double diffY = std::abs(new_positions[j][1] - new_position[1]) - meanMeshSize * 0.5;
						const double diffZ = std::abs(new_positions[j][2] - new_position[2]) - meanMeshSize * 0.5;
						if ((diffX < 0 && diffY < 0 && diffZ < 0) || (Element[FirstEdgeNode[maxCount]].IsNot(INLET) && Element[SecondEdgeNode[maxCount]].IsNot(INLET))) //  the node is in the same zone of a previously inserted node
						{
							suitableElement = false;
						}
					}

					if (suitableElement)
					{
						if (CountNodes >= ElementsToRefine)
						{
							new_positions.resize(CountNodes + 1);
							biggest_volumes.resize(CountNodes + 1, false);
							nodes_id_to_interpolate.resize(CountNodes + 1);
							addedNodesAtEulerianInlet++;
						}
						nodes_id_to_interpolate[CountNodes][0] = Element[FirstEdgeNode[maxCount]].GetId();
						nodes_id_to_interpolate[CountNodes][1] = Element[SecondEdgeNode[maxCount]].GetId();
						biggest_volumes[CountNodes] = ElementalVolume;
						new_positions[CountNodes] = new_position;
						CountNodes++;
					}
				}
			}

			KRATOS_CATCH("")
		}

		void SelectEdgeToRefine2DWithRefinement(Element::GeometryType &Element,
												std::vector<array_1d<double, 3>> &new_positions,
												std::vector<array_1d<SizeType, 4>> &nodes_id_to_interpolate,
												int &CountNodes,
												const int ElementsToRefine,
												int &nodesInTransitionZone)
		{
			KRATOS_TRY

			const SizeType nds = Element.size();

			SizeType rigidNodes = 0;
			SizeType freesurfaceNodes = 0;
			SizeType negativeDistanceNodes = 0;
			bool toEraseNodeFound = false;
			double rigidNodeLocalMeshSize = 0;
			double rigidNodeMeshCounter = 0;
			double meanMeshSize = mrRemesh.Refine->CriticalRadius;
			const auto &r_current_process_info = mrModelPart.GetProcessInfo();
			const double currentTime = r_current_process_info[TIME];
			bool insideTransitionZone = false;
			for (SizeType pn = 0; pn < nds; ++pn)
			{
				mMesherUtilities.DefineMeshSizeInTransitionZones2D(mrRemesh, currentTime, Element[pn].Coordinates(), meanMeshSize, insideTransitionZone);
			}
			const double distance_tolerance = 0.05 * meanMeshSize;
			for (SizeType pn = 0; pn < nds; ++pn)
			{
				if (Element[pn].Is(RIGID))
				{
					rigidNodes++;
					rigidNodeLocalMeshSize += Element[pn].FastGetSolutionStepValue(NODAL_H_WALL);
					rigidNodeMeshCounter += 1.0;
				}
				if (Element[pn].Is(TO_ERASE))
				{
					toEraseNodeFound = true;
				}
				if (Element[pn].Is(FREE_SURFACE))
				{
					freesurfaceNodes++;
				}
				if (Element[pn].GetSolutionStepValue(DISTANCE) < distance_tolerance)
				{
					negativeDistanceNodes++;
				}
			}

			if (rigidNodeMeshCounter > 0)
			{
				const double rigidWallMeshSize = rigidNodeLocalMeshSize / rigidNodeMeshCounter;
				const double ratio = rigidWallMeshSize / meanMeshSize;
				const double tolerance = 1.8;
				if (ratio > tolerance)
				{
					meanMeshSize *= 0.5;
					meanMeshSize += 0.5 * rigidWallMeshSize;
				}
			}
			double penalization = 1.0; // penalization here should be greater than 1

			if (freesurfaceNodes > 0)
			{
				penalization = 1.2; // to avoid to gain too much volume during remeshing step
			}
			else if (negativeDistanceNodes > 0) // to penalize insertion of nodes near the boundary
			{
				penalization = 0.9;
			}
			const double safetyCoefficient2D = 1.5;
			array_1d<double, 3> Edges(3, 0.0);
			array_1d<SizeType, 3> FirstEdgeNode(3, 0);
			array_1d<SizeType, 3> SecondEdgeNode(3, 0);
			double WallCharacteristicDistance = 0;
			array_1d<double, 3> CoorDifference = Element[1].Coordinates() - Element[0].Coordinates();
			double SquaredLength = CoorDifference[0] * CoorDifference[0] + CoorDifference[1] * CoorDifference[1];
			Edges[0] = std::sqrt(SquaredLength);
			FirstEdgeNode[0] = 0;
			SecondEdgeNode[0] = 1;
			if ((Element[0].Is(RIGID) && Element[1].Is(RIGID)) || (Element[0].Is(INLET) && Element[1].Is(INLET)))
			{
				WallCharacteristicDistance = Edges[0];
			}
			SizeType Counter = 0;
			for (SizeType i = 2; i < nds; i++)
			{
				for (SizeType j = 0; j < i; j++)
				{
					noalias(CoorDifference) = Element[i].Coordinates() - Element[j].Coordinates();
					SquaredLength = CoorDifference[0] * CoorDifference[0] + CoorDifference[1] * CoorDifference[1];
					Counter += 1;
					Edges[Counter] = std::sqrt(SquaredLength);
					FirstEdgeNode[Counter] = j;
					SecondEdgeNode[Counter] = i;
					if (Element[i].Is(RIGID) && Element[j].Is(RIGID) && Edges[Counter] > WallCharacteristicDistance)
					{
						WallCharacteristicDistance = Edges[Counter];
					}
				}
			}

			bool dangerousElement = false;

			for (SizeType i = 0; i < 3; i++)
			{
				if (rigidNodes > 1)
				{
					if ((Edges[i] < WallCharacteristicDistance * safetyCoefficient2D && (Element[FirstEdgeNode[i]].Is(RIGID) || Element[SecondEdgeNode[i]].Is(RIGID))) ||
						(Element[FirstEdgeNode[i]].Is(RIGID) && Element[SecondEdgeNode[i]].Is(RIGID)))
					{
						Edges[i] = 0;
					}
					if ((Element[FirstEdgeNode[i]].Is(FREE_SURFACE) || Element[FirstEdgeNode[i]].Is(RIGID)) &&
						(Element[SecondEdgeNode[i]].Is(FREE_SURFACE) || Element[SecondEdgeNode[i]].Is(RIGID)))
					{
						Edges[i] = 0;
					}
				}
				else if (rigidNodes == 0)
				{
					SizeType propertyIdFirstNode = Element[FirstEdgeNode[i]].FastGetSolutionStepValue(PROPERTY_ID);
					SizeType propertyIdSecondNode = Element[SecondEdgeNode[i]].FastGetSolutionStepValue(PROPERTY_ID);
					if (propertyIdFirstNode != propertyIdSecondNode)
					{
						penalization = 1.1; // 10% more than normal nodes
					}
				}
			}
			if ((Edges[0] == 0 && Edges[1] == 0 && Edges[2] == 0) || rigidNodes == 3)
			{
				dangerousElement = true;
			}
			const double limitEdgeLength = 1.9 * meanMeshSize * penalization;
			const double extraLimitEdgeLength = 2.5 * meanMeshSize * penalization;

			if (dangerousElement == false && toEraseNodeFound == false)
			{
				SizeType maxCount = 3;
				double LargestEdge = 0;

				for (SizeType i = 0; i < 3; i++)
				{
					if (Edges[i] > LargestEdge)
					{
						maxCount = i;
						LargestEdge = Edges[i];
					}
				}

				if (((CountNodes < (ElementsToRefine + nodesInTransitionZone) || insideTransitionZone == true) && LargestEdge > limitEdgeLength) || LargestEdge > extraLimitEdgeLength)
				{
					bool newNode = true;
					for (SizeType i = 0; i < unsigned(CountNodes); i++)
					{
						if ((nodes_id_to_interpolate[i][0] == Element[FirstEdgeNode[maxCount]].GetId() && nodes_id_to_interpolate[i][1] == Element[SecondEdgeNode[maxCount]].GetId()) ||
							(nodes_id_to_interpolate[i][1] == Element[FirstEdgeNode[maxCount]].GetId() && nodes_id_to_interpolate[i][0] == Element[SecondEdgeNode[maxCount]].GetId()))
						{
							newNode = false;
						}
					}
					if (newNode == true)
					{
						new_positions.resize(CountNodes + 1);
						nodes_id_to_interpolate.resize(CountNodes + 1);
						array_1d<double, 3> new_position = (Element[FirstEdgeNode[maxCount]].Coordinates() + Element[SecondEdgeNode[maxCount]].Coordinates()) * 0.5;
						nodes_id_to_interpolate[CountNodes][0] = Element[FirstEdgeNode[maxCount]].GetId();
						nodes_id_to_interpolate[CountNodes][1] = Element[SecondEdgeNode[maxCount]].GetId();
						new_positions[CountNodes] = new_position;
						CountNodes++;
						if (insideTransitionZone)
						{
							nodesInTransitionZone++;
						}
					}
				}
			}

			KRATOS_CATCH("")
		}

		void SelectEdgeToRefine3DWithRefinement(
			Element::GeometryType &Element,
			std::vector<array_1d<double, 3>> &new_positions,
			std::vector<array_1d<SizeType, 4>> &nodes_id_to_interpolate,
			int &CountNodes,
			const int ElementsToRefine,
			int &nodesInTransitionZone)
		{
			KRATOS_TRY

			const SizeType nds = Element.size();

			SizeType rigidNodes = 0;
			SizeType freesurfaceNodes = 0;
			SizeType negativeDistanceNodes = 0;
			bool toEraseNodeFound = false;
			double meanMeshSize = mrRemesh.Refine->CriticalRadius;
			const ProcessInfo &r_current_process_info = mrModelPart.GetProcessInfo();
			double currentTime = r_current_process_info[TIME];
			bool insideTransitionZone = false;
			double rigidNodeLocalMeshSize = 0;
			double rigidNodeMeshCounter = 0;
			for (SizeType pn = 0; pn < nds; ++pn)
			{
				mMesherUtilities.DefineMeshSizeInTransitionZones3D(mrRemesh, currentTime, Element[pn].Coordinates(), meanMeshSize, insideTransitionZone);
			}
			const double distance_tolerance = 0.05 * meanMeshSize;
			for (SizeType pn = 0; pn < nds; ++pn)
			{
				if (Element[pn].Is(RIGID))
				{
					rigidNodes++;
					rigidNodeLocalMeshSize += Element[pn].FastGetSolutionStepValue(NODAL_H_WALL);
					rigidNodeMeshCounter += 1.0;
				}
				if (Element[pn].Is(TO_ERASE))
				{
					toEraseNodeFound = true;
				}
				if (Element[pn].Is(FREE_SURFACE))
				{
					freesurfaceNodes++;
				}
				if (Element[pn].GetSolutionStepValue(DISTANCE) < distance_tolerance)
				{
					negativeDistanceNodes++;
				}
			}

			if (rigidNodeMeshCounter > 0)
			{
				const double rigidWallMeshSize = rigidNodeLocalMeshSize / rigidNodeMeshCounter;
				const double ratio = rigidWallMeshSize / meanMeshSize;
				const double tolerance = 1.8;
				if (ratio > tolerance)
				{
					meanMeshSize *= 0.5;
					meanMeshSize += 0.5 * rigidWallMeshSize;
				}
			}

			double penalization = 1.0; // penalization here should be greater than 1

			if (freesurfaceNodes > 0)
			{
				penalization = 1.2; // to avoid to gain too much volume during remeshing step
			}
			else if (negativeDistanceNodes > 0) // to penalize insertion of nodes near the boundary
			{
				penalization = 0.9;
			}

			const double safetyCoefficient3D = 1.6;
			array_1d<double, 6> Edges(6, 0.0);
			array_1d<SizeType, 6> FirstEdgeNode(6, 0);
			array_1d<SizeType, 6> SecondEdgeNode(6, 0);
			double WallCharacteristicDistance = 0;
			array_1d<double, 3> CoorDifference = Element[1].Coordinates() - Element[0].Coordinates();
			double SquaredLength = CoorDifference[0] * CoorDifference[0] + CoorDifference[1] * CoorDifference[1] + CoorDifference[2] * CoorDifference[2];
			Edges[0] = std::sqrt(SquaredLength);
			FirstEdgeNode[0] = 0;
			SecondEdgeNode[0] = 1;
			if ((Element[0].Is(RIGID) && Element[1].Is(RIGID)) || (Element[0].Is(INLET) && Element[1].Is(INLET)))
			{
				WallCharacteristicDistance = Edges[0];
			}
			SizeType Counter = 0;
			for (SizeType i = 2; i < nds; i++)
			{
				for (SizeType j = 0; j < i; j++)
				{
					noalias(CoorDifference) = Element[i].Coordinates() - Element[j].Coordinates();
					SquaredLength = CoorDifference[0] * CoorDifference[0] + CoorDifference[1] * CoorDifference[1] + CoorDifference[2] * CoorDifference[2];
					Counter += 1;
					Edges[Counter] = std::sqrt(SquaredLength);
					FirstEdgeNode[Counter] = j;
					SecondEdgeNode[Counter] = i;
					if (Element[i].Is(RIGID) && Element[j].Is(RIGID) && Edges[Counter] > WallCharacteristicDistance)
					{
						WallCharacteristicDistance = Edges[Counter];
					}
				}
			}
			// Edges connectivity: Edges[0]=d01, Edges[1]=d20, Edges[2]=d21, Edges[3]=d30, Edges[4]=d31, Edges[5]=d32
			bool dangerousElement = false;
			for (SizeType i = 0; i < 6; i++)
			{
				if (rigidNodes > 1)
				{
					if ((Edges[i] < WallCharacteristicDistance * safetyCoefficient3D && (Element[FirstEdgeNode[i]].Is(RIGID) || Element[SecondEdgeNode[i]].Is(RIGID))) ||
						(Element[FirstEdgeNode[i]].Is(RIGID) && Element[SecondEdgeNode[i]].Is(RIGID)))
					{
						Edges[i] = 0;
					}
					if ((Element[FirstEdgeNode[i]].Is(FREE_SURFACE) || Element[FirstEdgeNode[i]].Is(RIGID)) &&
						(Element[SecondEdgeNode[i]].Is(FREE_SURFACE) || Element[SecondEdgeNode[i]].Is(RIGID)))
					{
						Edges[i] = 0;
					}
				}
				else if (rigidNodes == 0)
				{
					SizeType propertyIdFirstNode = Element[FirstEdgeNode[i]].FastGetSolutionStepValue(PROPERTY_ID);
					SizeType propertyIdSecondNode = Element[SecondEdgeNode[i]].FastGetSolutionStepValue(PROPERTY_ID);
					if (propertyIdFirstNode != propertyIdSecondNode)
					{
						penalization = 1.2; // 20% less than normal nodes
					}
				}
			}
			if (rigidNodes == 1)
			{
				if (Element[0].Is(RIGID))
				{
					Edges[0] = 0;
					Edges[1] = 0;
					Edges[3] = 0;
				}
				if (Element[1].Is(RIGID))
				{
					Edges[0] = 0;
					Edges[2] = 0;
					Edges[4] = 0;
				}
				if (Element[2].Is(RIGID))
				{
					Edges[1] = 0;
					Edges[2] = 0;
					Edges[5] = 0;
				}
				if (Element[3].Is(RIGID))
				{
					Edges[3] = 0;
					Edges[4] = 0;
					Edges[5] = 0;
				}
			}

			if ((Edges[0] == 0 && Edges[1] == 0 && Edges[2] == 0 && Edges[3] == 0 && Edges[4] == 0 && Edges[5] == 0) || rigidNodes > 2)
			{
				dangerousElement = true;
			}
			const double limitEdgeLength = 1.9 * meanMeshSize * penalization;
			const double extraLimitEdgeLength = 2.5 * meanMeshSize * penalization;

			// just to fill the vector
			if (dangerousElement == false && toEraseNodeFound == false)
			{
				SizeType maxCount = 6;
				double LargestEdge = 0;
				for (SizeType i = 0; i < 6; i++)
				{
					if (Edges[i] > LargestEdge)
					{
						maxCount = i;
						LargestEdge = Edges[i];
					}
				}

				if (((CountNodes < (ElementsToRefine + nodesInTransitionZone) || insideTransitionZone == true) && LargestEdge > limitEdgeLength) || LargestEdge > extraLimitEdgeLength)
				{
					bool newNode = true;
					for (SizeType i = 0; i < unsigned(CountNodes); i++)
					{
						if ((nodes_id_to_interpolate[i][0] == Element[FirstEdgeNode[maxCount]].GetId() && nodes_id_to_interpolate[i][1] == Element[SecondEdgeNode[maxCount]].GetId()) ||
							(nodes_id_to_interpolate[i][1] == Element[FirstEdgeNode[maxCount]].GetId() && nodes_id_to_interpolate[i][0] == Element[SecondEdgeNode[maxCount]].GetId()))
						{
							newNode = false;
						}
					}
					if (newNode == true)
					{
						new_positions.resize(CountNodes + 1);
						nodes_id_to_interpolate.resize(CountNodes + 1);
						array_1d<double, 3> new_position = (Element[FirstEdgeNode[maxCount]].Coordinates() + Element[SecondEdgeNode[maxCount]].Coordinates()) * 0.5;
						nodes_id_to_interpolate[CountNodes][0] = Element[FirstEdgeNode[maxCount]].GetId();
						nodes_id_to_interpolate[CountNodes][1] = Element[SecondEdgeNode[maxCount]].GetId();
						new_positions[CountNodes] = new_position;
						CountNodes++;
						if (insideTransitionZone)
						{
							nodesInTransitionZone++;
						}
					}
				}
			}

			KRATOS_CATCH("")
		}

		///@}
		///@name Private  Access
		///@{

		///@}
		///@name Private Inquiry
		///@{

		///@}
		///@name Un accessible methods
		///@{

		/// Assignment operator.
		GenerateNewNodesBeforeMeshingCutPfemProcess &operator=(GenerateNewNodesBeforeMeshingCutPfemProcess const &rOther);

		/// this function is a private function

		/// Copy constructor.
		// Process(Process const& rOther);

		///@}

	}; // Class Process

	///@}

	///@name Type Definitions
	///@{

	///@}
	///@name Input and output
	///@{

	/// input stream function
	inline std::istream &operator>>(std::istream &rIStream,
									GenerateNewNodesBeforeMeshingCutPfemProcess &rThis);

	/// output stream function
	inline std::ostream &operator<<(std::ostream &rOStream,
									const GenerateNewNodesBeforeMeshingCutPfemProcess &rThis)
	{
		rThis.PrintInfo(rOStream);
		rOStream << std::endl;
		rThis.PrintData(rOStream);

		return rOStream;
	}
	///@}

} // namespace Kratos.
