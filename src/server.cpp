#include <cstdio>
#include <vector>
#include <enet/enet.h>
#include "baseclasses/graphicshandler.h"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

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

short width, height;
Pixel** data;

const int DEFAULT_WIDTH  = 100;
const int DEFAULT_HEIGHT = 100;

void saveData() {
  FILE *fout = fopen("savedcanvas.dat", "wb");
  fwrite(&width, sizeof(short), 1, fout);
  fwrite(&height, sizeof(short), 1, fout);
  for(int i = 0; i < width; ++i)
    for(int j = 0; j < height; ++j) {
      fwrite(&data[i][j].r, sizeof(unsigned char), 1, fout);
      fwrite(&data[i][j].g, sizeof(unsigned char), 1, fout);
      fwrite(&data[i][j].b, sizeof(unsigned char), 1, fout);
    }
  fclose(fout);
}

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

const int MAX_PEERS = 8;
ENetPeer* peers[MAX_PEERS];

int main() {
  for(int i = 0; i < MAX_PEERS; ++i)
    peers[i] = NULL;
  
  FILE *fin = fopen("savedcanvas.dat", "rb");
  if(fin == NULL) {
    width = DEFAULT_WIDTH;
    height = DEFAULT_HEIGHT;
    
    data = new Pixel*[height];
    for(int i = 0; i < height; ++i) {
      data[i] = new Pixel[width];
      for(int j = 0; j < width; ++j)
        if(i == j)
          data[i][j] = {0xff, 0xff, 0xff};
        else
          data[i][j] = {0, 0, 0};
    }
  } else {
    fread(&width, sizeof(short), 1, fin);
    fread(&height, sizeof(short), 1, fin);
    
    data = new Pixel*[height];
    for(int i = 0; i < height; ++i) {
      data[i] = new Pixel[width];
      for(int j = 0; j < width; ++j) {
        fread(&data[i][j].r, sizeof(unsigned char), 1, fin);
        fread(&data[i][j].g, sizeof(unsigned char), 1, fin);
        fread(&data[i][j].b, sizeof(unsigned char), 1, fin);
      }
    }
    fclose(fin);
  }
  
  initSDL();
  
  if(enet_initialize() < 0) {
		fprintf(stderr, "Enet failed to initialize\n");
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "Enet initialized succesfully\n");
  
  ENetAddress address;
	ENetHost* server;

	address.host = ENET_HOST_ANY;
	address.port = 9999;

	server = enet_host_create(&address, MAX_PEERS, 2, 0, 0);

	if(server == NULL) {
		fprintf(stderr, "Failed to create server\n");
		exit(EXIT_FAILURE);
	}
  
  ENetEvent event;
  SDL_Event sdlevent;
  
  bool quit = false;
  while(!quit) {
    while(enet_host_service(server, &event, 10) > 0) {
      if(event.type == ENET_EVENT_TYPE_CONNECT) {
        fprintf(stderr, "A new client connected from %x:%u.\n", event.peer->address.host,
                                                                event.peer->address.port);
        int i = 0;
        while(i < MAX_PEERS && peers[i] != NULL)
          ++i;
        
        if(peers[i] == NULL)
          peers[i] = event.peer;
        // We must send the entire picture to the newly connected peer
        
        unsigned char* packetData = new unsigned char[sizeof(short) * 2 +
                                                      3 * width * height * 
                                                      sizeof(unsigned char)];
        
        unsigned char* pnt = packetData;
        
        writeNumber(pnt, width);
        writeNumber(pnt, height);
        for(int i = 0; i < width; ++i)
          for(int j = 0; j < height; ++j) {
            writeNumber(pnt, data[i][j].r);
            writeNumber(pnt, data[i][j].g);
            writeNumber(pnt, data[i][j].b);
          }
        
        ENetPacket* packet = enet_packet_create(packetData, 
                             sizeof(short) * 2 + 3 * width * height * 
                                                 sizeof(unsigned char),
                                                 ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(event.peer, 0, packet);
      } else if(event.type == ENET_EVENT_TYPE_RECEIVE) {
        unsigned char* packetData = static_cast<unsigned char*>(event.packet->data);
        unsigned char* pnt = packetData;
        short lPixel, cPixel;
        Pixel newPixel;
        readNumber(pnt, lPixel);
        readNumber(pnt, cPixel);
        readNumber(pnt, newPixel.r);
        readNumber(pnt, newPixel.g);
        readNumber(pnt, newPixel.b);
        
        if(0 <= lPixel && lPixel < width && 0 <= cPixel && cPixel < height) {
          data[lPixel][cPixel] = newPixel;
          // We must broadcast the change to everyone
          
          for(int i = 0; i < MAX_PEERS; ++i)
            if(peers[i] != NULL) {
              unsigned char* sentpacketdata 
                           = new unsigned char[sizeof(short) * 2
                                             + sizeof(unsigned char) * 3];
              pnt = sentpacketdata;
              writeNumber(pnt, lPixel);
              writeNumber(pnt, cPixel);
              writeNumber(pnt, newPixel.r);
              writeNumber(pnt, newPixel.g);
              writeNumber(pnt, newPixel.b);
              
              ENetPacket* packet = enet_packet_create(sentpacketdata, 
                                                      sizeof(short) * 2 +
                                                      sizeof(unsigned char) * 3, 
                                                      ENET_PACKET_FLAG_RELIABLE);
              enet_peer_send(peers[i], 0, packet);
            }
        }
        
        //fprintf(stderr, "Received packet(%u): %s | %u :%s\n", event.packet->dataLength,
        //                                                      event.peer->data,
        //                                                      event.channelID,
        //                                                      event.packet->data);
        enet_packet_destroy(event.packet);
      } else if(event.type == ENET_EVENT_TYPE_DISCONNECT) {
        for(int i = 0; i < MAX_PEERS; ++i)
          if(peers[i] == event.peer)
            peers[i] = NULL;
        fprintf(stderr, "%x:%u disconnected.\n", event.peer->address.host,
                                                 event.peer->address.port);
      } else {
        fprintf(stderr, "No event\n");
      }
    }
    
    while(SDL_PollEvent(&sdlevent)) {
      if(sdlevent.type == SDL_QUIT)
        quit = true;
    }
    SDL_Delay(10);
  }

  saveData();

	enet_host_destroy(server);
  enet_deinitialize();
  
  deinitSDL();
  return 0;
}
