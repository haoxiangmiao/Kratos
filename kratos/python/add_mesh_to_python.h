//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Riccardo Rossi
//                   Pooyand Dadvand
//

#pragma once

// System includes
#include <pybind11/pybind11.h>

// External includes

// Project includes

namespace Kratos
{

namespace Python
{

void  AddNodeToPython(pybind11::module& m);
void  AddPropertiesToPython(pybind11::module& m);
void  AddMeshToPython(pybind11::module& m);

}  // namespace Python.

}  // namespace Kratos.
