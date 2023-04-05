//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Ruben Zorrilla
//                   Philipp Bucher
//

#pragma once

// System includes
#include <pybind11/pybind11.h>

// External includes

// Project includes

namespace Kratos::Python
{

void AddConvergenceAcceleratorsToPython(pybind11::module& m);

}  // namespace Kratos::Python.
