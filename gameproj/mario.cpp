/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, standard IO, and strings
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <stdio.h>
#include <string>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
int frame;
int frame2;
SDL_Rect MarioCollisionBox;
SDL_Rect tempbox;
int i;
SDL_Rect one = { 322,318,26,36 };
SDL_Rect platforms[10];
const Uint8* currentKeyStates;

SDL_Rect temporaryBox;


//Texture wrapper class
class LTexture
{
public:
	//Initializes variables
	LTexture();

	//Deallocates memory
	~LTexture();

	//Loads image at specified path
	bool loadFromFile(std::string path);

	//Deallocates texture
	void free();

	//Set color modulation
	void setColor(Uint8 red, Uint8 green, Uint8 blue);

	//Set blending
	void setBlendMode(SDL_BlendMode blending);



	//Set alpha modulation
	void setAlpha(Uint8 alpha);

	//Renders texture at given point
	void render(int x, int y, SDL_Rect* clip = NULL);

	//Gets image dimensions
	int getWidth();
	int getHeight();

private:
	//The actual hardware texture
	SDL_Texture* mTexture;

	//Image dimensions
	int mWidth;
	int mHeight;
};




//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();
bool checkCollision(SDL_Rect a, SDL_Rect b);

//Frees media and shuts down SDL
void close();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Scene textures
const int WALKING_ANIMATION_FRAMES = 8;
SDL_Rect gSpriteClips[WALKING_ANIMATION_FRAMES];
SDL_Rect gSpriteRight[WALKING_ANIMATION_FRAMES];
SDL_Rect gSpriteDown[2];
LTexture gMarioAnimationPage;
LTexture gBackgroundTexture;
LTexture gHammerBroAnimationPage;
SDL_Rect hammerbros[8];


SDL_Rect* currentClip;
SDL_Rect* HB;
int a;

int direction;

LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile(std::string path)
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{



			SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0XFF, 0xFF, 0xFF));
			newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);

		//Create texture from surface pixels

		if (newTexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	//Return success
	mTexture = newTexture;

	return mTexture != NULL;
}

