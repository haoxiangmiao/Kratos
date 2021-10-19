//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Philipp Bucher (https://github.com/philbucher)
//

// System includes
#include<unordered_map>

// External includes

// Project includes
#include "co_sim_io_conversion_utilities.h"

namespace Kratos {
namespace {

//  TODO refactor with switch?
const std::map<GeometryData::KratosGeometryType, CoSimIO::ElementType> elem_type_map {
    {GeometryData::KratosGeometryType::Kratos_Hexahedra3D20,    CoSimIO::ElementType::Hexahedra3D20   },
    {GeometryData::KratosGeometryType::Kratos_Hexahedra3D27,    CoSimIO::ElementType::Hexahedra3D27   },
    {GeometryData::KratosGeometryType::Kratos_Hexahedra3D8,     CoSimIO::ElementType::Hexahedra3D8     },
    {GeometryData::KratosGeometryType::Kratos_Prism3D15,        CoSimIO::ElementType::Prism3D15   },
    {GeometryData::KratosGeometryType::Kratos_Prism3D6,         CoSimIO::ElementType::Prism3D6     },
    {GeometryData::KratosGeometryType::Kratos_Quadrilateral2D4, CoSimIO::ElementType::Quadrilateral2D4     },
    {GeometryData::KratosGeometryType::Kratos_Quadrilateral2D8, CoSimIO::ElementType::Quadrilateral2D8     },
    {GeometryData::KratosGeometryType::Kratos_Quadrilateral2D9, CoSimIO::ElementType::Quadrilateral2D9     },
    {GeometryData::KratosGeometryType::Kratos_Quadrilateral3D4, CoSimIO::ElementType::Quadrilateral3D4     },
    {GeometryData::KratosGeometryType::Kratos_Quadrilateral3D8, CoSimIO::ElementType::Quadrilateral3D8     },
    {GeometryData::KratosGeometryType::Kratos_Quadrilateral3D9, CoSimIO::ElementType::Quadrilateral3D9     },
    {GeometryData::KratosGeometryType::Kratos_Tetrahedra3D10,   CoSimIO::ElementType::Tetrahedra3D10     },
    {GeometryData::KratosGeometryType::Kratos_Tetrahedra3D4,    CoSimIO::ElementType::Tetrahedra3D4   },
    {GeometryData::KratosGeometryType::Kratos_Triangle2D3,      CoSimIO::ElementType::Triangle2D3   },
    {GeometryData::KratosGeometryType::Kratos_Triangle2D6,      CoSimIO::ElementType::Triangle2D6   },
    {GeometryData::KratosGeometryType::Kratos_Triangle3D3,      CoSimIO::ElementType::Triangle3D3   },
    {GeometryData::KratosGeometryType::Kratos_Triangle3D6,      CoSimIO::ElementType::Triangle3D6   },
    {GeometryData::KratosGeometryType::Kratos_Line2D2,          CoSimIO::ElementType::Line2D2   },
    {GeometryData::KratosGeometryType::Kratos_Line2D3,          CoSimIO::ElementType::Line2D3   },
    {GeometryData::KratosGeometryType::Kratos_Line3D2,          CoSimIO::ElementType::Line3D2   },
    {GeometryData::KratosGeometryType::Kratos_Line3D3,          CoSimIO::ElementType::Line3D3   },
    {GeometryData::KratosGeometryType::Kratos_Point2D,          CoSimIO::ElementType::Point2D   },
    {GeometryData::KratosGeometryType::Kratos_Point3D,          CoSimIO::ElementType::Point3D   }
};

//  TODO refactor with switch?
const std::map<CoSimIO::ElementType, std::string> elem_name_map {
    {CoSimIO::ElementType::Hexahedra3D20, "Element3D20N"},
    {CoSimIO::ElementType::Hexahedra3D27, "Element3D27N"},
    {CoSimIO::ElementType::Hexahedra3D8, "Element3D8N"},
    {CoSimIO::ElementType::Prism3D15, "Element3D15N"},
    {CoSimIO::ElementType::Prism3D6, "Element3D6N"},
    {CoSimIO::ElementType::Quadrilateral2D4, "Element2D4N"},
    {CoSimIO::ElementType::Quadrilateral2D8, "Element2D8N"},
    {CoSimIO::ElementType::Quadrilateral2D9, "Element2D9N"},
    {CoSimIO::ElementType::Quadrilateral3D8, "Element3D8N"},
    {CoSimIO::ElementType::Tetrahedra3D10, "Element3D10N"},
    {CoSimIO::ElementType::Tetrahedra3D4, "Element3D4N"},
    {CoSimIO::ElementType::Triangle2D3, "Element2D3N"},
    {CoSimIO::ElementType::Triangle2D6, "Element2D6N"},
    {CoSimIO::ElementType::Triangle3D3, "Element3D3N"},
    {CoSimIO::ElementType::Line2D2, "Element2D2N"},
    {CoSimIO::ElementType::Line3D2, "Element3D2N"},
    {CoSimIO::ElementType::Point2D, "Element2D1N"},
    {CoSimIO::ElementType::Point3D, "Element3D1N"}
};

} // anonymous namespace

void CoSimIOConversionUtilities::CoSimIOModelPartToKratosModelPart(
    const CoSimIO::ModelPart& rCoSimIOModelPart,
    Kratos::ModelPart& rKratosModelPart)
{
    KRATOS_TRY

    // fill ModelPart from received Mesh
    KRATOS_ERROR_IF(rKratosModelPart.NumberOfNodes() > 0) << "ModelPart is not empty, it has nodes!" << std::endl;
    KRATOS_ERROR_IF(rKratosModelPart.NumberOfProperties() > 0) << "ModelPart is not empty, it has properties!" << std::endl;
    KRATOS_ERROR_IF(rKratosModelPart.IsDistributed()) << "ModelPart cannot be distributed!" << std::endl;

    // Need to use aux structure as passing Nodes by Id to CreateNewElement currently sorts the nodes array
    // this would screw up the ordering of Nodes and the data coming from external solvers could no longer be
    // assigned to the  correct nodes!
    std::unordered_map<Kratos::ModelPart::IndexType, Kratos::ModelPart::NodeType::Pointer> kratos_nodes;
    kratos_nodes.reserve(rCoSimIOModelPart.NumberOfNodes());

    // fill ModelPart with received entities
    for (const auto& r_node : rCoSimIOModelPart.Nodes()) {
        kratos_nodes[r_node.Id()] = rKratosModelPart.CreateNewNode(
            r_node.Id(),
            r_node.X(),
            r_node.Y(),
            r_node.Z()
        );
    };

    Properties::Pointer p_props;
    if (rCoSimIOModelPart.NumberOfElements() > 0) {
        p_props = rKratosModelPart.CreateNewProperties(0);
        rKratosModelPart.Elements().reserve(rCoSimIOModelPart.NumberOfElements());
    }

    Geometry<Node<3>>::PointsArrayType conn;
    for (const auto& r_elem : rCoSimIOModelPart.Elements()) {
        conn.clear();

        for (const auto& r_node : r_elem.Nodes()) {
            conn.push_back(kratos_nodes[r_node.Id()]); // no check necessary if Id exists, as the Elements could not have been created in the CoSimIO ModelPart without the nodes in the first place
        };

        auto elem_name_it = elem_name_map.find(r_elem.Type());
        if (elem_name_it == elem_name_map.end()) {
            std::stringstream err;
            err << "No Kratos element found for this element type (" << static_cast<int>(r_elem.Type()) << ")!\nOnly the following types are available:";
            for (const auto& r_type_name_pair : elem_name_map) {
                err << "\n\t" << r_type_name_pair.second;
            }
            KRATOS_ERROR << err.str();
        }

        rKratosModelPart.CreateNewElement(
            elem_name_it->second,
            r_elem.Id(),
            conn,
            p_props
        );
    };

    KRATOS_CATCH("")
}

void CoSimIOConversionUtilities::KratosModelPartToCoSimIOModelPart(
    const Kratos::ModelPart& rKratosModelPart,
    CoSimIO::ModelPart& rCoSimIOModelPart)
{
    KRATOS_TRY

    for (const auto& r_node : rKratosModelPart.Nodes()) {
        rCoSimIOModelPart.CreateNewNode(
            r_node.Id(),
            // TODO: use initial or current coordinates?
            r_node.X0(),
            r_node.Y0(),
            r_node.Z0()
        );
    };

    CoSimIO::ConnectivitiesType conn;
    for (const auto& r_elem : rKratosModelPart.Elements()) {
        const auto& r_geom = r_elem.GetGeometry();
        if (conn.size() != r_geom.PointsNumber()) {
            conn.resize(r_geom.PointsNumber());
        };

        for (std::size_t i=0; i<r_geom.PointsNumber(); ++i) {
            conn[i] = r_geom[i].Id();
        }

        auto elem_type_it = elem_type_map.find(r_geom.GetGeometryType());
        KRATOS_ERROR_IF(elem_type_it == elem_type_map.end()) << "No CoSimIO element type found for this Kratos element type (" << static_cast<int>(r_geom.GetGeometryType()) << ")!" << std::endl;

        rCoSimIOModelPart.CreateNewElement(
            r_elem.Id(),
            elem_type_it->second,
            conn
        );
    };

    KRATOS_CATCH("")
}

CoSimIO::Info CoSimIOConversionUtilities::InfoFromParameters(const Parameters rSettings)
{
    KRATOS_TRY

    CoSimIO::Info info;

    for (auto it = rSettings.begin(); it != rSettings.end(); ++it) {
        if      (it->IsString())       info.Set<std::string>(it.name(),   it->GetString());
        else if (it->IsInt())          info.Set<int>(it.name(),           it->GetInt());
        else if (it->IsBool())         info.Set<bool>(it.name(),          it->GetBool());
        else if (it->IsDouble())       info.Set<double>(it.name(),        it->GetDouble());
        else if (it->IsSubParameter()) info.Set<CoSimIO::Info>(it.name(), InfoFromParameters(*it));
        else KRATOS_WARNING("Kratos-CoSimIO") << "Setting with name \"" << it.name() << "\" cannot be converted to CoSimIO::Info and is ignored!" << std::endl;
    }

    return info;

    KRATOS_CATCH("")
}

}  // namespace Kratos.
