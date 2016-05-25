//Standard C++ Libraries
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <stdlib.h>
//SDL2 C++ Libraries

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

//boost C++ Libraries

#include <boost/filesystem.hpp>

//Custom Interface Classes
#include "framework/interface/texture.h"
#include "framework/interface/button.h"
#include "framework/interface/window.h"
#include "framework/interface/tile.h"
#include "framework/interface/checkbox.h"

//Screen dimension constants (will default to 640x480 if none are defined in config.ini
int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 480;

//Global Variables
SDL_Renderer* Renderer = NULL;
SDL_Window* window = NULL;
SDL_Surface* ScreenSurface = NULL;

struct Mouse_Resources {
    int x,y; //current x and y coordinates
    int tile_location_x, tile_location_y; //current tile coordinates
    int x_modifier=0; //x scroll modifier
    int y_modifier=0; //y scroll modifier
};

struct Terrain_Resources {
    int size=100;
    int width=50;
    int height=40;
    std::vector<Tile> terrain_individual_information;
    std::vector<std::pair<std::string,std::vector<std::string> > > terrain_type_information;
};

static const char *arrow[] = {
  /* width height num_colors chars_per_pixel */
  "    32    32        3            1",
  /* colors */
  "X c #000000",
  ". c #ffffff",
  "  c None",
  /* pixels */
  "XXXX                            ",
  "X...X                           ",
  "X.X..X                          ",
  "X..X..X                         ",
  " X..X..X                        ",
  "  X..X..X    X                  ",
  "   X..X..X XXX                  ",
  "    X..X..XX                    ",
  "     X..X.X                     ",
  "      X..XX                     ",
  "       XXXXX                    ",
  "      XX  X.X                   ",
  "      X    X.X                  ",
  "     XX     X.X                 ",
  "             XX                 ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "0,0"
};

static SDL_Cursor *init_system_cursor(const char *image[])
{
  int i, row, col;
  Uint8 data[4*32];
  Uint8 mask[4*32];
  int hot_x, hot_y;

  i = -1;
  for (row=0; row<32; ++row) {
    for (col=0; col<32; ++col) {
      if (col % 8) {
        data[i] <<= 1;
        mask[i] <<= 1;
      } else {
        ++i;
        data[i] = mask[i] = 0;
      }
      switch (image[4+row][col]) {
        case 'X':
          data[i] |= 0x01;
          mask[i] |= 0x01;
          break;
        case '.':
          mask[i] |= 0x01;
          break;
        case ' ':
          break;
      }
    }
  }
  sscanf(image[4+row], "%d,%d", &hot_x, &hot_y);
  return SDL_CreateCursor(data, mask, 32, 32, hot_x, hot_y);
}
//---------Text_Functions------------------------

int loadFromRenderedText(SDL_Renderer* Renderer,  std::string textureText, TTF_Font* Font, SDL_Color textColor, int x, int y) {
    SDL_Texture* texture;
    int Width;
    int Height;
    //Get rid of preexisting texture

    //Render text surface
    SDL_Surface* textSurface = TTF_RenderText_Solid( Font, textureText.c_str(), textColor );
    if( textSurface == NULL ) {
        printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
    }
    else {
        //Create texture from surface pixels
        texture = SDL_CreateTextureFromSurface(Renderer, textSurface );
        if( texture == NULL ) {
            printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
        }
        else {
            //Get image dimensions
            Width = textSurface->w;
            Height = textSurface->h;
        }

        //Get rid of old surface
        SDL_FreeSurface( textSurface );
    }
    SDL_Rect renderQuad = { x, y, Width, Height };
    SDL_RenderCopy(Renderer,texture, NULL, &renderQuad );
    return Height;
}

//---------Initializations------------------------


bool initConfig() {
    std::ifstream cfg("..//Settlements//config.ini");
    if (!cfg.good()) {
        printf("Can't open config.ini.\n");
        return false;
    }
    std::map<std::string,std::string> config; //Stores the config options in a map
    std::string parameter, value;
    while(!cfg.eof()) {
        cfg >> parameter;
        if(parameter[0]!='[') {
            cfg >> value;
            config.insert(std::pair<std::string,std::string>(parameter,value));
        }
    }
    if(config.find("SCREEN_WIDTH")!=config.end()) { //Checks for SCREEN_WIDTH
        SCREEN_WIDTH=std::atoi(config.find("SCREEN_WIDTH")->second.c_str());
    }
    if(config.find("SCREEN_HEIGHT")!=config.end()) { //Checks for SCREEN_HEIGHT
        SCREEN_HEIGHT=std::atoi(config.find("SCREEN_HEIGHT")->second.c_str());
    }
    return true;
}