void LTexture::free()
{
	//Free texture if it exists
	if (mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::setColor(Uint8 red, Uint8 green, Uint8 blue)
{
	//Modulate texture rgb
	SDL_SetTextureColorMod(mTexture, red, green, blue);
}

void LTexture::setBlendMode(SDL_BlendMode blending)
{
	//Set blending function
	SDL_SetTextureBlendMode(mTexture, blending);
}

void LTexture::setAlpha(Uint8 alpha)
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod(mTexture, alpha);
}

void LTexture::render(int x, int y, SDL_Rect* clip)
{

	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if (clip != NULL)
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopy(gRenderer, mTexture, clip, &renderQuad);
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		//Create window
		gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					success = false;
				}
			}
		}
	}

	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//Load front alpha texture
	if (!gMarioAnimationPage.loadFromFile("marioanimation.png"))
	{
		printf("Failed to load front texture!\n");
		success = false;
	}
	else
	{
		gSpriteClips[0].x = 0;
		gSpriteClips[0].y = 85;
		gSpriteClips[0].w = 55;
		gSpriteClips[0].h = 80;

		gSpriteClips[1].x = 55;
		gSpriteClips[1].y = 85;
		gSpriteClips[1].w = 55;
		gSpriteClips[1].h = 80;

		gSpriteClips[2].x = 110;
		gSpriteClips[2].y = 85;
		gSpriteClips[2].w = 55;
		gSpriteClips[2].h = 80;

		gSpriteClips[3].x = 165;
		gSpriteClips[3].y = 85;
		gSpriteClips[3].w = 55;
		gSpriteClips[3].h = 80;

		gSpriteClips[4].x = 215;
		gSpriteClips[4].y = 85;
		gSpriteClips[4].w = 55;
		gSpriteClips[4].h = 80;

		gSpriteClips[5].x = 270;
		gSpriteClips[5].y = 85;
		gSpriteClips[5].w = 55;
		gSpriteClips[5].h = 80;

		gSpriteClips[6].x = 325;
		gSpriteClips[6].y = 85;
		gSpriteClips[6].w = 55;
		gSpriteClips[6].h = 80;

		gSpriteClips[7].x = 380;
		gSpriteClips[7].y = 85;
		gSpriteClips[7].w = 55;
		gSpriteClips[7].h = 80;





		gSpriteRight[0].x = 0;
		gSpriteRight[0].y = 159;
		gSpriteRight[0].w = 55;
		gSpriteRight[0].h = 80;

		gSpriteRight[1].x = 55;
		gSpriteRight[1].y = 159;
		gSpriteRight[1].w = 55;
		gSpriteRight[1].h = 80;

		gSpriteRight[2].x = 110;
		gSpriteRight[2].y = 159;
		gSpriteRight[2].w = 55;
		gSpriteRight[2].h = 80;

		gSpriteRight[3].x = 165;
		gSpriteRight[3].y = 159;
		gSpriteRight[3].w = 55;
		gSpriteRight[3].h = 80;

		gSpriteRight[4].x = 215;
		gSpriteRight[4].y = 159;
		gSpriteRight[4].w = 55;
		gSpriteRight[4].h = 80;

		gSpriteRight[5].x = 270;
		gSpriteRight[5].y = 159;
		gSpriteRight[5].w = 55;
		gSpriteRight[5].h = 80;

		gSpriteRight[6].x = 325;
		gSpriteRight[6].y = 159;
		gSpriteRight[6].w = 55;
		gSpriteRight[6].h = 80;

		gSpriteRight[7].x = 380;
		gSpriteRight[7].y = 159;
		gSpriteRight[7].w = 55;
		gSpriteRight[7].h = 80;




		gSpriteDown[0].x = 0;
		gSpriteDown[0].y = 85;
		gSpriteDown[0].w = 55;
		gSpriteDown[0].h = 46;

		gSpriteDown[1].x = 0;
		gSpriteDown[1].y = 166;
		gSpriteDown[1].w = 55;
		gSpriteDown[1].h = 40;


	}
	if (!gHammerBroAnimationPage.loadFromFile("Hammer Bro.png"))
	{
		printf("Failed to load mario texture!\n");
		success = false;
	}

	else
	{
		hammerbros[0].x = 55;
		hammerbros[0].y = 97;
		hammerbros[0].w = 72;
		hammerbros[0].h = 68;

		for (a = 1; a < 8; a++)
		{

			hammerbros[a].x = hammerbros[a - 1].x + 80;
			hammerbros[a].y = 97;
			hammerbros[a].w = 72;
			hammerbros[a].h = 68;

			if (a == 5)
			{
				hammerbros[a].x = hammerbros[a - 1].x + 5;
			}
		}

	}

	if (!gBackgroundTexture.loadFromFile("mario_texture.png"))
	{
		printf("failed to load hammerbro texture!\n");
		success = false;
	}
	return success;
}

void close()
{
	//Free loaded images
	gMarioAnimationPage.free();
	gBackgroundTexture.free();
	gHammerBroAnimationPage.free();
	//Destroy window
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}


bool checkCollision(SDL_Rect a, SDL_Rect b)
{
	//The sides of the rectangles
	int leftA, leftB;
	int rightA, rightB;
	int topA, topB;
	int bottomA, bottomB;

	//Calculate the sides of rect A
	leftA = a.x;
	rightA = a.x + a.w;
	topA = a.y;
	bottomA = a.y + a.h;

	//Calculate the sides of rect B
	leftB = b.x;
	rightB = b.x + b.w;
	topB = b.y;
	bottomB = b.y + b.h;

	//If any of the sides from A are outside of B
	if (bottomA <= topB)
	{
		return false;
	}

	if (rightA <= leftB)
	{
		return false;
	}

	if (leftA >= rightB)
	{
		return false;
	}
	if (topA >= bottomB)
	{
		return false;
	}


	return true;
}




	void spawnMario();
	void crouchright();
	void crouchleft();
	void DoingNothing();
	void gravity();
	void move();
	int marioPosX;
	int marioPosY;
	int marioVelX;
	int marioVelY;

	static const int left = 1;
	static const int right = 2;
	static const int up = 3;
	bool ON;
	bool jump;
	bool liftoff;
	bool shiftdown;
	int animationFraction = 4;
	int frame3;

