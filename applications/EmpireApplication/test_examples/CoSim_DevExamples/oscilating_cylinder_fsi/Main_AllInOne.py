from __future__ import print_function, absolute_import, division  # makes KratosMultiphysics backward compatible with python 2.6 and 2.7

import KratosMultiphysics
import KratosMultiphysics.EmpireApplication

# imports Kratos tools
import co_simulation_tools as cs_tools
from co_simulation_tools import csprint, bold, CheckCoSimulationSettingsAndAssignDefaults

# import Kratos components
from co_simulation_predictors.co_simulation_predictor_factory import CreatePredictor
from co_simulation_convergence_accelerators.co_simulation_convergence_accelerator_factory import CreateConvergenceAccelerator
from co_simulation_convergence_criteria.co_simulation_convergence_criteria_factory import CreateConvergenceCriteria

### WORK HERE: imports to be resolved
import co_simulation_solvers.python_solvers_wrapper_co_simulation as solvers_wrapper

# imports from basic Python
import json
import sys

# read parameter file(s)
parameter_file_name = "project_parameters_cosim_oscilating_cylinder_fsi.json"
with open(parameter_file_name, 'r') as parameter_file:
    cosim_settings = json.load(parameter_file)


### CoSimulationAnalysis - init ###################################
if type(cosim_settings) != dict:
    raise Exception("Input is expected to be provided as a python dictionary")

CheckCoSimulationSettingsAndAssignDefaults(cosim_settings)

problem_data = cosim_settings["problem_data"]

cs_tools.PRINT_COLORS = problem_data["print_colors"]

parallel_type = problem_data["parallel_type"]
if parallel_type == "OpenMP":
    flush_stdout = True
    cs_tools.PRINTING_RANK = True
elif parallel_type == "MPI":
    from co_simulation_mpi_space import CoSimulationMPISpace

    cs_tools.COSIM_SPACE = CoSimulationMPISpace()
    cs_tools.PRINTING_RANK = (cs_tools.COSIM_SPACE.Rank() == 0)
else:
    raise Exception('"parallel_type" can only be "OpenMP" or "MPI"!')

flush_stdout = problem_data["flush_stdout"]
echo_level = problem_data["echo_level"]


### Initialize ###################################
cosim_solver_settings = cosim_settings["solver_settings"]

## CoSimulationBaseSolver __init__() ###################################
echo_level = 0
solver_level = 0
if "echo_level" in cosim_solver_settings:
    echo_level = cosim_solver_settings["echo_level"]
io_is_initialized = False

## CoSimulationBaseCouplingSolver __init__() ###################################
solver_names = []
solvers = {}

for solver_settings in cosim_solver_settings["coupling_loop"]:
    solver_name = solver_settings["name"]
    if solver_name in solver_names:
        raise NameError('Solver name "' + solver_name + '" defined twice!')
    solver_names.append(solver_name)
    cosim_solver_settings["solvers"][solver_name]["name"] = solver_name # adding the name such that the solver can identify itself
    solvers[solver_name] = solvers_wrapper.CreateSolver(
        cosim_solver_settings["solvers"][solver_name], solver_level-1) # -1 to have solver prints on same lvl

cosim_solver_details = cs_tools.GetSolverCoSimulationDetails(
    cosim_solver_settings["coupling_loop"])

predictor = None
if "predictor_settings" in cosim_solver_settings:
    predictor = CreatePredictor(cosim_solver_settings["predictor_settings"], solvers, solver_level)
    predictor.SetEchoLevel(echo_level)

# With this setting the coupling can start later
start_coupling_time = 0.0
if "start_coupling_time" in cosim_solver_settings:
    start_coupling_time = cosim_solver_settings["start_coupling_time"]
if start_coupling_time > 0.0:
    coupling_started = False
else:
    coupling_started = True

## GaussSeidelStrongCouplingSolver __init__() ###################################
if not len(cosim_solver_settings["solvers"]) == 2:
    raise Exception("Exactly two solvers have to be specified for the GaussSeidelStrongCouplingSolver!")

convergence_accelerator = CreateConvergenceAccelerator(
    cosim_solver_settings["convergence_accelerator_settings"],
    solvers, solver_level)
convergence_accelerator.SetEchoLevel(echo_level)

convergence_criteria = CreateConvergenceCriteria(
    cosim_solver_settings["convergence_criteria_settings"],
    solvers, solver_level)
convergence_criteria.SetEchoLevel(echo_level)

num_coupling_iterations = cosim_solver_settings["num_coupling_iterations"]

## GaussSeidelStrongCouplingSolver Initialize() ###################################
# from super -------------------
for solver_name in solver_names:
    solvers[solver_name].Initialize()
for solver_name in solver_names:
    solvers[solver_name].InitializeIO(solvers, echo_level)

if predictor is not None:
    predictor.Initialize()
# from sub -------------------
convergence_accelerator.Initialize()
convergence_criteria.Initialize()

## GaussSeidelStrongCouplingSolver Check() ###################################
# from super -------------------
for solver_name in solver_names:
    solvers[solver_name].Check()

if predictor is not None:
    predictor.Check()

# from sub -------------------
convergence_accelerator.Check()
convergence_criteria.Check()


if echo_level > 0:
    ## GaussSeidelStrongCouplingSolver PrintInfo() ###################################
    # from super -------------------
    cs_tools.couplingsolverprint("GaussSeidelStrongCouplingSolver", "Has the following participants:")
    for solver_name in solver_names:
        solvers[solver_name].PrintInfo()

    if predictor is not None:
        cs_tools.couplingsolverprint("GaussSeidelStrongCouplingSolver", "Uses a Predictor:")
        predictor.PrintInfo()

    # from sub -------------------
    cs_tools.couplingsolverprint("GaussSeidelStrongCouplingSolver", "Uses the following objects:")
    convergence_accelerator.PrintInfo()
    convergence_criteria.PrintInfo()

# Stepping and time settings
end_time = cosim_settings["problem_data"]["end_time"]
time = cosim_settings["problem_data"]["start_time"]
step = 0

if flush_stdout:
    sys.stdout.flush()

### RunSolutionLoop ###################################
print("")
while time < end_time:
    print("")
    step += 1








    ################################### BAUSTELLE ###################################TODO
    ### !!! solver is a GaussSeidelStrongCouplingSolver with self.lvl = solver_level

    time = solver.AdvanceInTime(time)

    ## InitializeSolutionStep ###################################
    csprint(0, bold("time={0:.12g}".format(time) + " | step=" + str(step)))
    solver.InitializeSolutionStep()

    solver.Predict()
    solver.SolveSolutionStep()

    ## FinalizeSolutionStep ###################################
    solver.FinalizeSolutionStep()

    ## OutputSolutionStep ###################################
    solver.OutputSolutionStep()

    if flush_stdout:
        sys.stdout.flush()

solver.Finalize()




