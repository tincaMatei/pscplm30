#ifndef __OBJECT_H
#define __OBJECT_H

#include <SDL2/SDL.h>
#include "geometry.h"

// The object will be rendered on the screen
class DisplayableObject {
protected:
  // True if the object is invisible
  bool invisible;
  
  // The depth at which the object will be rendered
  // The bigger the depth, the earlier the object will be rendered
  int depth;
  
  // Set the depth of an object
  void setDepth(int _depth);
public:
  DisplayableObject();

  // Display the object
  virtual void display(SDL_Renderer* renderer) = 0;
  
  // Make an object visible or invisible
  virtual void makeInvisible();
  virtual void makeVisible();
  
  // Returns the depth of an object
  int getDepth();
};

// The object will do something every tick
class IdleObject {
protected:
  // Priority of the object
  // The bigger the priority, the earlier it will be updated
  int idlePriority;
  
  // True if the object is active
  bool idleActive;
  
  // Sets the priority
  void setIdlePriority(int _prio);
public:
  IdleObject();

  // Actions to do every tick
  virtual void update() = 0;
  
  // Make the object active or inactive
  virtual void makeUpdateActive();
  virtual void makeUpdateInactive();

  // Returns the priority
  int getIdlePriority();
};

// The class will do something with every action of the mouse
class MouseObject {
protected:
  // True if the mouse is active
  bool mouseActive;
public:
  MouseObject();
  
  // Mouse events
  virtual void mouseMotion(float _x, float _y) = 0;
  virtual void mouseButtonPress(int button) = 0;
  virtual void mouseButtonRelease(int button) = 0;

  // Make the mouse active or inactive
  virtual void makeMouseActive();
  virtual void makeMouseInactive();
};

// The object will do something with every action of the keyboard
class KeyboardObject {
protected:
  // True if the keyboard actions are active
  bool keyboardActive;
public:
  KeyboardObject();
  
  // Keyboard events
  virtual void keyboardPress(int key, bool repeat = false) = 0;
  virtual void keyboardRelease(int key) = 0;
  virtual void keyboardHold(const Uint8* state) = 0;
  
  // Make keyboard actions active or inactive
  virtual void makeKeyboardActive();
  virtual void makeKeyboardInactive();
};


class PhysicalObject {
protected:
  // True if the object is active
  bool physicsActive;

  // Velocity vector
  Vector2D velocity;
  
  // Force vector applied on the object
  Vector2D force;
  
  // Mass of the object
  float mass;
  
  // Shape of the object. It's centroid is it's position
  Polygon* shape;
public:
  PhysicalObject(Polygon* _shape, float _mass = 1.0f);
  ~PhysicalObject();
  
  // Adds force to the object
  virtual void addForce(Vector2D addedForce);
  
  // Turns the force into velocity
  virtual void applyForce();
  
  // Moves the object according to its velocity
  virtual void applyVelocity();
  
  // Gets the shape of the polygon
  Polygon* getShape();
  
  // Make the object active or inactive
  void makePhysicsActive();
  void makePhysicsInactive();
};

class Collider {
public:
  virtual void checkCollision() = 0;
};

#endif
