#include "baseclasses/object.h"
#include "baseclasses/room.h"

// DisplayableObject implementation
DisplayableObject::DisplayableObject() {
  invisible = false;
  depth = 0;
}

int DisplayableObject::getDepth() {
  return depth;
}

void DisplayableObject::setDepth(int _depth) {
  depth = _depth;
}

void DisplayableObject::makeInvisible() {
  invisible = true;
}

void DisplayableObject::makeVisible() {
  invisible = false;
}

// IdleObject implementation
IdleObject::IdleObject() {
  idleActive = true;
  idlePriority = 0;
}

int IdleObject::getIdlePriority() {
  return idlePriority;
}

void IdleObject::makeUpdateActive() {
  idleActive = true;
}

void IdleObject::makeUpdateInactive() {
  idleActive = false;
}

void IdleObject::setIdlePriority(int _prio) {
  idlePriority = _prio;
}

// MouseObject implementation
MouseObject::MouseObject() {
  mouseActive = true;
}

void MouseObject::makeMouseActive() {
  mouseActive = true;
}

void MouseObject::makeMouseInactive() {
  mouseActive = false;
}

// KeyboardObject implementation
KeyboardObject::KeyboardObject() {
  keyboardActive = true;
}

void KeyboardObject::makeKeyboardActive() {
  keyboardActive = true;
}

void KeyboardObject::makeKeyboardInactive() {
  keyboardActive = false;
}

// PhysicalObject implementation
PhysicalObject::PhysicalObject(Polygon* _shape, float _mass) {
  shape = _shape;
  mass = _mass;
}

PhysicalObject::~PhysicalObject() {
  delete shape;
}

void PhysicalObject::addForce(Vector2D addedForce) {
  force = force + addedForce;
}

void PhysicalObject::applyForce() {
  velocity = velocity + force * ((1.0f / mass) * MS_TICK_SIZE);
  force = Vector2D();
}

void PhysicalObject::applyVelocity() {
  shape->translate(velocity);
}

Polygon* PhysicalObject::getShape() {
  return shape;
}

void PhysicalObject::makePhysicsActive() {
  physicsActive = true;
}

void PhysicalObject::makePhysicsInactive() {
  physicsActive = false;
}
