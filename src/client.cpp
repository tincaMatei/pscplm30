#include <SDL2/SDL.h>
#include "colorpicker.h"
#include <enet/enet.h>

void initENET() {
	if(enet_initialize() < 0) {
		fprintf(stderr, "Enet failed to initialize\n");
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "Enet initialized succesfully\n");
}

void deinitENET() {
	enet_deinitialize();
}

SDL_Window* window;
SDL_Renderer* renderer;

const int PIXEL_WIDTH = 24;
const int PIXEL_HEIGHT = 24;
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const int PICKER_WIDTH  = 256 + 10;
const int PICKER_HEIGHT = 200;

const float CAMERA_SPEED = 3.5f;

const int HUE_R1 =  90;
const int HUE_R2 = 100;

const int SAT_VAL_RAY = 80;
const int SAT_VAL_X = SCREEN_WIDTH  / 2 - SAT_VAL_RAY * sqrt(2.0f) / 2.0f + 1;
const int SAT_VAL_Y = SCREEN_HEIGHT / 2 - SAT_VAL_RAY * sqrt(2.0f) / 2.0f + 1;
const int SAT_VAL_W = sqrt(2.0f) * SAT_VAL_RAY - 2;
const int SAT_VAL_H = sqrt(2.0f) * SAT_VAL_RAY - 2;

const int HUE_PRECISION = 2000;

void initSDL() {
  
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
    SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
    exit(1);
  }
  
  window = SDL_CreateWindow("Deeznuts", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  
  if(window == NULL) {
    SDL_Log("Unable to create window: %s\n", SDL_GetError());
    exit(1);
  }
  
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if(renderer == NULL) {
    SDL_Log("Unable to create renderer: %s\n", SDL_GetError());
    exit(1);
  }
}

void deinitSDL() {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  
  renderer = NULL;
  window = NULL;
  
  SDL_Quit();
}

struct Pixel {
  unsigned char r, g, b;
};

template<typename T>
void readNumber(unsigned char* &data, T &x) {
  // bruh
  x = static_cast<T*>(static_cast<void*>(data))[0];
  data = data + sizeof(T);
}

template<typename T>
void writeNumber(unsigned char* &data, T &x) {
  // bruh
  static_cast<T*>(static_cast<void*>(data))[0] = x;
  data = data + sizeof(T);
}

class Canvas {
private:
  short width;
  short height;
  
  Pixel** data;
public:
  Canvas(unsigned char* dataInput) {
    if(dataInput != NULL) {    
      readNumber(dataInput, width);
      readNumber(dataInput, height);
      
      data = new Pixel*[height];
      for(int i = 0; i < height; ++i) {
        data[i] = new Pixel[width];
        for(int j = 0; j < width; ++j) {
          readNumber(dataInput, data[i][j].r);
          readNumber(dataInput, data[i][j].g);
          readNumber(dataInput, data[i][j].b);
        }
      }
    } else {
      width = height = 16;
      data = new Pixel*[height];
      for(int i = 0; i < height; ++i) {
        data[i] = new Pixel[width];
        for(int j = 0; j < width; ++j) {
          if(i == j)
            data[i][j] = {0xff, 0xff, 0xff};
          else
            data[i][j] = {0, 0, 0};
        }
      }
    }
  }
  
  void setPixel(int x, int y, Pixel p) {
    data[y][x] = p;
  }
  
  void display(SDL_Renderer* renderer, int xCamera, int yCamera) {
    for(int i = 0; i < height; ++i)
      for(int j = 0; j < width; ++j) {
        int realX = j * PIXEL_WIDTH - xCamera;
        int realY = i * PIXEL_HEIGHT - yCamera;
        SDL_Rect rect = {realX, realY, PIXEL_WIDTH, PIXEL_HEIGHT};
        SDL_SetRenderDrawColor(renderer, data[i][j].r, data[i][j].g, 
                                         data[i][j].b, 0xff);
        SDL_RenderFillRect(renderer, &rect);
      }
  }
  
  int getWidth() {
    return width;
  }
  
  int getHeight() {
    return height;
  }
  
  Pixel getPixel(int x, int y) {
    return data[y][x];
  }
};

ENetPeer* peer;
class Camera {
private:
  float x, y;
  Canvas* canvas;
  
  bool pressing, colorPicker, pipette;
  
  Pixel color;
  double globalHue, globalS, globalV;
  
public:
  Camera(Canvas* _canvas, float _x, float _y) {
    x = _x;
    y = _y;
    canvas = _canvas;
    pressing = false;
    colorPicker = false;
    pipette = false;
    
    color = {0x00, 0x00, 0x00};
    globalHue = globalS = globalV = 0.0f;
  }
  
  void mousePress(int key) {
    if(key == SDL_BUTTON_RIGHT)
      colorPicker ^= 1;
    if(key == SDL_BUTTON_LEFT)
      pressing = true;
  }
  