bool initSDL() {
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
        return false;
    }
    int imgFlags = IMG_INIT_PNG;
    if(!(IMG_Init(imgFlags)&imgFlags)) {
        printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return false;
    }
    else {
        //Get window surface
        ScreenSurface = SDL_GetWindowSurface(window);
    }
    return true;
}

bool initWindow() {
    window = SDL_CreateWindow("Settlements",0,0,SCREEN_WIDTH,SCREEN_HEIGHT,SDL_WINDOW_SHOWN);
    if( window == NULL ) {
        printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
        return false;
    }
    else {
        //Create renderer for window
        Renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED);
        if(Renderer == NULL){
            printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
            return false;
        }

        else {
            //Initialize renderer color
            SDL_SetRenderDrawColor( Renderer, 0xFF, 0xFF, 0xFF, 0xFF );

            //Initialize PNG loading
            int imgFlags = IMG_INIT_PNG;
            if( !( IMG_Init( imgFlags ) & imgFlags ) ) {
                printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
                return false;
            }

            if( TTF_Init() == -1 ){
                printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
                return false;
            }
        }
    }
    return true;
}

bool initTextures(std::map<std::string,std::vector<Texture> > &alltextures) {
    boost::filesystem::path path("../Settlements/assets/textures");
    boost::filesystem::directory_iterator b(path), e;
    std::vector<boost::filesystem::path> directory(b, e);
    std::vector<Texture> tmp;
    for(int i=0; i<directory.size();i++) {
        b=boost::filesystem::directory_iterator(directory[i]), e;
        std::vector<boost::filesystem::path> subtexture(b, e);
        std::string s=directory[i].string();
        s.replace(s.begin(),s.begin()+path.string().size()+1,"");
        std::sort(subtexture.begin(),subtexture.end());
        for(int j=0;j<subtexture.size();j++) {
            if(subtexture[j].extension().string()==".png") {
                tmp.push_back(Texture(Renderer,subtexture[j].string()));
            }
        }
        alltextures.insert(std::pair<std::string,std::vector<Texture> >(s,tmp));
        tmp.clear();
    }
    return true;
}

bool initTiles(std::map<std::string,Tile> &alltiles) {
    std::ifstream f_tiles("../Settlements/assets/tilesnew.txt");
    if (!f_tiles.good()) {
        printf("Can't open tiles.txt.\n");
        return false;
    }
    std::string buffer, name;
    while(!f_tiles.eof()) {
        f_tiles>>buffer;
        if(buffer[0]=='<') {
            name=buffer;
            name.replace(name.begin(),name.begin()+1,"");
            alltiles.insert(std::pair<std::string,Tile>(name,Tile(name)));
        }
        else if(buffer[0]=='>') {
            buffer.replace(buffer.begin(),buffer.begin()+1,"");
            if(buffer=="capacity") {
                f_tiles>>buffer;
                alltiles.find(name)->second.setCapacity(std::atoi(buffer.c_str()));
            }
            else if(buffer=="mobility") {
                f_tiles>>buffer;
                alltiles.find(name)->second.setMobility(std::atoi(buffer.c_str()));
            }
            else if(buffer=="level") {
                f_tiles>>buffer;
                alltiles.find(name)->second.setLevel(std::atoi(buffer.c_str()));
            }
            else if(buffer=="below") {
                f_tiles>>buffer;
                alltiles.find(name)->second.setBelow(buffer);
            }
        }
    }
    return true;
}

//---------Map_Functions------------------------

