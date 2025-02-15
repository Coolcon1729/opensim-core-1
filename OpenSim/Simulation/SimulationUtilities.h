#ifndef OPENSIM_SIMULATION_UTILITIES_H_
#define OPENSIM_SIMULATION_UTILITIES_H_
/* -------------------------------------------------------------------------- *
 *                     OpenSim:  SimulationUtilities.h                        *
 * -------------------------------------------------------------------------- *
 * The OpenSim API is a toolkit for musculoskeletal modeling and simulation.  *
 * See http://opensim.stanford.edu and the NOTICE file for more information.  *
 * OpenSim is developed at Stanford University and supported by the US        *
 * National Institutes of Health (U54 GM072970, R24 HD065690) and by DARPA    *
 * through the Warrior Web program.                                           *
 *                                                                            *
 * Copyright (c) 2005-2018 Stanford University and the Authors                *
 * Author(s): OpenSim Team                                                    *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may    *
 * not use this file except in compliance with the License. You may obtain a  *
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0.         *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */

#include "StatesTrajectory.h"
#include "osimSimulationDLL.h"
#include <regex>

#include <SimTKcommon/internal/State.h>

#include <OpenSim/Common/Reporter.h>
#include <OpenSim/Common/Storage.h>
#include <OpenSim/Simulation/Model/Model.h>