  void mouseRelease(int key) {
    if(key == SDL_BUTTON_LEFT)
      pressing = false;
  }
  
  void mouseMotion(int xMouse, int yMouse) {
    if(pressing && !colorPicker) {
      xMouse = (int)floor(x + xMouse); // Real x and y
      yMouse = (int)floor(y + yMouse);
      
      if(xMouse < 0)
        xMouse = xMouse - PIXEL_WIDTH + 1;
      if(yMouse < 0)
        yMouse = yMouse - PIXEL_HEIGHT + 1;
      xMouse = xMouse / PIXEL_WIDTH;
      yMouse = yMouse / PIXEL_HEIGHT;

      if(0 <= xMouse && xMouse < canvas->getWidth() &&
         0 <= yMouse && yMouse < canvas->getHeight()) {
        if(pipette)
          color = canvas->getPixel(xMouse, yMouse);
        else {
          short lPixel = yMouse, cPixel = xMouse;
          canvas->setPixel(xMouse, yMouse, color);
          unsigned char* packetsend = new unsigned char[sizeof(short) * 2 +
                                                        sizeof(unsigned char) * 3];
          unsigned char* pnt = packetsend;
          
          writeNumber(pnt, lPixel);
          writeNumber(pnt, cPixel);
          writeNumber(pnt, color.r);
          writeNumber(pnt, color.g);
          writeNumber(pnt, color.b);
          
          ENetPacket* packet = enet_packet_create(packetsend,
                                                  sizeof(short) * 2 +
                                                  sizeof(unsigned char) * 3,
                                                  ENET_PACKET_FLAG_RELIABLE);
          enet_peer_send(peer, 0, packet);
        }
      }
    } else if(pressing) {
      int xd = SCREEN_WIDTH / 2 - xMouse,
          yd = SCREEN_HEIGHT / 2 - yMouse;
      int dist = xd * xd + yd * yd;
      
      if(HUE_R1 * HUE_R1 <= dist && dist <= HUE_R2 * HUE_R2) {
                                
        globalHue = atan2(-yd, -xd) / (2.0f * M_PI) * 360.0f;
        if(globalHue < 0.0f)
          globalHue = globalHue + 360.0f;
      }
      
      if(SAT_VAL_X <= xMouse && xMouse <= SAT_VAL_X + SAT_VAL_W &&
         SAT_VAL_Y <= yMouse && yMouse <= SAT_VAL_Y + SAT_VAL_H) {
        globalS = (double)(xMouse - SAT_VAL_X) / SAT_VAL_W;
        globalV = (double)(yMouse - SAT_VAL_Y) / SAT_VAL_H;
      }
      rgb colorrgb = hsv2rgb({globalHue, globalS, globalV});
      color.r = (int)floor(colorrgb.r * 255.0f);
      color.g = (int)floor(colorrgb.g * 255.0f);
      color.b = (int)floor(colorrgb.b * 255.0f);
    }
  }
  
  void display(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
    SDL_RenderClear(renderer);
    if(colorPicker) {
      hsv colorhsv = rgb2hsv({color.r / 255.0f, color.g / 255.0f, color.b / 255.0f});
      
      for(int i = 0; i < HUE_PRECISION; ++i) {
        int x1, y1;
        int x2, y2;
        
        double angle = (double)i / HUE_PRECISION * 2 * M_PI;
        
        x1 = (int)floor(cos(angle) * HUE_R1) + SCREEN_WIDTH / 2;
        x2 = (int)floor(cos(angle) * HUE_R2) + SCREEN_WIDTH / 2;
        y1 = (int)floor(sin(angle) * HUE_R1) + SCREEN_HEIGHT / 2;
        y2 = (int)floor(sin(angle) * HUE_R2) + SCREEN_HEIGHT / 2;
        
        float h = (float)i / HUE_PRECISION * 360.0f, s = 1.0f, v = 1.0f;
        rgb newcol = hsv2rgb({h, s, v});
        
        SDL_SetRenderDrawColor(renderer, (int)floor(newcol.r * 255),
                                         (int)floor(newcol.g * 255),
                                         (int)floor(newcol.b * 255), 0xff);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        

      }
      for(int s = 0; s <= SAT_VAL_H; ++s)
        for(int v = 0; v <= SAT_VAL_W; ++v) {
          rgb newcol = hsv2rgb({globalHue, (double)s / SAT_VAL_H, 
                                            (double)v / SAT_VAL_W});
          SDL_SetRenderDrawColor(renderer, (int)floor(newcol.r * 255),
                                           (int)floor(newcol.g * 255),
                                           (int)floor(newcol.b * 255), 0xff);
          SDL_RenderDrawPoint(renderer, SAT_VAL_X + s, SAT_VAL_Y + v);
        }
      
      SDL_Rect rect = {0, 0, 40, 40};
      SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 0xff);
      SDL_RenderFillRect(renderer, &rect);
    } else {
      canvas->display(renderer, (int)floor(x), (int)floor(y));
    }
  }
  
  void keyHold(const Uint8* state) {
    if(state[SDL_SCANCODE_A])
      x -= CAMERA_SPEED;
    if(state[SDL_SCANCODE_D])
      x += CAMERA_SPEED;
    if(state[SDL_SCANCODE_S])
      y += CAMERA_SPEED;
    if(state[SDL_SCANCODE_W])
      y -= CAMERA_SPEED;
  }
  
  void keyPress(int key) {
    if(key == SDL_SCANCODE_E)
      pipette = true;
  }
  
  void keyRelease(int key) {
    if(key == SDL_SCANCODE_E)
      pipette = false;
  }
};