bool map_parse(std::map<std::string,Tile> alltiles, std::vector<Tile> &map_info, std::string location, std::map<std::string,std::vector<Texture> > &textures1) {
    int w, h;
    std::ifstream map(location.c_str());
    if (!map.good()) {
        printf("Can't open map.txt.\n");
        return false;
    }
    int x=0;int y=0;
    std::map<int,std::string> maps;
    std::string value,name;
    map >> w;
    map >> h;
    while(!map.eof()) {
        map >> value;
        if(value[0]=='>') {
            value.replace(value.begin(),value.begin()+1,"");
            name=value;
            map>>value;
            maps.insert(std::pair<int,std::string>(std::atoi(name.c_str()),value));
        }
        else if(value!="/") {
            int value_=std::atoi(value.c_str());
            std::string value1= maps.find(value_)->second;
            int random=rand()%(textures1.find(value1)->second.size());
            Tile t(alltiles.find(value1)->second,random,x,y);
            map_info.push_back(t);
            x+=50;
        }
        else {
            if(x%100==0) {
                x=25;
            }
            else {
                x=0;
            }
            y+=28;
        }
    }
    return true;
}


//---------Camera_Functions------------------------

//This function returns the tile location that the mouse is currently hovering over. Each hexagon is split into rectangles
//to find the general mouse position. After that, each triangle of the hexagon is checked and the general mouse position is
//fixed.

void GetMouseLocation(Mouse_Resources &Mouse_Resource, int width, int height, std::vector<Tile> tiles, int &left, int &right) {
    left=0;right=0;
    right=0;left=0;
    width=width;
    height=height;
    int x_half=width*.5; //half the tile width
    int y_rect=28; //70% of the tile height
    int y_tri=height-y_rect; //top 30% of the tile
    int x=Mouse_Resource.x-(Mouse_Resource.x_modifier); int y=Mouse_Resource.y-30-(Mouse_Resource.y_modifier); //get mouse x,y
    int tmp_y=y%y_rect; //count how much remain after 70%s go into the y coordinate of mouse
    Mouse_Resource.tile_location_y=y/(y_rect); //set y tile location to remainder of 70%s
    //these if statements deal with the triangle portion of the hexagons
    if(Mouse_Resource.tile_location_y%2==0) {  //even row =(when row is further left)
        if(x%width<=x_half) { //left half
            float line=(x%width)*.48;//slope of hex triangle is .48, used to calculate line
            if(line<(y_tri-tmp_y)) {//if mouse position is above the line, the tile position is corrected
                Mouse_Resource.tile_location_y-=1;
            }
        }
        else { //right half
            float slope=((width-x%width)*.48); //slope of hex triangle is .48, used to calculate line
            if(slope<(y_tri-tmp_y)) {//if mouse position is above the line, the tile position is corrected
                Mouse_Resource.tile_location_y-=1;
            }

        }
    }
    else if(Mouse_Resource.tile_location_y%2==1) { //odd row =(when row is further right)
        if((x-x_half)%width<=x_half) { //left half
            float slope=((x-x_half)%width)*.48; //slope of hex triangle is .48, used to calculate line
            if(slope<(y_tri-tmp_y)) {//if mouse position is above the line, the tile position is corrected
                Mouse_Resource.tile_location_y-=1;
            }
        }
        else { //right half
            float slope=((width-(x-x_half)%width)*.48); //slope of hex triangle is .48, used to calculate line
            if(slope<(y_tri-tmp_y)) {//if mouse position is above the line, the tile position is corrected
                Mouse_Resource.tile_location_y-=1;
            }
        }
    }
    int num=Mouse_Resource.tile_location_x/(50);
    int level=tiles[num + Mouse_Resource.tile_location_y*50].returnLevel()*(10);
    int level1=tiles[num + Mouse_Resource.tile_location_y*50].returnLevel();
    if (Mouse_Resource.tile_location_y%2==0) {
        right=(tiles[num + (Mouse_Resource.tile_location_y+1)*50].returnLevel()-level1);
        if(num>0) {
            left=(tiles[num + (Mouse_Resource.tile_location_y+1)*50-1].returnLevel()-level1);
        }

    }
    else {
        if(num<49) {
            right=(tiles[num + (Mouse_Resource.tile_location_y+1)*50+1].returnLevel()-level1);
        }
        left=(tiles[num + (Mouse_Resource.tile_location_y+1)*50].returnLevel()-level1);
    }
    if(Mouse_Resource.tile_location_y%2==0) { //set width for even rows
        Mouse_Resource.tile_location_x=(x/width)*width;
    }
    else { //set width for odd rows
        Mouse_Resource.tile_location_x=((x-x_half)/width*width)+x_half;
    }
    Mouse_Resource.tile_location_y=Mouse_Resource.tile_location_y*y_rect;//set height
    Mouse_Resource.tile_location_y-=level;
    if(right<0) right=0;
    if(left<0) left=0;
    if(Mouse_Resource.tile_location_y/y_rect>49 || Mouse_Resource.tile_location_x/width>49 || Mouse_Resource.tile_location_y<0) {
        left=-1;
        right=-1;
    }
}