void HammerBro()
{
		if (frame3/20 >= 8)
		{
			frame3 = 0;
		}
		HB = &hammerbros[frame3/20];
		++frame3;
		SDL_RenderClear(gRenderer);
		gBackgroundTexture.render(0, 0);
		gHammerBroAnimationPage.render(300, 290, HB);
}

void RenderandStore()
{
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);



		gMarioAnimationPage.render(marioPosX, marioPosY, currentClip);
		tempbox = *currentClip;
		MarioCollisionBox = { marioPosX , marioPosY, tempbox.w, tempbox.h };

		SDL_RenderPresent(gRenderer);
}

void spawnMario()
{
	marioPosX = 100;
	marioPosY = 290;
	marioVelX = 0;
	marioVelY = 0;
	tempbox = { 0,0,0,0 };
	currentClip = &gSpriteClips[0];
	HammerBro();
	RenderandStore();

}

void move()
{
	if (direction == left && marioPosX > 0)
	{
		marioPosX += marioVelX;
	}
	if (direction == right && marioPosX < SCREEN_WIDTH - 50)
	{
		marioPosX += marioVelX;
	}
	if (direction == left)
	{

			++frame;
			if (frame / animationFraction >= WALKING_ANIMATION_FRAMES)
			{
				frame = 0;
			}
			currentClip = &gSpriteClips[frame / animationFraction];
	}
	else if (direction == right)
	{

			++frame2;
			if (frame2 / animationFraction >= WALKING_ANIMATION_FRAMES)
			{
				frame2 = 0;
			}
			currentClip = &gSpriteRight[frame2 / animationFraction];
	}
	HammerBro();
	RenderandStore();
	gravity();
}



void crouchleft()
{
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(gRenderer);
	gBackgroundTexture.render(0, 0);
	currentClip = &gSpriteDown[0];
	gMarioAnimationPage.render(marioPosX, marioPosY + 18, currentClip);
	tempbox = *currentClip;

	MarioCollisionBox = { marioPosX , marioPosY + 18, tempbox.w, tempbox.h };

	SDL_Delay(5);
	SDL_RenderPresent(gRenderer);
}

void  crouchright()
{
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(gRenderer);
	gBackgroundTexture.render(0, 0);
	currentClip = &gSpriteDown[1];
	gMarioAnimationPage.render(marioPosX, marioPosY + 25, currentClip);
	tempbox = *currentClip;

	MarioCollisionBox = { marioPosX , marioPosY + 18, tempbox.w, tempbox.h };

	SDL_Delay(5);
	SDL_RenderPresent(gRenderer);
}

bool onPlatform()
{
	ON = false;

	temporaryBox = { (MarioCollisionBox.x) + ((MarioCollisionBox.w)/2) - 15, MarioCollisionBox.y +MarioCollisionBox.h - 15, 30, 5 };
	for (a = 0; a < 10; a++)
	{

		if (checkCollision(temporaryBox, platforms[a]))
		{
			ON = true;
			break;
		}

	}
	return ON;
}

void gravity()
{
	while (liftoff || !onPlatform())
	{

			liftoff = false;
			if (jump)
			{
				marioPosY -= 14;

			}
			if (marioPosX < SCREEN_WIDTH - 50 && marioPosX > 0)
			{
				marioPosX += marioVelX;
			}
			marioVelY++;
			marioPosY += marioVelY;
			HammerBro();
			RenderandStore();
		}

		jump = false;
}

