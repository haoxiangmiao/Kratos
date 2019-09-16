//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Philipp Bucher
//

// Project includes
#include "geometries/line_2d_2.h"
#include "geometries/triangle_3d_3.h"
#include "geometries/tetrahedra_3d_4.h"
#include "testing/testing.h"
#include "custom_mappers/barycentric_mapper.h"
#include "includes/stream_serializer.h"
#include "custom_utilities/mapper_utilities.h"
#include "mapping_application_variables.h"

namespace Kratos {
namespace Testing {


typedef typename MapperLocalSystem::MatrixType MatrixType;
typedef typename MapperLocalSystem::EquationIdVectorType EquationIdVectorType;

typedef Node<3> NodeType;

KRATOS_TEST_CASE_IN_SUITE(BarycentricInterfaceInfo_BasicTests, KratosMappingApplicationSerialTestSuite)
{
    const Point coords(1.0, 2.45, -23.8);

    const std::size_t source_local_sys_idx = 123;
    const std::size_t dummy_rank = 78;

    BarycentricInterfaceInfo barycentric_info(coords, source_local_sys_idx, 0, 2);

    const auto barycentric_info_1(barycentric_info.Create());
    const auto barycentric_info_2(barycentric_info.Create(coords, source_local_sys_idx, dummy_rank));

    // Test if the "Create" function returns the correct object
    KRATOS_CHECK_EQUAL(typeid(barycentric_info), typeid(*barycentric_info_1));
    KRATOS_CHECK_EQUAL(typeid(barycentric_info), typeid(*barycentric_info_2));
}

KRATOS_TEST_CASE_IN_SUITE(BarycentricInterfaceInfo_simple_line_interpolation, KratosMappingApplicationSerialTestSuite)
{
    Point coords(0.4, 0.0, 0.0);

    std::size_t source_local_sys_idx = 123;

    BarycentricInterfaceInfo barycentric_info(coords, source_local_sys_idx, 0, 2);

    auto node_1(Kratos::make_intrusive<NodeType>(1,  3.3, 0.0, 0.0)); // third closest (not used)
    auto node_2(Kratos::make_intrusive<NodeType>(3,  1.0, 0.1, -0.2)); // second closest
    auto node_3(Kratos::make_intrusive<NodeType>(15, 0.3, 0.0, 0.0)); // closest

    InterfaceObject::Pointer interface_node_1(Kratos::make_shared<InterfaceNode>(node_1.get()));
    InterfaceObject::Pointer interface_node_2(Kratos::make_shared<InterfaceNode>(node_2.get()));
    InterfaceObject::Pointer interface_node_3(Kratos::make_shared<InterfaceNode>(node_3.get()));

    node_1->SetValue(INTERFACE_EQUATION_ID, 13);
    node_2->SetValue(INTERFACE_EQUATION_ID, 5);
    node_3->SetValue(INTERFACE_EQUATION_ID, 108);

    // distances are not used internally!
    barycentric_info.ProcessSearchResult(*interface_node_1, 0.0);
    barycentric_info.ProcessSearchResult(*interface_node_2, 0.0);
    barycentric_info.ProcessSearchResult(*interface_node_3, 0.0);

    KRATOS_CHECK(barycentric_info.GetLocalSearchWasSuccessful());
    KRATOS_CHECK_IS_FALSE(barycentric_info.GetIsApproximation());

    std::vector<int> found_ids;
    barycentric_info.GetValue(found_ids, MapperInterfaceInfo::InfoType::Dummy);
    KRATOS_CHECK_EQUAL(found_ids.size(), 2);
    KRATOS_CHECK_EQUAL(found_ids[0], 108);
    KRATOS_CHECK_EQUAL(found_ids[1], 5);

    std::vector<double> neighbor_coords;
    barycentric_info.GetValue(neighbor_coords, MapperInterfaceInfo::InfoType::Dummy);
    KRATOS_CHECK_EQUAL(neighbor_coords.size(), 6);
    KRATOS_CHECK_DOUBLE_EQUAL(neighbor_coords[0], 0.3);  // X
    KRATOS_CHECK_DOUBLE_EQUAL(neighbor_coords[1], 0.0);  // Y
    KRATOS_CHECK_DOUBLE_EQUAL(neighbor_coords[2], 0.0);  // Z
    KRATOS_CHECK_DOUBLE_EQUAL(neighbor_coords[3], 1.0);  // X
    KRATOS_CHECK_DOUBLE_EQUAL(neighbor_coords[4], 0.1);  // Y
    KRATOS_CHECK_DOUBLE_EQUAL(neighbor_coords[5], -0.2); // Z
}

KRATOS_TEST_CASE_IN_SUITE(BarycentricInterfaceInfo_line_duplicated_point, KratosMappingApplicationSerialTestSuite)
{
    // test to make sure that if several points share the same coords only one of them is used!

    Point coords(0.4, 0.0, 0.0);

    std::size_t source_local_sys_idx = 123;

    BarycentricInterfaceInfo barycentric_info(coords, source_local_sys_idx, 0, 2);

    auto node_1(Kratos::make_intrusive<NodeType>(1,  3.3, 0.0, 0.0)); // third closest (not used)
    auto node_2(Kratos::make_intrusive<NodeType>(3,  1.0, 0.1, -0.2)); // second closest
    auto node_3(Kratos::make_intrusive<NodeType>(15, 0.3, 0.0, 0.0)); // closest
    auto node_4(Kratos::make_intrusive<NodeType>(16, 0.3, 0.0, 0.0)); // closest (but not used bcs duplicate)

    InterfaceObject::Pointer interface_node_1(Kratos::make_shared<InterfaceNode>(node_1.get()));
    InterfaceObject::Pointer interface_node_2(Kratos::make_shared<InterfaceNode>(node_2.get()));
    InterfaceObject::Pointer interface_node_3(Kratos::make_shared<InterfaceNode>(node_3.get()));
    InterfaceObject::Pointer interface_node_4(Kratos::make_shared<InterfaceNode>(node_4.get()));

    node_1->SetValue(INTERFACE_EQUATION_ID, 13);
    node_2->SetValue(INTERFACE_EQUATION_ID, 5);
    node_3->SetValue(INTERFACE_EQUATION_ID, 108);
    node_4->SetValue(INTERFACE_EQUATION_ID, 32);

    // distances are not used internally!
    barycentric_info.ProcessSearchResult(*interface_node_1, 0.0);
    barycentric_info.ProcessSearchResult(*interface_node_2, 0.0);
    barycentric_info.ProcessSearchResult(*interface_node_3, 0.0);
    barycentric_info.ProcessSearchResult(*interface_node_4, 0.0);

    KRATOS_CHECK(barycentric_info.GetLocalSearchWasSuccessful());
    KRATOS_CHECK_IS_FALSE(barycentric_info.GetIsApproximation());

    std::vector<int> found_ids;
    barycentric_info.GetValue(found_ids, MapperInterfaceInfo::InfoType::Dummy);
    KRATOS_CHECK_EQUAL(found_ids.size(), 2);
    KRATOS_CHECK_EQUAL(found_ids[0], 108);
    KRATOS_CHECK_EQUAL(found_ids[1], 5);

    std::vector<double> neighbor_coords;
    barycentric_info.GetValue(neighbor_coords, MapperInterfaceInfo::InfoType::Dummy);
    KRATOS_CHECK_EQUAL(neighbor_coords.size(), 6);
    KRATOS_CHECK_DOUBLE_EQUAL(neighbor_coords[0], 0.3);  // X
    KRATOS_CHECK_DOUBLE_EQUAL(neighbor_coords[1], 0.0);  // Y
    KRATOS_CHECK_DOUBLE_EQUAL(neighbor_coords[2], 0.0);  // Z
    KRATOS_CHECK_DOUBLE_EQUAL(neighbor_coords[3], 1.0);  // X
    KRATOS_CHECK_DOUBLE_EQUAL(neighbor_coords[4], 0.1);  // Y
    KRATOS_CHECK_DOUBLE_EQUAL(neighbor_coords[5], -0.2); // Z
}

}  // namespace Testing
}  // namespace Kratos