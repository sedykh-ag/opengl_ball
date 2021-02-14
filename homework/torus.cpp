#include <set>
#include <cmath>
#include <gmsh.h>
#include <iostream>

int main(int argc, char **argv)
{
  gmsh::initialize(argc, argv);

  gmsh::model::add("torus");

  double lc = 1e-1;

  int n = 100; // loops quantity
  double R = 1;
  double r = .5;

  //int cn = 1;
  int arcs = 0;
  int loops = 0;
  for (int i=1, k=1; i<=n; i++) {

    double x = R + r * cos(2 * M_PI * i / n);
    double y = x;
    double z = r * sin(2 * M_PI * i / n);

    int a=k;
    gmsh::model::geo::addPoint(0, 0, z, lc, k++);

    int b=k;
    gmsh::model::geo::addPoint(x*cos(-2 * M_PI / 3),
                               y*sin(-2 * M_PI / 3),
                               z,
                               lc, k++);
    int c=k;
    gmsh::model::geo::addPoint(x*cos(0),
                               y*sin(0),
                               z,
                               lc, k++);
    int d=k;
    gmsh::model::geo::addPoint(x*cos(2 * M_PI / 3),
                               y*sin(2 * M_PI / 3),
                               z,
                               lc, k++);

    gmsh::model::geo::addCircleArc(b, a, c, ++arcs);
    gmsh::model::geo::addCircleArc(c, a, d, ++arcs);;
    gmsh::model::geo::addCircleArc(d, a, b, ++arcs);
    gmsh::model::geo::addCurveLoop({arcs-2, arcs-1, arcs}, ++loops);

  }

  int surfaces = 0;

  for (int i=1; i < loops; i++) {
    gmsh::model::geo::addPlaneSurface({i, (i+1)}, ++surfaces);
  }

  gmsh::model::geo::addPlaneSurface({loops, 1}, ++surfaces);

  /*
    Uncomment for volumed version
  */
  /*
  std::vector<int> surf_vec(surfaces);
  for (int i=0; i<surfaces; i++)
    surf_vec[i] = i+1;

  gmsh::model::geo::addSurfaceLoop(surf_vec, 1);

  gmsh::model::geo::addVolume({1}, 1);
  */

  gmsh::model::geo::synchronize();
  gmsh::model::mesh::generate(3);

  std::set<std::string> args(argv, argv + argc);
  if(!args.count("-nopopup")) gmsh::fltk::run();

  gmsh::finalize();

  return 0;
}