void renderPlatforms()
{
	platforms[0].x = 330;
	platforms[0].y = 301;
	platforms[0].w = 10;
	platforms[0].h = 8;

	platforms[1].x = 348;
	platforms[1].y = 275;
	platforms[1].w = 10;
	platforms[1].h = 8;


	platforms[2].x = 382;
	platforms[2].y = 249;
	platforms[2].w = 10;
	platforms[2].h = 8;


	platforms[3].x = 415;
	platforms[3].y = 218;
	platforms[3].w = 30;
	platforms[3].h = 8;


	platforms[4].x = 477;
	platforms[4].y = 160;
	platforms[4].w = 30;
	platforms[4].h = 8;


	platforms[5].x = 509;
	platforms[5].y = 130;
	platforms[5].w = 30;
	platforms[5].h = 8;


	platforms[6].x = 540;
	platforms[6].y = 103;
	platforms[6].w = 30;
	platforms[6].h = 8;


	platforms[7].x = 448;
	platforms[7].y = 190;
	platforms[7].w = 30;
	platforms[7].h = 8;


	platforms[8].x = 90;
	platforms[8].y = 156;
	platforms[8].w = 290;
	platforms[8].h = 10;

	platforms[9].x = 0;
	platforms[9].y = 355;
	platforms[9].w = 640;
	platforms[9].h = 100;
}



int main(int argc, char* args[])
{

	//Start up SDL and create window
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		//Load media
		if (!loadMedia())
		{
			printf("Failed to load media!\n");
		}
		else
		{

			//Main loop flag
			bool quit = false;

			//Event handler
			SDL_Event e;
			frame = 0;
			frame2 = 0;
			frame3 = 0;
			jump = false;
			liftoff = false;
			shiftdown = false;

			spawnMario();

			while (!quit)
			{
				HammerBro();

				RenderandStore();


				//Handle events on queue
				while (SDL_PollEvent(&e) != 0)
				{
					renderPlatforms();
					//User requests quit
					if (e.type == SDL_QUIT)
					{
						quit = true;
					}
					//Set texture based on current keystate
					else if (e.type == SDL_KEYDOWN)
					{
						const Uint8*currentKeyStates = SDL_GetKeyboardState(NULL);
						if (currentKeyStates[SDL_SCANCODE_RSHIFT] || currentKeyStates[SDL_SCANCODE_LSHIFT])
						{
							shiftdown = true;
							animationFraction = 1;
						}

						if (currentKeyStates[SDL_SCANCODE_UP] && e.key.repeat ==0)
						{
							direction = up;
							jump = true;
							liftoff = true;
							gravity();
						}
						else if (currentKeyStates[SDL_SCANCODE_DOWN])
						{
							if (direction == left)
							{
								crouchleft();
							}
							else if (direction == right)
							{
								crouchright();
							}
						}
						else if (currentKeyStates[SDL_SCANCODE_LEFT])
						{
							direction = left;
							if (shiftdown)
							{
								marioVelX = -8;
							}
							else
							{
								marioVelX = -3;
							}

							move();

						}
						else if (currentKeyStates[SDL_SCANCODE_RIGHT])
						{
							direction = right;
							if (shiftdown)
							{
								marioVelX = 8;
							}
							else
							{
								marioVelX = 3;
							}
							move();

						}
						else
						{
							HammerBro();
							RenderandStore();
						}


					}
					else if (e.type == SDL_KEYUP && e.key.repeat == 0)
					{
						animationFraction = 4;
						liftoff = false;

						shiftdown = false;
						marioVelX = 0;
						marioVelY = 0;

						if (direction == right)
						{
							currentClip = &gSpriteRight[0];
						}

						if (direction == left)
						{
							currentClip = &gSpriteClips[0];
						}
						HammerBro();
						RenderandStore();
					}
				}
			}
		}
		//Free resources and close SDL
		close();
		return 0;
	}
}
