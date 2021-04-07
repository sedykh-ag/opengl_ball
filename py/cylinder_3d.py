import gmsh
import sys
import math

gmsh.initialize()

gmsh.model.add("cylinder")

lc = 1e-1

n = 3

for i in range(1, n+1):
    gmsh.model.geo.addPoint(math.cos(2 * math.pi * i / n)*(.1), math.sin(2 * math.pi * i / n)*(.1), 0, lc, i)

gmsh.model.geo.addPoint(0, 0, 0, 4)

gmsh.model.geo.addCircleArc(1, 4, 2, 1)
gmsh.model.geo.addCircleArc(2, 4, 3, 2)
gmsh.model.geo.addCircleArc(3, 4, 1, 3)

gmsh.model.geo.addCurveLoop([1, 2, 3], 1)
gmsh.model.geo.addPlaneSurface([1], 1)

ov = gmsh.model.geo.extrude([[2, 1]], 0, 0, .3);

"""
int n = 3;
  for (int i=1; i<=n; i++) {
    gmsh::model::geo::addPoint(cos(2 * M_PI * i / n)*(.1), sin(2 * M_PI * i / n)*(.1), 0, lc, i);
  }
  gmsh::model::geo::addPoint(0, 0, 0, 4);

  gmsh::model::geo::addCircleArc(1, 4, 2, 1);
  gmsh::model::geo::addCircleArc(2, 4, 3, 2);
  gmsh::model::geo::addCircleArc(3, 4, 1, 3);

  gmsh::model::geo::addCurveLoop({1, 2, 3}, 1);
  gmsh::model::geo::addPlaneSurface({1}, 1);


  std::vector<std::pair<int, int>> ov;
  gmsh::model::geo::extrude({{2, 1}}, 0, 0, .3, ov);
"""
gmsh.model.geo.synchronize()
gmsh.model.mesh.generate(3)

if '-nopopup' not in sys.argv:
    gmsh.fltk.run()

gmsh.finalize()
