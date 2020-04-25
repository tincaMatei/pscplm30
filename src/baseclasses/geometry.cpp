#include "baseclasses/geometry.h"
#include <algorithm>

// Geometry utility functions
const float EPS = 1.0f;

// Returns the value of the area determinant of point abc
// More precisely, it returns the area of the triangle a, b, c
// multiplied by 2. The sign of the result can be positive or negative
// depending on the order of the points (if a, b, c are in trigonometric
// order or not)
static inline float ccwDeterminant(Vector2D a, Vector2D b, Vector2D c) {
  return a.x * b.y +
         b.x * c.y +
         c.x * a.y -
         a.x * c.y -
         b.x * a.y -
         c.x * b.y;
}

// Checks if two segments intersect
bool segmentToSegmentCollision(Vector2D a1, Vector2D a2, Vector2D b1, Vector2D b2) {
  return ccwDeterminant(a1, a2, b1) * ccwDeterminant(a1, a2, b2) < 0.0f &&
         ccwDeterminant(b1, b2, a1) * ccwDeterminant(b1, b2, a2) < 0.0f;
}

// Return the distance between two points
float pointDistance(Vector2D a, Vector2D b) {
  return (a - b).length();
}

// Checks if two floats are approximately equal
bool approxEqual(float a, float b) {
  return fabs(a - b) <= EPS;
}

// Area of triangle abc
float triangleArea(Vector2D a, Vector2D b, Vector2D c) {
  return fabs(ccwDeterminant(a, b, c)) / 2.0f;
}

// Gets the distance between point c to segment [AB]
float pointToSegmentDistance(Vector2D c, Vector2D a, Vector2D b) {
  float ab = pointDistance(a, b);
  float h = triangleArea(c, a, b) * 2.0f / ab;
  float da = pointDistance(c, a),
        db = pointDistance(c, b);
  
  float foot1 = sqrt(da * da - h * h),
        foot2 = sqrt(db * db - h * h);
  
  if(approxEqual(foot1 + foot2, ab))
    return h;
  return std::min(da, db);
}

// Check if the given point is inside a triangle
bool insideTriangle(Vector2D a, Vector2D b, Vector2D c, Vector2D point) {
  return approxEqual(triangleArea(a, b, point) + triangleArea(b, c, point) +
                     triangleArea(c, a, point),  triangleArea(a, b, c));
}

// Vector2D implementation
Vector2D::Vector2D() {
  x = y = 0.0f;
}

Vector2D::Vector2D(float _x, float _y) {
  x = _x;
  y = _y;
}

float Vector2D::length() {
  return sqrt(x * x + y * y);
}

Vector2D Vector2D::operator+ (const Vector2D &t) const {
  return Vector2D(x + t.x, y + t.y);
}

Vector2D Vector2D::operator- (const Vector2D &t) const {
  return Vector2D(x - t.x, y - t.y);
}

Vector2D Vector2D::operator* (const float &t) const {
  return Vector2D(x * t, y * t);
}

// Polygon implementation
Polygon::Polygon() {
  centroidPos = Vector2D();
  circleRadius = -1.0f;
}

Vector2D Polygon::getCenterOfMass() {  
  Vector2D com = Vector2D();
  for(int i = 0; i < polygon.size(); ++i)
    com = com + polygon[i];
  com = com * (1.0f / polygon.size());
  return com;
}

void Polygon::centerOfMassCentroid() {
  centroidPos = getCenterOfMass();
}

void Polygon::setCentroidPosition(Vector2D pos) {
  centroidPos = pos;
}

void Polygon::translate(Vector2D translation) {
  centroidPos = Vector2D();
  for(unsigned int i = 0; i < polygon.size(); ++i) {
    polygon[i] = polygon[i] + translation;
    centroidPos = centroidPos + polygon[i] * (1.0f / polygon.size());
  }
}

void Polygon::rotate(float angle, float x, float y) {
  float dx, dy;
  for(unsigned int i = 0; i < polygon.size(); ++i) {
    dx = polygon[i].x - x;
    dy = polygon[i].y - y;
    
    polygon[i].x = x + (dx * cos(angle) - dy * sin(angle));
    polygon[i].y = y + (dx * sin(angle) + dy * cos(angle));
  }
  
  dx = centroidPos.x - x;
  dy = centroidPos.y - y;
  centroidPos = Vector2D({x + (dx * cos(angle) - dy * sin(angle)),
                          y + (dx * sin(angle) + dy * cos(angle))});
}

void Polygon::rotate(float angle) {
  rotate(angle, centroidPos.x, centroidPos.y);
}

