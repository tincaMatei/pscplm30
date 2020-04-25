#ifndef __ROOM_H
#define __ROOM_H

#include <vector> // std::vector
#include <SDL2/SDL.h>

// TODO: decide MS_TICK_SIZE and check it's data type
// TODO: destructors

const int MS_TICK_SIZE = 10;

// If the transition is -1, that means that we exit the game
const int EXIT_ROOM = -1;

// Instance that holds informations about other objects
// And runs the game loop
class Room {
protected:
  // This should contain the room we need to change to after
  // runGameLoop() returns true
  int transition;
public:
  ~Room();

  // Runs one game loop
  // Should return false if we should change to another room
  virtual bool runGameLoop() = 0;
  
  // Render all objects inside the room
  virtual void render(SDL_Renderer* renderer) = 0;
  
  virtual void initialize() = 0;
  
  // Returns which room we should change
  int getTransition();
};

// Holds every room of the game
class RoomContext {
private:
  // Vector that holds the rooms
  std::vector<Room*> rooms;
public:
  // Insert a new room in the vector
  void pushRoom(Room*);
  
  // Get the room with the given ID
  Room* getRoom(int roomId);
};

#endif

