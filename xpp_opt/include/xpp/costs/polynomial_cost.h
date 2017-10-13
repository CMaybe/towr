/**
 @file    polynomial_cost.h
 @author  Alexander W. Winkler (winklera@ethz.ch)
 @date    May 30, 2016
 @brief   Declaration of ASplineCost, QuadraticSplineCost, SquaredSplineCost
 */

#ifndef USER_TASK_DEPENDS_XPP_OPT_INCLUDE_XPP_OPT_POLYNOMIAL_COST_H_
#define USER_TASK_DEPENDS_XPP_OPT_INCLUDE_XPP_OPT_POLYNOMIAL_COST_H_

#include <string>

#include <xpp/composite.h>
#include <xpp/matrix_vector.h>

namespace xpp {
namespace opt {


/** @brief Calculates the scalar cost associated with a vector of variables.
  *
  * This class is responsible for getting the current value of the optimization
  * variables from the subject and calculating the scalar cost from these.
  */
class QuadraticPolynomialCost : public Cost {
public:

  QuadraticPolynomialCost(const OptVarsPtr&, const MatVec&,
                          const std::string& variables, double weight);
  virtual ~QuadraticPolynomialCost();

  /**  The cost is calculated as
    *  cost = x^T * M * x   +   v^T * x
    */
  VectorXd GetValues () const override;

private:
  void FillJacobianWithRespectTo(std::string var_set, Jacobian&) const;

  double weight_ = 1.0;
  MatVec matrix_vector_;  ///< a matrix and a vector used to calculate a scalar cost
  std::string variables_;
};

//class SquaredSplineCost : public ASplineCost {
//
//  /**  The cost is calculated as:
//    *  g = M*x + v
//    *  cost = sqrt(g^T*g)
//    */
//  double EvaluateCost () const override;
//  virtual VectorXd EvaluateGradientWrt(std::string var_set) final { assert(false); };
//
//};

} /* namespace opt */
} /* namespace xpp */

#endif /* USER_TASK_DEPENDS_XPP_OPT_INCLUDE_XPP_OPT_POLYNOMIAL_COST_H_ */