int main() {
  initSDL();
  initENET();
  
	ENetHost* client;
	
	client = enet_host_create(NULL, 1, 2, 0, 0);
  
  if(client == NULL) {
		fprintf(stderr, "Failed to create client\n");
		exit(EXIT_FAILURE);
	}
  
  ENetAddress address;
	ENetEvent enetevent;
	
	enet_address_set_host(&address, "localhost");
	address.port = 9999;
  
	peer = enet_host_connect(client, &address, 2, 0);

	if(peer == NULL) {
		fprintf(stderr, "No available peers for initiating an ENet connection.\n");
		exit(EXIT_FAILURE);
	}
  
	fprintf(stderr, "Created peer successfully.\n");
	enet_host_service(client, NULL, 0);
  
  Canvas* canvas = NULL;
  
  if(enet_host_service(client, &enetevent, 1000) && enetevent.type == ENET_EVENT_TYPE_CONNECT) {
		fprintf(stderr, "Connection to server succeeded.\n");

    fprintf(stderr, "Loading map:\n");
    
    if(enet_host_service(client, &enetevent, 10000) > 0) {
      canvas = new Canvas(enetevent.packet->data);
      fprintf(stderr, "Loaded map successfuly\n");
    }
	} else {
		enet_peer_reset(peer);
		fprintf(stderr, "Connection to server failed.\n");
	  enet_host_destroy(client);
    exit(EXIT_FAILURE);
	}
  
  Camera* camera = new Camera(canvas, 0, 0);
  
  SDL_Event event;
  bool quit = false;
  
  while(!quit) {
    while(!quit && enet_host_service(client, &enetevent, 0) > 0) {
      if(enetevent.type == ENET_EVENT_TYPE_RECEIVE) {
        unsigned char* packetdata = enetevent.packet->data;
        short lPixel, cPixel;
        Pixel newPixel;
        
        readNumber(packetdata, lPixel);
        readNumber(packetdata, cPixel);
        readNumber(packetdata, newPixel.r);
        readNumber(packetdata, newPixel.g);
        readNumber(packetdata, newPixel.b);
      
        canvas->setPixel(cPixel, lPixel, newPixel);
        
        enet_packet_destroy(enetevent.packet);
      } else if(enetevent.type == ENET_EVENT_TYPE_DISCONNECT) {
        fprintf(stderr, "Disconnected from server\n");
        quit = true;
      }
    }
    while(SDL_PollEvent(&event)) {
      if(event.type == SDL_QUIT) {
        quit = true;
		    enet_peer_disconnect(peer, 0);
        bool disconnect = false;
        while(!disconnect && enet_host_service(client, &enetevent, 2000) > 0) {
          if(enetevent.type == ENET_EVENT_TYPE_DISCONNECT) {
            fprintf(stderr, "Disconnected succesfully from server\n");
            disconnect = true;
          } else if(enetevent.type == ENET_EVENT_TYPE_RECEIVE)
            enet_packet_destroy(enetevent.packet);
        }
        
        if(!disconnect) {
          fprintf(stderr, "Disconnected forcefuly from server\n");
          enet_peer_reset(peer);
        }
      } else if(event.type == SDL_MOUSEBUTTONDOWN)
        camera->mousePress(event.button.button);
      else if(event.type == SDL_MOUSEBUTTONUP)
        camera->mouseRelease(event.button.button);
      else if(event.type == SDL_KEYDOWN)
        camera->keyPress(event.key.keysym.scancode);
      else if(event.type == SDL_KEYUP)
        camera->keyRelease(event.key.keysym.scancode);
    }
    
    int x, y;
    SDL_GetMouseState(&x, &y);
    camera->mouseMotion(x, y);
    
    const Uint8* state;
    state = SDL_GetKeyboardState(NULL);
    
    camera->keyHold(state);
    
    camera->display(renderer);
    SDL_RenderPresent(renderer);
    SDL_Delay(10);
  }
  
	enet_host_destroy(client);
  
  deinitENET();
  deinitSDL();
  return 0;
}