//closes and frees sdl assets

void close() {
    SDL_DestroyRenderer(Renderer);
    SDL_DestroyWindow(window);
    window = NULL;
    Renderer=NULL;
    IMG_Quit();
    SDL_Quit();
    TTF_Quit();
}

//this function moves the camera when the user hovers over the edge of the map

bool UpdateCamera(Mouse_Resources &Mouse_Resource) {
    bool moved=0;
    if(Mouse_Resource.x<10){//Moves Left based on proximity to edge
        Mouse_Resource.x_modifier+=2;
        moved=1;
    }
    else if(Mouse_Resource.x<20){//Moves Left based on proximity to edge
        Mouse_Resource.x_modifier+=1;
        moved=1;
    }
    if(Mouse_Resource.x>SCREEN_WIDTH-10) {//Moves Right based on proximity to edge
        Mouse_Resource.x_modifier-=2;
        moved=1;
    }
    else if(Mouse_Resource.x>SCREEN_WIDTH-20) {//Moves Right based on proximity to edge
        Mouse_Resource.x_modifier-=1;
        moved=1;
    }
    if(Mouse_Resource.y<40 && Mouse_Resource.y>30) {//Moves Up based on proximity to edge
        Mouse_Resource.y_modifier+=2;
        moved=1;
    }
    else if(Mouse_Resource.y<50 && Mouse_Resource.y>30) {//Moves Up based on proximity to edge
        Mouse_Resource.y_modifier+=1;
        moved=1;
    }
    if(Mouse_Resource.y>SCREEN_HEIGHT-10) {//Moves Down based on proximity to edge
        Mouse_Resource.y_modifier-=2;
        moved=1;
    }
    else if(Mouse_Resource.y>SCREEN_HEIGHT-20) {//Moves Down based on proximity to edge
        Mouse_Resource.y_modifier-=1;
        moved=1;
    }
    if(Mouse_Resource.y_modifier>0) { //Prevents the user from leaving the map
        Mouse_Resource.y_modifier=0;
        moved=1;
    }
    if(Mouse_Resource.x_modifier>0) { //Prevents the user from leaving the map
        Mouse_Resource.x_modifier=0;
        moved=1;
    }
    if(Mouse_Resource.x_modifier<(-50*50-25)+SCREEN_WIDTH) {
        Mouse_Resource.x_modifier=(-50*50-25)+SCREEN_WIDTH;
    }
    if(Mouse_Resource.y_modifier<-50*28-42+(SCREEN_HEIGHT)) {
        Mouse_Resource.y_modifier=-50*28-42+(SCREEN_HEIGHT);
    }
}

std::string getLower(std::map<std::string,Tile> tiles, Tile tile, int j) {
    while(tiles.find(tile.returnName())->second.returnLevel()>j) {
        tile=tiles.find(tile.returnBelow())->second;
    }
    return tile.returnName();
}