namespace OpenSim {

/** Simulate a model from an initial state and return the final state.
    If the model's useVisualizer flag is true, the user is repeatedly prompted
    to either begin simulating or quit. The provided state is not updated but
    the final state is returned at the end of the simulation, when finalTime is
    reached. %Set saveStatesFile=true to save the states to a storage file as:
    "<model_name>_states.sto".
    @ingroup simulationutil */
OSIMSIMULATION_API SimTK::State simulate(Model& model,
    const SimTK::State& initialState,
    double finalTime,
    bool saveStatesFile = false);

/// Update a vector of state labels (in place) to use post-4.0 state paths
/// instead of pre-4.0 state names. For example, this converts labels as
/// follows:
///   - `pelvis_tilt` -> `/jointset/ground_pelvis/pelvis_tilt/value`
///   - `pelvis_tilt_u` -> `/jointset/ground_pelvis/pelvis_tilt/speed`
///   - `soleus.activation` -> `/forceset/soleus/activation`
///   - `soleus.fiber_length` -> `/forceset/soleus/fiber_length`
/// This can also be used to update the column labels of an Inverse
/// Kinematics Tool solution MOT file so that the data can be used as
/// states. If a label does not identify a state in the model, the column
/// label is not changed.
/// @throws Exception if labels are not unique.
/// @ingroup simulationutil
OSIMSIMULATION_API void updateStateLabels40(
        const Model& model, std::vector<std::string>& labels);

#ifndef SWIG
/** Not available through scripting. 
 @returns nullptr if no update is necessary.
 @ingroup simulationutil */
OSIMSIMULATION_API std::unique_ptr<Storage>
updatePre40KinematicsStorageFor40MotionType(const Model& pre40Model,
        const Storage &kinematics);
#endif // SWIG
    
/** This function can be used to upgrade MOT files generated with versions
    before 4.0 in which some data columns are associated with coordinates
    that were incorrectly marked as Rotational (rather than Coupled). Specific
    instances of the issue are the patella coordinate in the Rajagopal 2015 and
    leg6dof9musc models. In these cases, the patella will visualize incorrectly
    in the GUI when replaying the kinematics from the MOT file, and Static
    Optimization will yield incorrect results.
 
    The new files are written to the same directories as the original files,
    but with the provided suffix (before the file extension). To overwrite your
    original files, set the suffix to an emtpty string.
 
    If the file does not need to be updated, no new file is written.
 
    Conversion of the data only occurs for files in degrees ("inDegrees=yes"
    in the header).
 
    Do not use this function with MOT files generated by 4.0 or later; doing
    so will cause your data to be altered incorrectly. We do not detect whether
    or not your MOT file is pre-4.0.
 
    In OpenSim 4.0, MotionTypes for
    Coordinates are now determined strictly by the coordinates' owning Joint.
    In older models, the MotionType, particularly for CustomJoints, were user-
    specified. That entailed in some cases, incorrectly labeling a Coordinate
    as being Rotational, for example, when it is in fact Coupled. For the above
    models, for example, the patella Coordinate had been user-specified to be
    Rotational, but the angle of the patella about the Z-axis of the patella
    body, is a spline function (e.g. coupled function) of the patella
    Coordinate. Thus, the patella Coordinate is not an angle measurement
    and is not classified as Rotational. Use this utility to remove any unit
    conversions from Coordinates that were incorrectly labeled
    as Rotational in the past. For these Coordinates only, the utility will undo
    the incorrect radians to degrees conversion.
    @ingroup simulationutil */
OSIMSIMULATION_API
void updatePre40KinematicsFilesFor40MotionType(const Model& model,
        const std::vector<std::string>& filePaths,
        std::string suffix="_updated");

/** This function attempts to update the connectee path for any Socket anywhere
 * in the model whose connectee path does not point to an existing component.
 * The paths are updated by searching the model for a component with the
 * correct name. For example, a connectee path like
 * `../../some/invalid/path/to/foo` will be updated to `/bodyset/foo` if a Body
 * named `foo` exists in the Model's BodySet. If a socket specifies a Body `foo` and
 * more than one Body `foo` exists in the model, we emit a warning and the
 * socket that specified `foo` is not altered.
 *
 * This method is intended for use with models loaded from version-30516 XML
 * files to bring them up to date with the 4.0 interface.
 * @ingroup simulationutil
 * */
OSIMSIMULATION_API
void updateSocketConnecteesBySearch(Model& model);

/// The map provides the index of each state variable in
/// SimTK::State::getY() from its each state variable path string.
/// Empty slots in Y (e.g., for quaternions) are ignored.
/// @ingroup simulationutil
OSIMSIMULATION_API
std::vector<std::string> createStateVariableNamesInSystemOrder(
        const Model& model);

#ifndef SWIG
/// Same as above, but you can obtain a map from the returned state variable
/// names to the index in SimTK::State::getY() that accounts for empty slots
/// in Y.
/// @ingroup simulationutil
OSIMSIMULATION_API
std::vector<std::string> createStateVariableNamesInSystemOrder(
        const Model& model, std::unordered_map<int, int>& yIndexMap);

/// The map provides the index of each state variable in
/// SimTK::State::getY() from its state variable path string.
/// @ingroup simulationutil
OSIMSIMULATION_API
std::unordered_map<std::string, int> createSystemYIndexMap(const Model& model);
#endif

/// Create a vector of control names based on the actuators in the model for
/// which appliesForce == True. For actuators with one control (e.g.
/// ScalarActuator) the control name is simply the actuator name. For actuators
/// with multiple controls, each control name is the actuator name appended by
/// the control index (e.g. "/actuator_0"); modelControlIndices has length equal
/// to the number of controls associated with actuators that apply a force
/// (appliesForce == True). Its elements are the indices of the controls in the
/// Model::updControls() that are associated with actuators that apply a force.
/// @ingroup simulationutil
OSIMSIMULATION_API
std::vector<std::string> createControlNamesFromModel(
        const Model& model, std::vector<int>& modelControlIndices);
/// Same as above, but when there is no mapping to the modelControlIndices.
/// @ingroup simulationutil
OSIMSIMULATION_API
std::vector<std::string> createControlNamesFromModel(const Model& model);
/// The map provides the index of each control variable in the SimTK::Vector
/// returned by Model::getControls(), using the control name as the
/// key.
/// @throws Exception if the order of actuators in the model does not match
///     the order of controls in Model::getControls(). This is an internal
///     error, but you may be able to avoid the error by ensuring all Actuator%s
///     are in the Model's ForceSet.
/// @ingroup simulationutil
OSIMSIMULATION_API
std::unordered_map<std::string, int> createSystemControlIndexMap(
        const Model& model);

/// Throws an exception if the order of the controls in the model is not the
/// same as the order of the actuators in the model.
/// @ingroup simulationutil
OSIMSIMULATION_API void checkOrderSystemControls(const Model& model);

/// Throws an exception if any label in the provided list does not match any
/// state variable names in the model.
/// @ingroup simulationutil
OSIMSIMULATION_API void checkLabelsMatchModelStates(
        const Model& model, const std::vector<std::string>& labels);

/// Calculate the requested outputs using the model in the problem and the
/// provided states and controls tables.
/// The controls table is used to set the model's controls vector.
/// We assume the states and controls tables contain the same time points.
/// The output paths can be regular expressions. For example,
/// ".*activation" gives the activation of all muscles.
///
/// The output paths must correspond to outputs that match the type provided in
/// the template argument, otherwise they are not included in the report.
///
/// Controls missing from the controls table are given a value of 0.
///
/// If you analysis depends on the values of discrete variables in the state,
/// you may provide those values via the optional argument
/// "discreteVariablesTable". This table should contain column labels with the
/// following format: <path_to_component>/<discrete_var_name>. For example,
/// "/forceset/muscle/implicitderiv_normalized_tendon_force".
///
/// @note The provided trajectory is not modified to satisfy kinematic
/// constraints, but SimTK::Motions in the Model (e.g., PositionMotion) are
/// applied. Therefore, this function expects that you've provided a trajectory
/// that already satisfies kinematic constraints. If your provided trajectory
/// does not satisfy kinematic constraints, many outputs will be incorrect.
/// For example, in a model with a patella whose location is determined by a
/// CoordinateCouplerConstraint, the length of a muscle that crosses the patella
/// will be incorrect.
/// @ingroup simulationutil
template <typename T>
TimeSeriesTable_<T> analyze(Model model, const TimeSeriesTable& statesTable,
        const TimeSeriesTable& controlsTable,
        const std::vector<std::string>& outputPaths,
        const TimeSeriesTable& discreteVariablesTable = {}) {

    // Initialize the system so we can access the outputs.
    model.initSystem();
    // Create the reporter object to which we'll add the output data to create
    // the report.
    auto* reporter = new TableReporter_<T>();
    // Loop through all the outputs for all components in the model, and if
    // the output path matches one provided in the argument and the output type
    // agrees with the template argument type, add it to the report.
    for (const auto& comp : model.getComponentList()) {
        for (const auto& outputName : comp.getOutputNames()) {
            const auto& output = comp.getOutput(outputName);
            auto thisOutputPath = output.getPathName();
            for (const auto& outputPathArg : outputPaths) {
                if (std::regex_match(
                        thisOutputPath, std::regex(outputPathArg))) {
                    // Make sure the output type agrees with the template.
                    if (dynamic_cast<const Output<T>*>(&output)) {
                        log_debug("Adding output {} of type {}.",
                                output.getPathName(), output.getTypeName());
                        reporter->addToReport(output);
                    } else {
                        log_warn("Ignoring output {} of type {}.",
                                output.getPathName(), output.getTypeName());
                    }
                }
            }
        }
    }
    model.addComponent(reporter);
    model.initSystem();

    const auto statesTraj =
            StatesTrajectory::createFromStatesTable(model, statesTable);

    const std::vector<std::string>& controlNames =
            controlsTable.getColumnLabels();
    const std::unordered_map<std::string, int> controlMap =
            createSystemControlIndexMap(model);
    SimTK::Vector controls((int)controlsTable.getNumColumns(), 0.0);

    OPENSIM_THROW_IF(statesTable.getNumRows() != controlsTable.getNumRows(),
            Exception,
            "Expected statesTable and controlsTable to contain the "
            "same number of rows, but statesTable contains {} rows "
            "and controlsTable contains {} rows.",
            statesTable.getNumRows(), controlsTable.getNumRows());

    // If the table for discrete variables was provided, get references
    // to the components associated with each discrete variable.
    std::vector<std::pair<std::string, SimTK::ReferencePtr<const Component>>>
    discreteComponentRefs;
    if (discreteVariablesTable.getNumColumns()) {
        OPENSIM_THROW_IF(discreteVariablesTable.getNumRows() !=
                         statesTable.getNumRows(), Exception,
            "Expected discreteVariablesTable to contain the "
            "same number of rows as statesTable and controlsTable, "
            "but discreteVariablesTable contains {} rows "
            "and statesTable contains {} rows.",
            discreteVariablesTable.getNumRows(), statesTable.getNumRows());

        // The labels for each discrete variable are in the following format:
        //      <path_to_component>/<discrete_var_name>
        // We can use ComponentPath to split up the component path from the
        // discrete variable name.
        for (const auto& label : discreteVariablesTable.getColumnLabels()) {
            ComponentPath discreteVarPath(label);
            const std::string& discreteVarName =
                    discreteVarPath.getComponentName();
            const std::string& componentPath =
                    discreteVarPath.getParentPathString();
            const auto& component = model.getComponent(componentPath);
            discreteComponentRefs.emplace_back(discreteVarName, &component);
        }
    }

    // Loop through the states trajectory to create the report.
    for (int itime = 0; itime < (int)statesTraj.getSize(); ++itime) {
        // Get the current state.
        auto state = statesTraj[itime];

        // Enforce any SimTK::Motion's included in the model.
        model.getSystem().prescribe(state);

        // Create a SimTK::Vector of the control values for the current state.
        const auto& controlsRow = controlsTable.getRowAtIndex(itime);
        for (int icontrol = 0; icontrol < (int)controlNames.size();
                ++icontrol) {
            controls[controlMap.at(controlNames[icontrol])] =
                    controlsRow[icontrol];
        }

        // Set the controls on the state object.
        model.realizeVelocity(state);
        model.setControls(state, controls);

        // Apply discrete variables to the state.
        if (discreteVariablesTable.getNumColumns()) {
           const auto& labels = discreteVariablesTable.getColumnLabels();
            for (int idv = 0; idv < (int)labels.size(); ++idv) {
                const auto& label = labels[idv];
                const auto& discreteCol =
                        discreteVariablesTable.getDependentColumn(label);
                const auto& component = discreteComponentRefs[idv].second.getRef();
                component.setDiscreteVariableValue(state,
                        discreteComponentRefs[idv].first, discreteCol[itime]);
            }
        }

        // Generate report results for the current state.
        model.realizeReport(state);
    }

    return reporter->getTable();
}

/// Calculate "synthetic" acceleration signals equivalent to signals recorded
/// from inertial measurement units (IMUs). First, this utility computes the
/// linear acceleration for each frame included in 'framePaths' using Frame's
/// 'linear_acceleration' Output. Then, to mimic acceleration signals measured
/// from IMUs, the model's gravitational acceleration vector is subtracted from
/// the linear accelerations and the resulting accelerations are re-expressed in
/// the bases of the associated Frame%s.
///
/// @note The linear acceleration Output%s are computed using the analyze()
/// simulation utility, and therefore the 'statesTable' and 'controlsTable'
/// arguments must contain the same time points and we assume that the states
/// obey any kinematic constraints in the Model.
///
/// @note The passed in model must have the correct mass and inertia properties
/// included, since computing accelerations requires realizing to
/// SimTK::Stage::Acceleration which depends on SimTK::Stage::Dynamics.
///
/// @ingroup simulationutil
OSIMSIMULATION_API TimeSeriesTableVec3 createSyntheticIMUAccelerationSignals(
        const Model& model,
        const TimeSeriesTable& statesTable, const TimeSeriesTable& controlsTable,
        const std::vector<std::string>& framePaths);

} // end of namespace OpenSim

#endif // OPENSIM_SIMULATION_UTILITIES_H_
