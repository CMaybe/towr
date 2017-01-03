/**
 @file    step_sequence_planner.h
 @author  Alexander W. Winkler (winklera@ethz.ch)
 @date    Jul 21, 2016
 @brief   Declares the interface to the StepSequencePlanner.
 */

#ifndef USER_TASK_DEPENDS_XPP_OPT_INCLUDE_XPP_ZMP_STEP_SEQUENCE_PLANNER_H_
#define USER_TASK_DEPENDS_XPP_OPT_INCLUDE_XPP_ZMP_STEP_SEQUENCE_PLANNER_H_

#include <xpp/utils/state.h>
#include <xpp/opt/contact.h>
#include <xpp/hyq/hyq_robot_interface.h>

namespace xpp {
namespace hyq {


/** Plans the sequence of steps (LH, LF, ...) for a given optimization problem.
  *
  * This is information that has to be passed to the NLP optimizer beforehand.
  * This class is specific for HyQ and must be adapted for different robots.
  * It contains all the information needed to determine the step sequence.
  */
class StepSequencePlanner {
public:
  typedef std::vector<LegID> LegIDVec;
  typedef std::vector<xpp::opt::Contact> VecFoothold;
  typedef xpp::utils::StateLin2d State;
  typedef Eigen::Vector2d Vector2d;

  StepSequencePlanner ();
  virtual ~StepSequencePlanner ();

  /** Necessary information to determine the step sequence.
    *
    * @param the current pos/vel/acc of the robot.
    * @param the desired pos/vel/acc that the robot should achieve.
    * @param start_stance the 4 footholds right before swinging the first leg.
    * @param robot_height the walking height [m] of the robot.
    */
  void Init(const State& curr, const State& goal,
            const VecFoothold& start_stance, double robot_height,
            double max_step_lenght_,
            int swingleg_of_last_spline);

  /** Determines whether an initial stance phase is inserted.
    *
    * This is necessary, if the initial ZMP is outside the area of support
    * of the first step, so the optimizer would not be able to find a solution.
    *
    * @param step sequence the order of swing legs.
    * @return true if stance phase must be inserted.
    */
  bool StartWithStancePhase() const;

  /** Determines the sequence of steps (LH, LF, ...) to take.
    *
    * @param swingleg_of_last_spline The leg of the last spline that was completed. Could also be a stance spline.
    * @return vector of swinglegs.
    */
  LegIDVec DetermineStepSequence();

private:
  bool IsStepNecessary() const;
  LegID NextSwingLeg(LegID curr) const;
  LegID NextSwingLegBackwards(LegID curr) const;
  LegID NextSwingLegTrott(LegID curr) const;

  bool IsGoalOutsideRangeOfMotion() const;

  State curr_state_;
  State goal_state_;
  VecFoothold start_stance_;
  double robot_height_;
  double max_step_length_;

  HyqRobotInterface robot_;

  int curr_swingleg_; // this could also be no swingleg (stance phase)
};

} /* namespace hyq */
} /* namespace xpp */

#endif /* USER_TASK_DEPENDS_XPP_OPT_INCLUDE_XPP_ZMP_STEP_SEQUENCE_PLANNER_H_ */