void create_map_layers(Texture &layers, Texture &minimap, Terrain_Resources &Terrain_Resource, Mouse_Resources &Mouse_Resource,std::map<std::string,std::vector<Texture> > textures,std::map<std::string,Tile> tiles) {
    SDL_SetRenderDrawColor(Renderer,0,0,0,255);
    minimap.createBlank(Renderer,(50*50+25)/5,(50*28+12)/5,SDL_TEXTUREACCESS_TARGET);
    layers.createBlank(Renderer,(50*50+25),(50*28+12),SDL_TEXTUREACCESS_TARGET);
    std::string name; Tile tile; int placex, placey, level;
    SDL_Rect l = {0,0,50*50+25,50*28+12};
    layers.setAsRenderTarget(Renderer);
    SDL_RenderFillRect(Renderer,&l);
    l.w=l.w/5;
    l.h=l.h/5;
    for(int i=0;i<Terrain_Resource.terrain_individual_information.size();i++) {
        name=Terrain_Resource.terrain_individual_information[i].returnName();
        tile=Terrain_Resource.terrain_individual_information[i];
        level=tile.returnLevel()*10;
        placex=tile.returnX()+Mouse_Resource.x_modifier;
        placey=tile.returnY()+Mouse_Resource.y_modifier+Terrain_Resource.height-textures.find(name)->second[tile.returnIndex()].getHeight();
        textures.find(name)->second[tile.returnIndex()].render(Renderer,placex,placey);
    }
    minimap.setAsRenderTarget(Renderer);
    SDL_RenderFillRect(Renderer,&l);
    SDL_Rect rect = {0,0,(50*50+25)/5,(50*28+12)/5};
    layers.renderRect(Renderer,&rect,NULL);
    SDL_SetRenderTarget(Renderer,NULL);
    SDL_SetRenderDrawColor(Renderer,255,255,255,255);
}

