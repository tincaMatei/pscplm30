#ifndef __GEOMETRY_H
#define __GEOMETRY_H

/* TODO:
 * implement rotation
 * implement a bad collision function for the moment
 * implement a good collision function
 */
#include <cmath>
#include <vector>

const float PI = acos(-1);

// 2d vectors
struct Vector2D {
  float x, y;
  Vector2D();
  Vector2D(float _x, float _y);

  float length();

  Vector2D operator+ (const Vector2D &x) const;
  Vector2D operator- (const Vector2D &x) const;
  Vector2D operator* (const float &x) const;
};

// Struct that holds a polygon
struct Polygon {
  Polygon();
  
  // Centroid of polygon
  Vector2D centroidPos;
  
  // Sets the center of the polygon at the given position
  void setCentroidPosition(Vector2D pos);
  
  // Sets the center of the polygon at the center of mass of the polygon
  void centerOfMassCentroid();
  Vector2D getCenterOfMass();
  
  // Radius of a circle
  // Should be negative if the shape is not a circle
  float circleRadius;
  
  
  // This holds the vertices of the polygon in trigonometric order
  // (counter clockwise)
  std::vector<Vector2D> polygon;
  
  // Translates the polygon with the vector (x, y)
  void translate(Vector2D translation);

  // Rotates the polygon with the given angle
  // The center off mass will be point (x, y)
  // The rotation will be in counter-clockwise direction
  void rotate(float angle, float x, float y);
    
  // Rotates the polygon with the given angle
  // The center of rotation will be the center of mass
  // The rotation will be in counter-clockwise direction
  void rotate(float angle);

  // Checks collision between two polygons
  // Note: at the moment, this just returns true if the
  //       bounding boxes of the two polygons intersect
  bool collides(Polygon* otherPolygon);
  
  // Checks if the given point is inside the polygon
  bool inside(Vector2D point);
};

float pointDistance(Vector2D a, Vector2D b);

// Creates various shapes
Polygon* getRectangle(float x, float y, float w, float h);
Polygon* getPoint(float x, float y);
Polygon* getRegularPolygon(float x, float y, float radius, int n);
Polygon* getSegment(float x1, float y1, float x2, float y2);
Polygon* getCircle(float x, float y, float radius);

#endif

