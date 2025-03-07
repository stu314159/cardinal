/********************************************************************/
/*                  SOFTWARE COPYRIGHT NOTIFICATION                 */
/*                             Cardinal                             */
/*                                                                  */
/*                  (c) 2021 UChicago Argonne, LLC                  */
/*                        ALL RIGHTS RESERVED                       */
/*                                                                  */
/*                 Prepared by UChicago Argonne, LLC                */
/*               Under Contract No. DE-AC02-06CH11357               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*                 See LICENSE for full restrictions                */
/********************************************************************/

#include "GeometryUtility.h"
#include "MooseUtils.h"

namespace geom_utility {

bool isPointZero(const Point & pt)
{
  Point zero(0.0, 0.0, 0.0);
  return pt.absolute_fuzzy_equals(zero);
}

Point unitVector(const Point & pt, const std::string & name)
{
  if (isPointZero(pt))
    mooseError("'" + name + "' cannot have zero norm!");

  return pt.unit();
}

Real minDistanceToPoints(const Point & pt, const std::vector<Point> & candidates,
  const unsigned int & axis)
{
  const auto idx = projectedIndices(axis);

  Real distance = std::numeric_limits<Real>::max();
  for (const auto & c : candidates)
  {
    Real dx = c(idx.first) - pt(idx.first);
    Real dy = c(idx.second) - pt(idx.second);
    Real d = std::sqrt(dx * dx + dy * dy);
    distance = std::min(d, distance);
  }

  return distance;
}

Point projectPoint(const Real & x0, const Real & x1, const unsigned int & axis)
{
  auto i = projectedIndices(axis);
  Point point;
  point(i.first) = x0;
  point(i.second) = x1;
  point(axis) = 0.0;

  return point;
}

Real projectedLineHalfSpace(Point pt1, Point pt2, Point pt3, const unsigned int & axis)
{
  // project points onto plane perpendicular to axis
  pt1(axis) = 0.0;
  pt2(axis) = 0.0;
  pt3(axis) = 0.0;

  auto i = projectedIndices(axis);

  return (pt1(i.first) - pt3(i.first)) * (pt2(i.second) - pt3(i.second)) -
    (pt2(i.first) - pt3(i.first)) * (pt1(i.second) - pt3(i.second));
}

bool pointInPolygon(const Point & point, const std::vector<Point> & corners,
  const unsigned int & axis)
{
  auto n_pts = corners.size();

  std::vector<bool> negative_half_space;
  std::vector<bool> positive_half_space;
  for (unsigned int i = 0; i < corners.size(); ++i)
  {
    int next = (i == n_pts - 1) ? 0 : i + 1;
    auto half = projectedLineHalfSpace(point, corners[i], corners[next], axis);
    negative_half_space.push_back(half < 0);
    positive_half_space.push_back(half > 0);
  }

  bool negative = std::find(negative_half_space.begin(), negative_half_space.end(), true) !=
                  negative_half_space.end();
  bool positive = std::find(positive_half_space.begin(), positive_half_space.end(), true) !=
                  positive_half_space.end();

  bool in_polygon = !(negative && positive);
  if (in_polygon) return true;

  for (unsigned int i = 0; i < corners.size(); ++i)
    if (pointOnEdge(point, corners, axis))
      return true;

  return false;
}

bool pointOnEdge(const Point & point, const std::vector<Point> & corners,
  const unsigned int & axis)
{
  auto n_pts = corners.size();
  auto idx = projectedIndices(axis);

  Real tol = 1e-8;
  for (unsigned int i = 0; i < corners.size(); ++i)
  {
    int next = (i == n_pts - 1) ? 0 : i + 1;
    const auto & pt1 = corners[i];
    const auto & pt2 = corners[next];
    bool close_to_line = projectedDistanceFromLine(point, pt1, pt2, axis) < tol;

    // we can stop early if we know we're not close to the line
    if (!close_to_line)
      continue;

    // check that the point is "between" the two points; TODO: first pass
    // we can just compare x and y coordinates
    bool between_points = (point(idx.first) >= std::min(pt1(idx.first), pt2(idx.first))) &&
                          (point(idx.first) <= std::max(pt1(idx.first), pt2(idx.first))) &&
                          (point(idx.second) >= std::min(pt1(idx.second), pt2(idx.second))) &&
                          (point(idx.second) <= std::max(pt1(idx.second), pt2(idx.second)));

    // point needs to be close to the line AND "between" the two points
    if (close_to_line && between_points)
      return true;
  }

  return false;
}

std::pair<unsigned int, unsigned int> projectedIndices(const unsigned int & axis)
{
  std::pair<unsigned int, unsigned int> indices;

  if (axis == 0)
  {
    indices.first = 1;
    indices.second = 2;
  }
  else if (axis == 1)
  {
    indices.first = 0;
    indices.second = 2;
  }
  else
  {
    indices.first = 0;
    indices.second = 1;
  }

  return indices;
}

Point projectedUnitNormal(Point pt1, Point pt2, const unsigned int & axis)
{
  // project the points to the plane perpendicular to the axis
  pt1(axis) = 0.0;
  pt2(axis) = 0.0;

  auto i = projectedIndices(axis);

  Real dx = pt2(i.first) - pt1(i.first);
  Real dy = pt2(i.second) - pt1(i.second);

  Point normal = projectPoint(dy, -dx, axis);
  Point gap_line = pt2 - pt1;

  auto cross_product = gap_line.cross(normal);

  if (cross_product(axis) > 0)
    return normal.unit();
  else
  {
    Point corrected_normal = projectPoint(-dy, dx, axis);
    return corrected_normal.unit();
  }
}

Real distanceFromLine(const Point & pt, const Point & line0, const Point & line1)
{
  const Point a = pt - line0;
  const Point b = pt - line1;
  const Point c = line1 - line0;

  return (a.cross(b).norm()) / c.norm();
}

Real projectedDistanceFromLine(Point pt, Point line0, Point line1, const unsigned int & axis)
{
  // project all the points to the plane perpendicular to the axis
  pt(axis) = 0.0;
  line0(axis) = 0.0;
  line1(axis) = 0.0;

  return distanceFromLine(pt, line0, line1);
}

std::vector<Point> polygonCorners(const unsigned int & num_sides, const Real & radius,
  const unsigned int & axis)
{
  std::vector<Point> corners;
  Real theta = 2.0 * M_PI / num_sides;
  Real first_angle = M_PI / 2.0 - theta / 2.0;

  for (unsigned int i = 0; i < num_sides; ++i)
  {
    Real angle = first_angle + i * theta;
    Real x = radius * cos(angle);
    Real y = radius * sin(angle);

    corners.push_back(projectPoint(x, y, axis));
  }

  return corners;
}

Point rotatePointAboutAxis(const Point & p, const Real & angle, const Point & axis)
{
  Real cos_theta = cos(angle);
  Real sin_theta = sin(angle);

  Point pt;
  Real xy = axis(0) * axis(1);
  Real xz = axis(0) * axis(2);
  Real yz = axis(1) * axis(2);

  Point x_op(cos_theta + axis(0) * axis(0) * (1.0 - cos_theta),
             xy * (1.0 - cos_theta) - axis(2) * sin_theta,
             xz * (1.0 - cos_theta) + axis(1) * sin_theta);

  Point y_op(xy * (1.0 - cos_theta) + axis(2) * sin_theta,
             cos_theta + axis(1) * axis(1) * (1.0 - cos_theta),
             yz * (1.0 - cos_theta) - axis(0) * sin_theta);

  Point z_op(xz * (1.0 - cos_theta) - axis(1) * sin_theta,
             yz * (1.0 - cos_theta) + axis(0) * sin_theta,
             cos_theta + axis(2) * axis(2) * (1.0 - cos_theta));

  pt(0) = x_op * p;
  pt(1) = y_op * p;
  pt(2) = z_op * p;
  return pt;
}

std::vector<Point> boxCorners(const BoundingBox & box, const Real & factor)
{
  Point diff = (box.max() - box.min()) / 2.0;
  Point origin = box.min() + diff;

  // Rescale sidelength of box by specified factor
  diff *= factor;

  // Vectors for sides of box
  Point dx(2.0 * diff(0), 0.0, 0.0);
  Point dy(0.0, 2.0 * diff(1), 0.0);
  Point dz(0.0, 0.0, 2.0 * diff(2));

  std::vector<Point> verts(8, origin - diff);
  const unsigned int pts_per_dim = 2;
  for (unsigned int z = 0; z < pts_per_dim; z++)
    for (unsigned int y = 0; y < pts_per_dim; y++)
      for (unsigned int x = 0; x < pts_per_dim; x++)
        verts[pts_per_dim * pts_per_dim * z + pts_per_dim * y + x] += x * dx + y * dy + z * dz;

  return verts;
}

} // end namespace geom_utility
