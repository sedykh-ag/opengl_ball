#include <set>
#include <cmath>
#include <gmsh.h>

int main(int argc, char **argv)
{
  gmsh::initialize(argc, argv);

  gmsh::model::add("t3");

  double lc = 1e-1;

  int n = 3;
  for (int i=1; i<=n; i++) {
    gmsh::model::geo::addPoint(cos(2 * M_PI * i / n)*(.1), sin(2 * M_PI * i / n)*(.1), 0, lc, i);
  }
  gmsh::model::geo::addPoint(0, 0, 0, lc, 4);

  gmsh::model::geo::addCircleArc(1, 4, 2, 1);
  gmsh::model::geo::addCircleArc(2, 4, 3, 2);
  gmsh::model::geo::addCircleArc(3, 4, 1, 3);

  gmsh::model::geo::addCurveLoop({1, 2, 3}, 1);
  gmsh::model::geo::addPlaneSurface({1}, 1);

  std::vector<std::pair<int, int>> ov;
  std::vector<std::pair<int, int>> ov2;
  std::vector<std::pair<int, int>> ov3;
  std::vector<std::pair<int, int>> ov4;
  gmsh::model::geo::addPoint(0, 0.2, 0, lc, 5);
  gmsh::model::geo::revolve({{2, 1}}, 0, 0.2, 0, .1, 0, 0, 2*M_PI / 3 , ov);
  gmsh::model::geo::revolve({{2, 1}}, 0, 0.2, 0, .1, 0, 0, -2*M_PI / 3 , ov2);
  gmsh::model::geo::copy({{2,1}}, ov3);
  gmsh::model::geo::rotate({ov3}, 0, 0.2, 0, .1, 0, 0, 8.00001*M_PI / 12);
  gmsh::model::geo::revolve({ov3}, 0, 0.2, 0, .1, 0, 0, 2*M_PI / 3 , ov4);



  //gmsh::model::geo::copy({ov2}, ov3);
  //gmsh::model::geo::rotate({ov3}, 0, 0.2, 0, .1, 0, 0, -2*M_PI / 3);
  //gmsh::model::geo::extrude({{2, 1}}, 0, 0, .3, ov);


  gmsh::model::geo::synchronize();
  gmsh::model::mesh::generate(3);

  std::set<std::string> args(argv, argv + argc);
  if(!args.count("-nopopup")) gmsh::fltk::run();

  gmsh::finalize();

  return 0;
}

