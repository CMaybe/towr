/**
 @file    contact_schedule.h
 @author  Alexander W. Winkler (winklera@ethz.ch)
 @date    Apr 5, 2017
 @brief   Brief description
 */

#ifndef XPP_OPT_INCLUDE_XPP_OPT_CONTACT_SCHEDULE_H_
#define XPP_OPT_INCLUDE_XPP_OPT_CONTACT_SCHEDULE_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <xpp/endeffectors.h>
#include <xpp/opt/bound.h>
#include <xpp/opt/constraints/composite.h>

namespace xpp {
namespace opt {


class ContactSchedule : public Component {
public:
  using SwingPhase    = std::pair<EndeffectorsBool, double>; // swinglegs and time
  using SwingPhaseVec = std::vector<SwingPhase>;
  using Phase        = std::pair<bool, double>; // contact state and duration
  using PhaseVec     = std::vector<Phase>;

  ContactSchedule (EndeffectorID ee, double t_total, const SwingPhaseVec& phases);
  ContactSchedule (EndeffectorID ee, double t_total, int n_phases);
  ContactSchedule (EndeffectorID ee, const std::vector<double>& timings);
  virtual ~ContactSchedule ();

  bool IsInContact(double t_global) const;

  std::vector<double> GetTimePerPhase() const;

  // zmp_ make these std::vectors?
  virtual VectorXd GetValues() const override;
  virtual void SetValues(const VectorXd&) override;
  VecBound GetBounds () const override;
//  int GetPhaseCount() const { return durations_.size(); };

  Jacobian GetJacobianOfPos(const VectorXd& duration_deriv,
                            const VectorXd& current_vel,
                            double t_global) const;

private:
  void SetPhaseSequence (const SwingPhaseVec& phases, EndeffectorID ee);
  bool GetContact(int phase) const;

  bool first_phase_in_contact_ = true;
  double t_total_;

  std::vector<double> durations_;
};



/** Makes sure all phase durations sum up to final specified motion duration.
 */
class DurationConstraint : public Primitive {
public:
  using SchedulePtr = std::shared_ptr<ContactSchedule>;

  DurationConstraint(const OptVarsPtr& opt_vars, double T_total, int ee);
  ~DurationConstraint();

  VectorXd GetValues() const override;
  VecBound GetBounds() const override;
  void FillJacobianWithRespectTo (std::string var_set, Jacobian&) const override;

private:
  SchedulePtr schedule_;
  double T_total_;
};




} /* namespace opt */
} /* namespace xpp */

#endif /* XPP_OPT_INCLUDE_XPP_OPT_CONTACT_SCHEDULE_H_ */