int main(int argc, char* args[]) {
    Mouse_Resources Mouse_Resource;
    Terrain_Resources Terrain_Resource;
    std::map<std::string,std::vector<Texture> > textures;
    std::map<std::string,Tile> tiles;
    Texture minimap;
    Texture layers;
    srand(time(NULL));
    if(!initConfig() && !initSDL() && !initWindow() && !initTextures(textures) && !initTiles(tiles)) {//Loads basic settings
        std::cerr<<"Failed to initialize config!\n";
    }
    else {
        if(!initSDL() && !initWindow()) {//Initializes SDL and related libraries
            std::cerr<<"Failed to initialize SDL!\n";
        }
        else {
            if(!initWindow()) { //Creates a window
                std::cerr<<"Failed to initialize window!\n";
            }
            else {
                if(!initTextures(textures)) { //Loads all textures
                    std::cerr<<"Failed to load textures!\n";
                }
                else {
                    if(!initTiles(tiles)) {
                        std::cerr<<"Failed to load tiles!\n";
                    }
                    else {
                        //Cursor Initialization
                        SDL_Cursor* cursor;
                        cursor=init_system_cursor(arrow);
                        SDL_SetCursor(cursor);

                        //Map Initialization
                        map_parse(tiles, Terrain_Resource.terrain_individual_information,"..//Settlements//map.map",textures);
                        create_map_layers(layers, minimap,Terrain_Resource,Mouse_Resource,textures,tiles);

                        //Event Initialization
                        const Uint8* currentKeyStates;
                        //initiates end of event loop
                        SDL_Event e; //event value

                        //keyboard logic
                        bool QUIT = false;

                        //Viewport Initialization
                        SDL_Rect screen={0,0,SCREEN_WIDTH,SCREEN_HEIGHT}; //the main screen viewport
                        SDL_Rect header={0,0,SCREEN_WIDTH,30}; //the header menu viewport
                        SDL_Rect header_highlight={0,0,SCREEN_WIDTH,29}; //the header highlights viewport
                        SDL_Rect map={0,30,SCREEN_WIDTH,SCREEN_HEIGHT-30}; //viewport for map area

                        SDL_Rect srcrect;
                        SDL_Rect dsrect;

                        //Window Initializations
                        Window Map(Renderer,0,screen.h-250, 380, 250);
                        SDL_Rect window0_rect;
                        SDL_Rect minimap_selector;


                        std::vector<Checkbox> window01;
                        window01.push_back(Checkbox(Renderer,"hex",22,21));
                        window01.push_back(Checkbox(Renderer,"layer",22,63));

                        std::vector<Button> window0;
                        window0.push_back(Button(Renderer,"cardinalarrow",22,0,0));
                        window0.push_back(Button(Renderer,"cardinalarrow",43,21,90));
                        window0.push_back(Button(Renderer,"cardinalarrow",22,42,180));
                        window0.push_back(Button(Renderer,"cardinalarrow",1,21,270));
                        window0.push_back(Button(Renderer,"diagonalarrow",1,0,270));
                        window0.push_back(Button(Renderer,"diagonalarrow",43,0,0));
                        window0.push_back(Button(Renderer,"diagonalarrow",43,42,90));
                        window0.push_back(Button(Renderer,"diagonalarrow",1,42,180));

                        int placex=0;int placey=0;
                        Uint32 startTime;
                        Uint32 endTime;
                        int left=0, right=0;

                        while(!QUIT) {
                            startTime = SDL_GetTicks();

                            SDL_GetMouseState(&Mouse_Resource.x,&Mouse_Resource.y);
                            GetMouseLocation(Mouse_Resource,Terrain_Resource.width, Terrain_Resource.height, Terrain_Resource.terrain_individual_information,left,right);
                            UpdateCamera(Mouse_Resource);

                            while(SDL_PollEvent(&e)!=0) {
                                currentKeyStates=SDL_GetKeyboardState( NULL );
                                if(e.type==SDL_QUIT) {
                                    QUIT = true;
                                }
                                Map.handleEvent(&e);
                                for(int i=0; i<window01.size();i++) {
                                    window01[i].handleEvent(&e);
                                }
                                for(int i=0; i<window0.size();i++) {
                                    window0[i].handleEvent(&e);
                                }
                            }
                            //Clear screen
                            SDL_RenderClear(Renderer);

                            //Map Viewport
                            SDL_RenderSetViewport(Renderer,&map); {
                                srcrect={srcrect.x-=Mouse_Resource.x_modifier, srcrect.y-=Mouse_Resource.y_modifier,map.w,map.h};
                                dsrect={map.x,0,map.w,map.h};
                                layers.renderRect(Renderer,&dsrect,&srcrect);
                                if(left!=-1 && right!=-1) {
                                    placex=Mouse_Resource.tile_location_x+(Mouse_Resource.x_modifier);
                                    placey=Mouse_Resource.tile_location_y+(Mouse_Resource.y_modifier);
                                    //textures.find("lcursor")->second[left].render(Renderer, placex, placey);
                                    //textures.find("rcursor")->second[right].render(Renderer, placex, placey);
                                }
                            }

                            //Screen Viewport
                            SDL_RenderSetViewport(Renderer, &screen); {
                                Map.render(Renderer);
                                window0_rect=Map.returnUsuableViewport();
                            }
                            SDL_RenderSetViewport(Renderer,&window0_rect); {
                                minimap_selector={-Mouse_Resource.x_modifier/5,-Mouse_Resource.y_modifier/5,map.w/5,map.h/5};

                                srcrect={0,0,window0_rect.w,window0_rect.h};
                                dsrect={0,0,srcrect.w,srcrect.h};

                                if(minimap_selector.x+minimap_selector.w>window0_rect.w-64) {
                                    srcrect.x=minimap_selector.x+minimap_selector.w-window0_rect.w+64;
                                    minimap_selector.x-=srcrect.x;
                                }



                                minimap.renderRect(Renderer,&dsrect,&srcrect);

                                SDL_RenderDrawRect(Renderer,&minimap_selector);

                                window0_rect.x-=64;
                                window0_rect.x+=window0_rect.w;
                                window0_rect.w=64;
                            }

                            SDL_RenderSetViewport(Renderer,&window0_rect); {
                                SDL_SetRenderDrawColor(Renderer, 0,0,0,255);
                                SDL_RenderFillRect(Renderer,NULL);
                                SDL_SetRenderDrawColor(Renderer, 255,255, 255, 255);
                                for(int i=0; i<window01.size();i++) {
                                    window01[i].setRenderRect(window0_rect);
                                }
                                for(int i=0; i<window0.size();i++) {
                                    window0[i].setRenderRect(window0_rect);
                                }
                                for(int i=0; i<window01.size();i++) {
                                    window01[i].render(Renderer);
                                }
                                for(int i=0; i<window0.size();i++) {
                                    window0[i].render(Renderer);
                                }
                            }

                            SDL_RenderPresent(Renderer);
                            endTime=SDL_GetTicks();
                            if(endTime-startTime>0) {
                                std::cout << 1000/(endTime-startTime) << " ";
                            }
                        }
                    }
                }
            }
        }
    //Free resources and close SDL2
    close();
    return 0;
    }
}