bool Polygon::inside(Vector2D point) {
  for(int i = 1; i + 1 < polygon.size(); ++i) {
    if(insideTriangle(polygon[0], polygon[i], polygon[i + 1], point)) {
      return true;
    }
  }
  return false;
}

bool Polygon::collides(Polygon* otherPoly) {
  // Circle to circle intersection
  /*if(circleRadius >= 0.0f && otherPoly->circleRadius >= 0.0f) {
    return pointDistance(polygon[0], otherPoly->polygon[0]) 
        <= circleRadius + otherPoly->circleRadius;
  } else if(circleRadius >= 0.0f) { // Circle to polygon intersection
    for(int i = 0; i < otherPoly->polygon.size(); ++i) {
      int ii = (i + 1) % otherPoly->polygon.size();
      
      if(pointToSegmentDistance(polygon[0], otherPoly->polygon[i],
         otherPoly->polygon[ii]) <= circleRadius)
        return true;
    }
    
    return otherPoly->inside(polygon[0]);
  } else if(otherPoly->circleRadius >= 0.0f) { // Polygon to circle intersection
    return otherPoly->collides(this);
  } else { // Polygon to polygon intersection
    for(int i = 0; i < polygon.size(); ++i)
      for(int j = 0; j < otherPoly->polygon.size(); ++j) {
        int ii = (i + 1) % polygon.size();
        int jj = (j + 1) % otherPoly->polygon.size();
        if(segmentToSegmentCollision(polygon[i], polygon[ii],
                          otherPoly->polygon[j], otherPoly->polygon[jj]))
          return true;
      }
    return otherPoly->inside(polygon[0]) ||
           inside(otherPoly->polygon[0]);
  }*/
  
  //return false;
  
  float x1, y1, x2, y2;
  float ox1, oy1, ox2, oy2;
  
  x1 = y1 = ox1 = oy1 =  1e9;
  x2 = y2 = ox2 = oy2 = -1e9;
  
  for(unsigned int i = 0; i < polygon.size(); ++i) {
    x1 = std::min(x1, polygon[i].x);
    x2 = std::max(x2, polygon[i].x);
    y1 = std::min(y1, polygon[i].y);
    y2 = std::max(y2, polygon[i].y);
  }
  
  for(unsigned int i = 0; i < otherPoly->polygon.size(); ++i) {
    ox1 = std::min(ox1, otherPoly->polygon[i].x);
    ox2 = std::max(ox2, otherPoly->polygon[i].x);
    oy1 = std::min(oy1, otherPoly->polygon[i].y);
    oy2 = std::max(oy2, otherPoly->polygon[i].y);
  }
  
  return std::max(x1, ox1) <= std::min(x2, ox2) &&
         std::max(y1, oy1) <= std::min(y2, oy2);
}

Polygon* getRectangle(float x, float y, float w, float h) {
  Polygon* polygon = new Polygon();
  polygon->polygon.push_back(Vector2D(x - w / 2.0f, y - h / 2.0f));
  polygon->polygon.push_back(Vector2D(x + w / 2.0f, y - h / 2.0f));
  polygon->polygon.push_back(Vector2D(x + w / 2.0f, y + h / 2.0f));
  polygon->polygon.push_back(Vector2D(x - w / 2.0f, y + h / 2.0f));
  polygon->centerOfMassCentroid();
  return polygon;
}

Polygon* getPoint(float x, float y) {
  Polygon* polygon = new Polygon();
  polygon->polygon.push_back(Vector2D(x, y));
  polygon->centerOfMassCentroid();
  return polygon;
}

Polygon* getRegularPolygon(float x, float y, float radius, int n) {
  Polygon* polygon = new Polygon();
  float angleGrowth = PI * 2 / n, angle = 0.0f;
  
  for(int i = 0; i < n; ++i) {
    polygon->polygon.push_back(Vector2D(x, y) + 
                               Vector2D(cos(angle), sin(angle)) * radius);
    angle += angleGrowth;
  }
  polygon->centerOfMassCentroid();
  return polygon;
}

Polygon* getSegment(float x1, float y1, float x2, float y2) {
  Polygon* polygon = new Polygon();
  polygon->polygon.push_back(Vector2D(x1, y1));
  polygon->polygon.push_back(Vector2D(x2, y2));
  polygon->centerOfMassCentroid();
  return polygon;
}

Polygon* getCircle(float x, float y, float radius) {
  Polygon* polygon = getPoint(x, y);
  polygon->circleRadius = radius;
  polygon->centerOfMassCentroid();
  return polygon;
}

