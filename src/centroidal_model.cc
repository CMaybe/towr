/**
 @file    centroidal_model.cc
 @author  Alexander W. Winkler (winklera@ethz.ch)
 @date    May 19, 2017
 @brief   Brief description
 */

#include "xpp/opt/centroidal_model.h"

namespace xpp {
namespace opt {

CentroidalModel::CentroidalModel ()
{
  // rough values for hyq
  m_ = 80;
  I_ = buildInertiaTensor( 1.209488,
                           5.5837,
                           6.056973,
                           0.00571,
                           -0.190812,
                           -0.012668);

  I_inv_ = I_.inverse().sparseView();
}

CentroidalModel::~CentroidalModel ()
{
  // TODO Auto-generated destructor stub
}

CentroidalModel::BaseAcc
CentroidalModel::GetBaseAcceleration () const
{
  Vector3d f_lin, ang; f_lin.setZero(); ang.setZero();

  for (auto ee : ee_pos_.GetEEsOrdered()) {
    Vector3d f = ee_load_.At(ee);
    ang += f.cross(com_pos_-ee_pos_.At(ee));
    f_lin += f;
  }

  //zmp_ take gravity out, as this is constant and messes up SNOPT
  static const Vector3d fg_W(0.0, 0.0, -m_*kGravity);
  f_lin += fg_W;

  BaseAcc acc;
//  acc.segment(AX, 3) = I_inv_*ang;
  acc.segment(AX, 3).setZero(); // zmp_ for now just testing the linear part
  acc.segment(LX, 3) = 1./m_  *f_lin;

  return acc;
}

JacobianRow
CentroidalModel::GetJacobianOfAccWrtBase (const BaseMotion& base, double t,
                                          Coords6D dim) const
{
  Jacobian jac_6d = GetJacobianOfAccWrtBase1(base, t);

  // zmp_ remove these again
  if (dim==AX || dim==AY || dim==AZ) {
    jac_6d.setZero();
  }

  return jac_6d.row(dim);
}

JacobianRow
CentroidalModel::GetJacobianofAccWrtLoad (const EndeffectorsForce& ee_force,
                                          double t,
                                          EndeffectorID ee,
                                          Coords6D dim) const
{
  Jacobian jac_6d = GetJacobianofAccWrtLoad1(ee_force, t, ee);

  // zmp_ remove these again
  if (dim==AX || dim==AY || dim==AZ) {
    jac_6d.setZero();
  }

  return jac_6d.row(dim);
}

JacobianRow
CentroidalModel::GetJacobianofAccWrtEEPos (const EndeffectorsMotion& ee_motion,
                                           double t,
                                           EndeffectorID ee,
                                           Coords6D dim) const
{
  // zmp_ eliminate this function
  Jacobian jac_6d = GetJacobianofAccWrtEEPos1(ee_motion, t, ee);

  // zmp_ remove these again
  if (dim==AX || dim==AY || dim==AZ) {
    jac_6d.setZero();
  }

  return jac_6d.row(dim);
}

Jacobian
CentroidalModel::GetJacobianOfAccWrtBase1 (const BaseMotion& base,
                                          double t) const
{
  // build the com jacobian
  Jacobian com_jac(kDim3d, base.GetRows());
  for (auto dim : {X,Y,Z})
    com_jac.row(dim) = base.GetJacobian(t, kPos, To6D(dim));

  Jacobian jac_ang(kDim3d, base.GetRows());
  for (const Vector3d& f : ee_load_.ToImpl())
    jac_ang += BuildCrossProductMatrix(f)*com_jac;

  Jacobian jac(kDim6d, base.GetRows());
  jac.middleRows(AX, kDim3d) = I_inv_*jac_ang;
  // linear acceleration does not depend on com position

  return jac;
}

Jacobian
CentroidalModel::GetJacobianofAccWrtLoad1 (const EndeffectorsForce& ee_force,
                                          double t,
                                          EndeffectorID ee) const
{
  int n = ee_force.GetRows();
  Jacobian ee_force_jac(kDim3d, n);
  for (auto dim : {X,Y,Z})
    ee_force_jac.row(dim) = ee_force.GetJacobian(t, ee, dim);

  Jacobian jac_lin = ee_force_jac;

  Vector3d r = com_pos_-ee_pos_.At(ee);
  Jacobian jac_ang = -BuildCrossProductMatrix(r)*ee_force_jac;


  Jacobian jac(kDim6d, n);
  jac.middleRows(AX, kDim3d) = I_inv_*jac_ang;
  jac.middleRows(LX, kDim3d) = 1./m_*jac_lin;

  return jac;
}

Jacobian
CentroidalModel::GetJacobianofAccWrtEEPos1 (const EndeffectorsMotion& ee_motion,
                                           double t_global,
                                           EndeffectorID ee) const
{
  int n = ee_motion.GetRows();
  Jacobian jac_ang(kDim3d, n);

  Jacobian jac_ee(kDim3d, n);
  for (auto dim : {d2::X,d2::Y}) // zmp_ this is hacky, include z height
    jac_ee.row(dim) = ee_motion.GetJacobianPos(t_global, ee, dim);

  Vector3d f = ee_load_.At(ee);
  jac_ang = BuildCrossProductMatrix(f)*(-jac_ee);

  Jacobian jac(kDim6d, n);
  jac.middleRows(AX, kDim3d) = I_inv_*jac_ang;
  // linear acceleration does not depend on endeffector position.
  return jac;
}


} /* namespace opt */
} /* namespace xpp */
