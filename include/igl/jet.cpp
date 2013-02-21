#include "jet.h"
#include "colon.h"

#ifndef IGL_NO_EIGEN
//void igl::jet(const int m, Eigen::MatrixXd & J)
//{
//  using namespace Eigen;
//  using namespace igl;
//  // Applications/MATLAB_R2012b.app/toolbox/matlab/graph3d/jet.m
//  const int n = ceil(m/4);
//  // resize output
//  J.resize(m,3);
//  // u = [(1:1:n)/n ones(1,n-1) (n:-1:1)/n]';
//  VectorXd u(n*3-1);
//  u.block(0,0,n-1,1) = colon(1,n)/double(n);
//  VectorXd g;
//  colon(0,n*3-1-1,g);
//  g.array() = g.array() + ceil(n/2) - int((m%4)==1);
//  VectorXd r = (g.array() + n).eval().matrix();
//  VectorXd b = (g.array() - n).eval().matrix();
//  int ri = 0;
//  int gi = 0;
//  int sb = 0;
//  // count number of indices in b
//  for(int j = 0;j<g.rows();j++)
//  {
//    sb += b(j)<m;
//  }
//
//  for(int j = 0;j<g.rows();j++)
//  {
//    if(r(j)<m)
//    {
//      J(r(j),0) = u(ri++);
//    }
//    if(g(j)<m)
//    {
//      J(g(j),1) = u(gi++);
//    }
//    if(b(j)<m)
//    {
//      //J(b(j),2) = u(m- --sb);
//    }
//  }
//}
#endif


template <typename T>
void igl::jet(const T x, T * rgb)
{
  igl::jet(x,rgb[0],rgb[1],rgb[2]);
}

template <typename T>
void igl::jet(const T x, T & r, T & g, T & b)
{
  // Only important if the number of colors is small. In which case the rest is
  // still wrong anyway
  // x = linspace(0,1,jj)' * (1-1/jj) + 1/jj;
  //
  const double rone = 0.8;
  const double gone = 1.0;
  const double bone = 1.0;

  if(x<1/8.)
  {
    r = 0;
    g = 0;
    b = bone*(0.5+(x)/(1/8.)*0.5);
  }else if(x<3/8.)
  {
    r = 0;
    g = gone*(x-1/8.)/(3/8.-1/8.);
    b = bone; 
  }else if(x<5/8.)
  {
    r = rone*(x-3/8.)/(5/8.-3/8.);
    g = gone;
    b = (bone-(x-3/8.)/(5/8.-3/8.));
  }else if(x<7/8.)
  {
    r = rone;
    g = (gone-(x-5/8.)/(7/8.-5/8.));
    b = 0;
  }else
  {
    r = (bone-(x-7/8.)/(1.-7/8.)*0.5);
    g = 0;
    b = 0;
  }
}

#ifndef IGL_NO_HEADER
template void igl::jet<double>(double, double*);
#endif