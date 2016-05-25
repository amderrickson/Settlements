#ifndef BUTTON_H
#define BUTTON_H

class Button {
public:
    //Constructors & Deconstructors
    Button(){state=0;x=0;y=0;} //Default Constructor
    Button(SDL_Renderer* Renderer, std::string path, int x_, int y_,int angle_); //Initializes All Variables

    //Rendering & Events
    void handleEvent(SDL_Event* e);
    void render(SDL_Renderer* Renderer);

    //Accessors
    int getState();
    int getWidth();
    int getHeight();
    int getX();
    int getY();
    bool getActivate() {return activate;}
    void setActivate(bool act) {activate=act;}

    //Modifiers
    void setWidth(int w);
    void setHeight(int h);
    void setX(int x_);
    void setY(int y_);
    void setRenderRect(SDL_Rect Rect_) {renderrect=Rect_;}

private:
    Texture texture0;
    Texture texture1;
    Texture texture2;
    SDL_Rect renderrect={0,0,0,0};
    int state,x,y,angle;
    bool activate=false;
};

#endif // BUTTON_H
