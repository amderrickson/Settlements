#ifndef TEXTURE
#define TEXTURE

class Texture {
    public:
        //Constructors & Deconstructors
        Texture(); //Default Constructor
        Texture(SDL_Renderer* Renderer, std::string path); //Create Texture from image
        Texture(SDL_Texture* t);//Create texture from existing texture
        ~Texture(); //Deallocates Memory

        //Rendering & Events
        bool createBlank( SDL_Renderer* Renderer, int width, int height, SDL_TextureAccess access);
        void setAsRenderTarget(SDL_Renderer* Renderer);
        bool loadFromFile(SDL_Renderer* Renderer, std::string path); //Load texture from image file
        void render(SDL_Renderer* Renderer, int x, int y, int w=0, int h=0); //Renders the texture
        void renderRect(SDL_Renderer* Renderer, SDL_Rect* dstrect, SDL_Rect* srcrect); //Renders to rect

        //Accessors
        SDL_Texture* getTexture() {return texture;} //return the texture
        int getWidth() {return width;} //Get width of the texture
        int getHeight() {return height;} //Get height of the texture

        //Modifiers
        void setAngle(double angle_) {angle=angle_;}
        void setWidth(int width_) {width=width_;} //Set the width of the texture
        void setHeight(int height_) {height=height_;} //Set the height of the texture
        bool setTexture(SDL_Texture* t); //Sets texture

        //Miscellaneous
        void free();//Used by deconstructor to deallocate memory

    private:
        //The texture
        SDL_Texture* texture;

        //Parameters
        int width;
        int height;

        //Customization
        double angle=0.0;
        SDL_Rect* srcrect=NULL;
        SDL_Rect* dstrect=NULL;
};

#endif // TEXTURE

