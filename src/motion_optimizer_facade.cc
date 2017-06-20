/**
 @file    motion_optimizer.cc
 @author  Alexander W. Winkler (winklera@ethz.ch)
 @date    Nov 20, 2016
 @brief   Brief description
 */

#include <xpp/opt/motion_optimizer_facade.h>

#include <algorithm>
#include <cassert>
#include <deque>
#include <utility>

#include <xpp/cartesian_declarations.h>
#include <xpp/endeffectors.h>

#include <xpp/ipopt_adapter.h>
#include <xpp/opt/com_spline.h>
#include <xpp/opt/cost_constraint_factory.h>
#include <xpp/opt/variables/base_motion.h>
#include <xpp/opt/variables/contact_schedule.h>
#include <xpp/opt/variables/endeffectors_force.h>
#include <xpp/opt/variables/endeffectors_motion.h>
#include <xpp/snopt_adapter.h>

namespace xpp {
namespace opt {

MotionOptimizerFacade::MotionOptimizerFacade ()
{
  opt_variables_ = std::make_shared<Composite>("variables", true);
}

MotionOptimizerFacade::~MotionOptimizerFacade ()
{
}

void
MotionOptimizerFacade::BuildDefaultStartStance ()
{
  State3d base;
  base.lin.p_.z() = 0.58;
  EndeffectorsBool contact_state(motion_parameters_->GetEECount());
  contact_state.SetAll(true);

  start_geom_.SetBase(base);
  start_geom_.SetContactState(contact_state);

  auto ee_start_W = motion_parameters_->GetNominalStanceInBase();
  for (auto ee : ee_start_W.GetEEsOrdered()) {
    ee_start_W.At(ee).z() = 0.0;
  }
  start_geom_.SetEEStateInWorld(kPos, ee_start_W);
}

void
MotionOptimizerFacade::BuildVariables ()
{
  // initialize the contact schedule
  auto contact_schedule = std::make_shared<ContactSchedule>(
      motion_parameters_->GetContactSchedule());

  // initialize the ee_motion with the fixed parameters
  auto ee_motion = std::make_shared<EndeffectorsMotion>(start_geom_.GetEEPos(),
                                                        *contact_schedule);

  double T = motion_parameters_->GetTotalTime();

  auto com_motion = std::make_shared<ComSpline>();
  com_motion->Init(T, motion_parameters_->duration_polynomial_, start_geom_.GetBase().lin);
  auto base_motion = std::make_shared<BaseMotion>(com_motion);
//  base_motion->SetOffsetGeomToCom(motion_parameters_->offset_geom_to_com_);

  auto force = std::make_shared<EndeffectorsForce>(motion_parameters_->load_dt_,
                                                   *contact_schedule);

  std::cout << "load values: " << force->GetLoadValues(0.0) << std::endl;

  opt_variables_->ClearComponents();
  opt_variables_->AddComponent(base_motion);
  opt_variables_->AddComponent(ee_motion);
  opt_variables_->AddComponent(force);
  opt_variables_->AddComponent(contact_schedule);
}

void
MotionOptimizerFacade::SolveProblem (NlpSolver solver)
{
  BuildVariables();

  CostConstraintFactory factory;
  factory.Init(opt_variables_, motion_parameters_, start_geom_, goal_geom_);

  nlp.Init(opt_variables_);

  auto constraints = std::make_unique<Composite>("constraints", true);
  for (ConstraintName name : motion_parameters_->GetUsedConstraints()) {
    constraints->AddComponent(factory.GetConstraint(name));
  }
  constraints->Print();
  nlp.AddConstraint(std::move(constraints));


  auto costs = std::make_unique<Composite>("costs", false);
  for (const auto& pair : motion_parameters_->GetCostWeights()) {
    CostName name = pair.first;
    costs->AddComponent(factory.GetCost(name));
  }
  costs->Print();
  nlp.AddCost(std::move(costs));


  switch (solver) {
    case Ipopt:   IpoptAdapter::Solve(nlp); break;
    case Snopt:   SnoptAdapter::Solve(nlp); break;
    default: assert(false); // solver not implemented
  }
}

MotionOptimizerFacade::RobotStateVec
MotionOptimizerFacade::GetTrajectory (double dt) const
{
  RobotStateVec trajectory;

  auto base_motion      = std::dynamic_pointer_cast<BaseMotion>        (opt_variables_->GetComponent("base_motion"));
  auto ee_motion        = std::dynamic_pointer_cast<EndeffectorsMotion>(opt_variables_->GetComponent("endeffectors_motion"));
  auto contact_schedule = std::dynamic_pointer_cast<ContactSchedule>   (opt_variables_->GetComponent("contact_schedule"));
  auto ee_forces        = std::dynamic_pointer_cast<EndeffectorsForce> (opt_variables_->GetComponent("endeffector_force"));

  double t=0.0;
  double T = motion_parameters_->GetTotalTime();
  while (t<T) {

    RobotStateCartesian state(start_geom_.GetEECount());
    state.SetBase(base_motion->GetBase(t));

    state.SetEEStateInWorld(ee_motion->GetEndeffectors(t));
    state.SetEEForcesInWorld(ee_forces->GetLoadValues(t));
    state.SetContactState(contact_schedule->IsInContact(t));

    state.SetTime(t);
    trajectory.push_back(state);
    t += dt;
  }

  return trajectory;
}

void
MotionOptimizerFacade::SetMotionParameters (const MotionParametersPtr& params)
{
  motion_parameters_ = params;
}

} /* namespace opt */
} /* namespace xpp */


