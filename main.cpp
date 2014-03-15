//Author    : Joshua Thompson
//PID       : J1344017
//Date      : 8/07/06
//Assignment: Project - Terrain Rendering

/*NOTES: My axis system placed the y axis as going up and down, as reflected in the program

When placing an object or billboard, it has to be placed one away from the far
edge at the furthest.  For example, for a 128x128 terrain, the object can be placed
from 1, 1 to 127, 127.  For non-square terrains, the x-coordinate of the object
must be smaller than width, and the y-coordinate (really z-coordinate for my system)
must be smaller than length, where width and length are defined in the map pgm file

Filenames *must* not have spaces in them

The origin of the terrain corresponds to the bottom left point in the pgm file

x-axis    y-axis
\     |
 \    |
  \   |
   \  |
    \ |
     \|__________ Z-axis

ranges:
x: [0, map->height - 1]
y: [minHeight, maxHeight]
z: [0, map->width - 1]

My environment starts with x and z equal to zero, even though objects and billboards
are placed starting from 1, 1.  I take care of this in the program, but for mapfiles
with unequal width and heights, some consideration needs to be made when defining where
to place objects.  For example, the feep.pgm included in the zip has width 24 and length 7.
All objects/billboards can be defined anywhere from 1 1 to 24 7 in this case.  Objects/billboards
are placed at the vertex specified, so placing it at 1 1 will put it at the very corner!

Environment file is a global variable, change it to change what file to use

When g is hit, the user can switch windows by either alt-tabbing or by leaving the window from the
right side, as I use glutWarpPointer to force the user's pointer to stay within the window

Sun is set to be at the opposite corner from where you start, and 3 times the height range
(maxHeight - minHeight) high

HEIGHT must be first keyword in environment file, or at least be before any object or billboard
declarations, because I use the values read in the height map to determine whether or not to insert
objects or billboards into their respective linked lists, as I don't insert them if they're defined
to be outside of the terrain.
*/

#include <Windows.h>
#include "SDL.h"
#include "SDL_mixer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glut.h>
#include <time.h>
#include <math.h>
#include <glaux.h>
#include "User.h"
#include "SphereRing.h"
#include "Box.h"

const double PI = 3.14159265358979323846;

#define STATIC 0					//for OBJ types
#define ANIMATED 1					//for OBJ types
#define TILED 0						//tile the terrain texture, applying it to each quad
#define STRETCHED 1					//stretch the whole texture over the entire terrain
#define MAXBILLBOARDS 20			//max billboards per texture

int width=1280, height=960;			//initial window values
float FPS = 100;
float speed = 1;

bool failureSound = false;
bool passedSound = false;
bool jetPackSound = false;

//Texture array
GLuint texture[10];					//storage for 10 textures
//0 -> terrain texture
//1 -> water texture
//2 -> sky texture
//3,4 -> sun textures
//rest -> billboard textures

//Call lists
GLuint terrainList;	//for terrain call list
GLuint waterList;   //for water call list
GLuint *objectList;	//for object call list
GLuint lightList[7];

//Values read in from the environment
int XLEN = 1;						//xlength of quad mesh
int ZLEN = 1;						//zlength of quad mesh (I define y as up)
int maxval;							//maximum gray value
float minHeight, maxHeight;			//min and max y-values defined by environment file
float waterHeight, waterOsc;		//water height and how much it oscillates
int skydomeAngle;					//how much the skydome rotates

//variables storing which things exist
int textureExists = 0;				//whether a texture will cover the terrain
int waterExists = 0;				//whether or not there is water
int skyExists = 0;
int sun1Exists = 0;
int sun2Exists = 0;

//mouse variables
float mouseXsens = -.1f;				//mouse sensitivities: increase to increase
float mouseYsens = -.1f;				//sensitivity, negative values invert look
float mouseX = 0;					//stores x value when mouse is clicked
float mouseY = 0;					//stores y value when mouse is clicked
float oldMouseX = 0;				//stores change in x when mouse is unclicked
float oldMouseY = 0;				//stores change in y when mouse is unclicked
int oldY;
float dx = width/2;					//change in mouse x coordinate, used as an amount
//to rotate around y axis (up) to pan the screen
float dy = height/2;				//change in mouse y coordinate, used as an amount
//to rotate around x axis to look higher up or down
int whatButton = 0;					//what mouse button is clicked 
//(0 = none, 1 = left, 2 = right, 3 = both)

//variables affecting how fast things are animated (alter these according to system speed)
int animationSpeed = 5;				//affects animation speed for objects, etc. (lower = faster)

int skySpeed = 50;	//affects sky speed.  Even though the project description
//said to rotate it X degrees per render, as determined
//by the environment file, this tended to cause it to rotate
//hundreds of revolutions per second, so I don't want to cause
//seizures in people viewing this on faster computers
//(lower = faster)

int waterSpeed = 100;				//how fast water oscillates

float jumpFactor = 0;
bool jumped = 0;
float jumpHeight;					//height in which the user jumps
float g = -90;
float yVel = 0;
int crouchFactor = 0;

//water variables
float waterHeightMax;
float waterHeightMin;				//min and max water heights, aka +/- waterOsc
int waterSign = 1;

//skydome variables
int angle;							//updates rotation angle based on skydomeAngle
float skyRadius;					//radius of the sky

//billboard variables
int curTexture = 5;					//which texture to associate with the billboard

//sun variables
int whichSun = 0;					//which sun to render (based on random seed)


//other variables

int numOfObjects = 0;
float terrainAngle;					//represents the initial angle to look at, for a square
//map this should be pi/4 radians
float scaleFactor = 1;				//how much to scale objects/billboards
int tileTerrain = TILED;			//change this variable to make the terrain STRETCHED or TILED

float*** normals;					//like terrain, but holds normal vector for each quad
float*** averageNormals;			//holds average normal vector for each vertex

float viewer[3] = {0, 50, 0};		//position of camera
int bmpx, bmpy;						//stores image width and height when textures are loaded

char *environmentFile = "environment.txt"; //the environment file to be read in

User player;

SphereRing *firstRing, *lastRing, *currentRing, *sphRing;

Box box(8, 3, 8, 10, 10, 10);

//image struct used to store the heightmap
typedef struct 
{
  double **grayValues;	//read in from the .pgm and later converted to actual heights
  int width;
  int height;
} image_type;

image_type *map;

//obj type for objects defined in the environment file
typedef struct obj
{
  float ***values;				//stores all the vertices etc. values
  float minHeight;				//the bottom of the object
  int objType;					//animated or static
  char *objName;					//object's filename
  int *modLines;					//lines of vertices etc. per file
  int xpos;						
  int zpos;
  int fileCount;					//how many files are in the animation
  int Counter;					//which file to animate in the frame
  struct obj *next, *previous;
} obj;

obj *first, *last, *current, *ob;	//these are used for the linked list, like in hw1

//billboard type for billboards defined in the environment file
typedef struct billboard
{
  int texNumber;									//the texture index for textures[]
  int numOfBoards;								//how many billboards from env. file
  int xpos[MAXBILLBOARDS], zpos[MAXBILLBOARDS];
  int width, height;
  struct billboard *next, *previous;
} billboard;

billboard *firstBillboard, *lastBillboard, *currentBillboard, *bb; //these are used for the linked list, like in hw1

//light type used for setting material properties
typedef struct light_t
{
  float ambient[4];
  float diffuse[4];
  float specular[4];
  float emission[4];
  float shininess;
} light_t;

light_t landlight, waterlight, objectlight, billboardlight, ringlighta, ringlightb, boxlight;

//Load bmps and convert to textures
bool loadTextures(char *filename, int i)		
{
  AUX_RGBImageRec *textureImage = auxDIBImageLoad(filename);

  //Load the bmp and check for errors
  if (textureImage == NULL) return false;

  glGenTextures(1, &texture[i]);					//Create the texture

  //Texture generation
  glBindTexture(GL_TEXTURE_2D, texture[i]);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, textureImage->sizeX, textureImage->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, textureImage->data);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

  //stores the bitmap size information into global variables
  bmpx = textureImage->sizeX;
  bmpy = textureImage->sizeY;

  if (textureImage->data)
    free(textureImage->data);
  free(textureImage);

  return true;										//Returns whether or not it succeeds
}
/*
void PlaySound(Mix_Chunk *sound)
{
//Play the sound on the first channel available, with no looping
if(Mix_PlayChannel(-1, sound, 0)==-1) 
{
//Must've been an error playing the sound!
printf("Mix_PlayChannel: %s\n", Mix_GetError());
}
}
*/
void billboardInsert(char *name, char *filename, int coords[MAXBILLBOARDS][2], int count)
{
  int i;
  billboard *temp;

  printf("	Loading billboard . . . . . . ");
  temp = (billboard *)malloc(sizeof(billboard));
  temp->texNumber = curTexture;					//set the billboard texture number to curTexture
  temp->numOfBoards = count;						//how many billboards are listed in the line in the env. file

  if(loadTextures(filename, temp->texNumber))		//if it loads the texture correctly
  {
    for(i = 0; i < count; i++)
    {
      //inserts the billboard into the linked list
      temp->width = bmpx;
      temp->height = bmpy;
      temp->xpos[i] = coords[i][0];
      temp->zpos[i] = coords[i][1];

      temp->previous = currentBillboard;
      temp->next = lastBillboard;
      currentBillboard->next = temp;
      currentBillboard = currentBillboard->next;
      currentBillboard->next = lastBillboard;
      lastBillboard->previous = currentBillboard;
    }

    curTexture++;
  }
  else
  {
    //prints a failure statement :(
    printf("FAILED\n\t\tCould not open billboard: %s\n", filename);
  }
}

//inserts the object into a double linked list
void objInsert(int type, char name[], int x, int z)
{
  obj *temp;
  FILE *f;
  char dummy;
  char text[256];
  char newname[32];
  char buffer[256];
  int line = 0;
  int i, j;
  int fileCounter = 1;

  bool status = true;

  temp = (obj *)malloc(sizeof(obj));

  //inserts a static object
  if(type == STATIC)
  {
    temp->fileCount = 1;
    temp->values = (float ***)malloc(1*sizeof(float **));
    temp->modLines = (int *)malloc(1*sizeof(int));
    f = fopen(name, "r");
    if(f == NULL)
    {
      printf("FAILED\n\t\tCould not open object file: %s\n", name);
      status = false;
    }
    else
    {
      //first it reads how many lines of vertices, etc., there are in the .mod file
      while(fgets(text, 128*sizeof(char), f) != NULL)
      {
        if (text[0] == 'N')
          line++;
      }
      temp->modLines[0] = line;

      fclose(f);
      //dynamically allocates the proper amount of space based on how many lines there are
      temp->values[0] = (float **)malloc(line*sizeof(float *));

      //allocates 8 elements: first 3 are Normals, next 2 are for Tex coords, last 3 are vertices
      for(i = 0; i < line; i++)
        temp->values[0][i] = (float *)malloc(8*sizeof(float));

      f = fopen(name, "r");

      for(i = 0; fgets(text, 128*sizeof(char), f) != NULL; i++)
      {

        //scans in the values for a line.  Note: I have it going as N, N[2],
        //and N[1] because I took the y-direction as up and down, so putting
        //it in this order made the object right-side-up.

        sscanf(text, "%c%f%f%f%c%c%f%f%c%c%f%f%f", &dummy, &(temp->values[0][i][0]), 
          &(temp->values[0][i][2]), &(temp->values[0][i][1]), &dummy, &dummy,
          &(temp->values[0][i][3]), &(temp->values[0][i][4]), &dummy, &dummy, 
          &(temp->values[0][i][5]), &(temp->values[0][i][7]), &(temp->values[0][i][6]));
      }
      fclose(f);

      //finds the minimum height of the object
      temp->minHeight = temp->values[0][0][6];
      for(i = 0; i < temp->modLines[0]; i++)
      {
        if (temp->values[0][i][6] < temp->minHeight)
          temp->minHeight = temp->values[0][i][6];
      }
      printf("DONE\n");
    }
  }
  else if (type == ANIMATED)
  {
    //gets the base name (ie anim.mod becomes anim)
    for (i = 0; name[i] != '.'; i++)
      newname[i] = name[i];
    newname[i] = '\0';

    //calculates how many files are in a row
    for(sprintf(buffer, "%s%.2d%s%c", newname, fileCounter, ".mod", '\0'); 
      (fopen(buffer, "r") != NULL); )
    {
      fileCounter++;
      sprintf(buffer, "%s%.2d%s%c", newname, fileCounter, ".mod", '\0');
    }

    //fileCounter will end up being one more than the number of files, so reduce it by one
    temp->fileCount = fileCounter - 1;
    //allocates memory like in the static case
    temp->values = (float ***)malloc((temp->fileCount)*sizeof(float **));
    temp->modLines = (int *)malloc((temp->fileCount)*sizeof(int));

    //this is the same as for the static case, but this does it for each file
    for(i = 0; i < temp->fileCount; i++)
    {
      line = 0;
      sprintf(buffer, "%s%.2d%s%c", newname, i+1, ".mod", '\0');
      f = fopen(buffer, "r");
      while(fgets(text, 128*sizeof(char), f) != NULL)
      {
        if (text[0] == 'N')
          line++;
      }
      fclose(f);

      temp->modLines[i] = line;
      temp->values[i] = (float **)malloc((temp->modLines[i])*sizeof(float *));
      for(j = 0; j < temp->modLines[i]; j++)
        temp->values[i][j] = (float *)malloc(8*sizeof(float));

      f = fopen(buffer, "r");

      for(j = 0; fgets(text, 128*sizeof(char), f) != NULL; j++)
      {

        //scans in the values for a line.  Note: I have it going as N, N[2],
        //and N[1] because I took the y-direction as up and down, so putting
        //it in this order made the object right-side-up.

        sscanf(text, "%c%f%f%f%c%c%f%f%c%c%f%f%f", &dummy, &(temp->values[i][j][0]), 
          &(temp->values[i][j][2]), &(temp->values[i][j][1]), &dummy, &dummy,
          &(temp->values[i][j][3]), &(temp->values[i][j][4]), &dummy, &dummy, 
          &(temp->values[i][j][5]), &(temp->values[i][j][7]), &(temp->values[i][j][6]));
      }
      fclose(f);

      temp->minHeight = temp->values[0][0][6];

      for(j = 0; j < temp->modLines[0]; j++)
      {
        if (temp->values[0][j][6] < temp->minHeight)
          temp->minHeight = temp->values[0][j][6];
      }
    }
    //if the fileCounter is 1, that means it didn't load any files correctly, which is bad :(
    if(fileCounter == 1)
    {
      printf("FAILED\n\t\tCould not open object file starting with: %s\n", newname);
      status = false;
    }
    else
      printf("DONE\n");
  }

  //if the object was successfully loaded, insert it into the linked list
  if(status)
  {
    temp->Counter = 0;
    temp->objType = type;
    temp->xpos = x;
    temp->zpos = z;
    temp->objName = name;

    temp->previous = current;
    temp->next = last;
    current->next = temp;
    current = current->next;
    current->next = last;
    last->previous = current;

    numOfObjects++;
  }
}

//creates an image_type
image_type *new_image(image_type *image, int w, int h, int max)
{
  int i;

  image->width = w;
  image->height = h;
  maxval = max;

  //creates a 2D array to store grayscale values
  image->grayValues = (double **)malloc(h*sizeof(double*));
  for (i = 0; i < h; i++)
    image->grayValues[i] = (double *)malloc(w*sizeof(double));

  return image;
}

//reads an image_type
image_type *read_image(image_type *image, char *filename)
{
  int i, j;
  FILE *inf;
  char temp[256];
  int width, height,color;
  inf = fopen(filename, "r+");
  //some error handling
  if((inf = fopen(filename, "r+")) == NULL)
  {
    printf("Unable to open pgm file: %s!\n", filename);
    exit(1);
  }
  if(fscanf(inf, " P2 "))
  {
    printf("not a valid pgm: %s!\n",filename);
    exit(1);
  }

  fgets(temp, 128 * sizeof(char), inf);
  while(temp[0] == '#')						//skips commented lines
    fgets(temp, 128 * sizeof(char), inf);

  sscanf(temp, "%d %d", &width, &height);		//scans for width, height
  fscanf(inf, "%d", &maxval);					//and maxval

  //creates an image_type based on information in the .pgm file
  image = new_image(image, width, height, maxval);

  //fills the 2D array with grayscale vlues from .pgm file
  for(i=image->height - 1; i >= 0; i--)
  {
    for(j=0; j<image->width; j++)
    {
      fscanf(inf, "%d ", &color);
      image->grayValues[i][j] = (double) color;
    }
  }
  fclose(inf);

  return image;
}

//reads the environment file from myinit()
void read_environment(char *filename)
{
  char keyword[10];
  char param1[20];
  char param2[20];
  char param3[4];
  char param4[4];
  char temp[256];
  int bbcoords[MAXBILLBOARDS][2];
  int i, j, k, objx, objz, failCount = 0;
  int x, y, z, num;
  float ang, sphRad, rRad;
  //these variables prevent the environment file from having two lines specifying things like
  //texture, heightmap, water, etc.
  int readHeight = 0, readTexture = 0, readLengths = 0, readWater = 0, readSky = 0, readSun = 0;
  FILE *f;

  printf("Loading environment file: %s\n", filename);
  f = fopen(filename, "r");

  //grabs a new line from the env. file
  while(fgets(temp, 128*sizeof(char), f) != NULL)
  {
    //skips any blank lines
    if(temp[0] == '\n')
      temp[0] = ' ';
    for (i = 0; temp[i] != ' '; i++)
      keyword[i] = temp[i];
    keyword[i] = '\0';

    //compares the keyword to the possible keywords of
    //HEIGHT, LENGTHS, TEXTURE, OBJ, BILLBOARD, SKYDOME, WATER, SUN
    if(strcmp(keyword, "HEIGHT") == 0)
    {
      printf("	Loading height map  . . . . . ");
      if(readHeight == 0)
      {
        while(temp[i] == ' ')	//skip whitespace
          i++;

        for (j = 0; temp[i] != ' '; j++, i++)	//read in the name of the heightmap
          param1[j] = temp[i];
        param1[j] = '\0';

        map = (image_type *)malloc(sizeof(image_type)); //create a map image
        map = read_image(map, param1); //read the map image

        //scans for minHeight and maxHeight
        sscanf(temp + sizeof(char)*i, "%f %f", &minHeight, &maxHeight);
        readHeight = 1;
        printf("DONE\n");
      }
      else
        printf("FAILED\n\t\tAlready read height map!\n");
    }
    else if(strcmp(keyword, "LENGTHS") == 0)
    {
      printf("	Loading lengths . . . . . . . ");
      if(readLengths == 0)
      {
        //scans the XLEN and ZLEN values from the string
        sscanf(temp + sizeof(char)*i, "%d %d", &XLEN, &ZLEN);
        readLengths = 1;
        printf("DONE\n");
      }
      else
        printf("FAILED\n\t\tAlready read lengths!\n");
    }
    else if (strcmp(keyword, "TEXTURE") == 0)
    {
      printf("	Loading land texture  . . . . ");
      if(readTexture == 0)
      {
        while(temp[i] == ' ')	//skips white space
          i++;
        //reads in the name of the texture.  For future reference, the strlen(temp)
        //part is in case the texture is the last line, and there is no newline character
        for (j = 0; temp[i] != '\n' && temp[i] != ' ' && i < (int)strlen(temp); j++, i++)
          param1[j] = temp[i];
        param1[j] = '\0';

        //Loads the texture and sets the textureExists parameter
        textureExists = loadTextures(param1, 0);
        if(textureExists)
          printf("DONE\n");
        else
          printf("FAILED\n\t\tCould not open texture: %s\n", param1);

        readTexture = 1;
      }
      else
        printf("FAILED\n\t\tAlready read land texture!\n");
    }
    else if (strcmp(keyword, "OBJ") == 0)
    {
      //gets the first parameter, whether it is animated or not
      while(temp[i] == ' ')
        i++;
      for(j = 0; temp[i] != ' '; j++, i++)
        param1[j] = temp[i];
      param1[j] = '\0';

      //gets the second parameter, the object's name
      while(temp[i] == ' ')
        i++;
      for(j = 0; temp[i] != ' '; j++, i++)
        param2[j] = temp[i];
      param2[j] = '\0';

      //scans the string for the object's x and z values
      //note that the first parameter is objz since in the pdf
      //it is described as "column, row", and when reading in
      //from the .pgm file, I define rows as marked by x and
      //columns as marked by y
      sscanf(temp + sizeof(char)*i, "%d %d", &objz, &objx);

      //inserts the object into the linked list
      if(strcmp(param1, "Static") == 0)
      {
        printf("	Loading static object . . . . ");
        if(objx > map->height || objz > map->width)
          printf("FAILED\n\t\tObject outside of terrain\n");
        else
          objInsert(STATIC, param2, objx, objz);
      }
      else if(strcmp(param1, "Animated") == 0)
      {
        printf("	Loading animated object . . . ");
        if(objx > map->height || objz > map->width)
          printf("FAILED\n\t\tObject outside of terrain\n");
        else
          objInsert(ANIMATED, param2, objx, objz);
      }
    }
    else if (strcmp(keyword, "BILLBOARD") == 0)
    {
      while(temp[i] == ' ')	//skip whitespace
        i++;
      for(j = 0; temp[i] != ' '; j++, i++)	//get name of billboard
        param1[j] = temp[i];
      param1[j] = '\0';

      while(temp[i] == ' ')	//skip whitespace
        i++;
      for(j = 0; temp[i] != ' '; j++, i++)	//get filename of billboard
        param2[j]= temp[i];
      param2[j] = '\0';

      k = 0;
      //get all the x/z coordinates
      while(i < (int)strlen(temp))
      {
        if(k == MAXBILLBOARDS)	//I set MAXBILLBOARDS to 20
        {
          printf("	Warning: maximum billboard limit surpassed!\n");
          break;
        }
        while(temp[i] == ' ' || temp[i] == '\n')	//skip whitespace
          i++;
        if(i < (int)strlen(temp))
        {
          for(j = 0; temp[i] != ' '; j++, i++)
            param3[j] = temp[i];
          param3[j] = '\0';

          while(temp[i] == ' ')
            i++;

          for(j = 0; temp[i] != ' ' && temp[i] != '\n' && i < (int)strlen(temp); j++, i++)
            param4[j] = temp[i];
          param4[j] = '\0';

          //scans z coordinate first then x coordinate for the same reason as objects
          sscanf(param3, "%d", &bbcoords[k][1]);
          sscanf(param4, "%d", &bbcoords[k][0]);
          //if the billboard is outside of the terrain, increase failCount
          if(bbcoords[k][0] > map->height || bbcoords[k][1] > map->width)
            failCount++;
          else
            k++;
        }
      }
      billboardInsert(param1, param2, bbcoords, k);
      if(failCount)
        printf("DONE WITH ERRORS\n\t\tOnly %d of %d billboards loaded!\n\t\t%d billboards outside of terrain\n", k, k + failCount, failCount);
      else
        printf("DONE\n");
    }
    else if (strcmp(keyword, "SKYDOME") == 0)
    {
      printf("	Loading skydome texture . . . ");
      if(readSky == 0)
      {	
        while(temp[i] == ' ')	//skip whitespace
          i++;
        //get the bmp name for the skydome
        for(j = 0; temp[i] != ' '; j++, i++)
          param1[j] = temp[i];
        param1[j] = '\0';

        //scan in the rotation angle
        sscanf(temp + sizeof(char)*i, "%d", &skydomeAngle);
        angle = skydomeAngle;	//set angle to skydomeAngle, since angle is what's actually going to be increased
        if(skyExists = loadTextures(param1, 2))
          printf("DONE\n");	//load texture
        else
          printf("FAILED\n\t\tCould not open texture: %s\n", param1);

        readSky = 1;
      }
      else
        printf("FAILED\n\t\tAlready read skydome texture!\n");
    }
    else if (strcmp(keyword, "WATER") == 0)
    {
      printf("	Loading water texture . . . . ");
      if(readWater == 0)
      {
        while(temp[i] == ' ')	//skip whitespace
          i++;
        //get the bmp name for the water
        for(j = 0; temp[i] != ' '; j++, i++)
          param1[j] = temp[i];
        param1[j] = '\0';

        //scan for water height and amount of oscillation
        sscanf(temp + sizeof(char)*i, "%f %f", &waterHeight, &waterOsc);

        if(waterExists = loadTextures(param1, 1))
          printf("DONE\n");	//load texture
        else
          printf("FAILED\n\t\tCould not open texture: %s\n", param1);
        readWater = 1;
      }
      else
        printf("FAILED\n\t\tAlready read water values!\n");
    }
    else if (strcmp(keyword, "SUN") == 0)
    {
      if(readSun == 0)
      {
        printf("	Loading sun texture 1 . . . . ");
        while(temp[i] == ' ')	//skip whitespace
          i++;
        //get bmp name for first sun
        for(j = 0; temp[i] != ' '; j++, i++)
          param1[j] = temp[i];
        param1[j] = '\0';

        while(temp[i] == ' ')	//skip whitespace
          i++;
        //get bmp name for second sun
        for(j = 0; temp[i] != ' ' && temp[i] != '\n' && i < (int)strlen(temp); j++, i++)
          param2[j] = temp[i];
        param2[j] = '\0';

        sun1Exists = loadTextures(param1, 3);	//load texture
        if(sun1Exists)
        {
          printf("DONE\n");
          whichSun = 3;
        }
        else
          printf("FAILED\n\t\tCould not open sun: %s", param1);

        printf("	Loading sun texture 2 . . . . ");
        sun2Exists = loadTextures(param2, 4);	//load texture
        if(sun2Exists)
        {
          printf("DONE\n");
          whichSun = 4;
        }
        else
          printf("FAILED\n\t\tCould not open sun: %s", param2);

        readSun = 1;
      }
      else
        printf("	Loading sun texture 1 . . . . FAILED\n\t\tAlready read sun textures!\n");
    }
    else if (strcmp(keyword, "RING") == 0)
    {
      printf("	Loading ring  . . . . . . . . ");

      while(temp[i] == ' ') //skip whitespace
        i++;

      sscanf(temp + sizeof(char)*i, "%d %d %d %f %d %f %f", &x, &y, &z, &ang, &num, &sphRad, &rRad);
      SphereRing ring(x, y, z, ang, num, sphRad, rRad);
      ring.insert();

      printf("DONE\n");
    }
  }

  printf("Loading complete, executing program\n");

  fclose(f);
}

void quit()
{
  int i, j;

  free(objectList);
  for(i = 0; i < map->height; i++)
  {
    for(j = 0; j < map->width; j++)
      free(averageNormals[i][j]);
    free(averageNormals[i]);
  }
  free(averageNormals);
  SDL_Quit();
  exit(1);
}

void getNormal(float *norm,float pointa[3],float pointb[3],float pointc[3])
{
  float vect[2][3];	//holds the two vectors that are crossed to get the normal
  float point[3][3];	//holds the three points passed to getNormal
  float magnitude;	//for normalizing the vector
  int i;

  //puts the points into one array
  for (i = 0; i < 3; i++)
  {
    point[0][i]=pointa[i];
    point[1][i]=pointb[i]; 
    point[2][i]=pointc[i];
  }

  //calculate the two vectors by subtracting one point from the other
  for (i = 0; i < 3; i++)
    vect[0][i] = point[1][i] - point[0][i];
  for (i = 0; i < 3; i++)
    vect[1][i] = point[2][i] - point[0][i];

  //norm represents the cross product which I already calculated out using
  //the determinant of a matrix
  norm[0] = vect[0][1]*vect[1][2] - vect[0][2]*vect[1][1];
  norm[1] = vect[0][2]*vect[1][0] - vect[0][0]*vect[1][2];
  norm[2] = vect[0][0]*vect[1][1] - vect[0][1]*vect[1][0];

  //normalize the normal vector
  magnitude = sqrt(pow(norm[0],2) + pow(norm[1],2) + pow(norm[2],2));

  for (i = 0; i < 3; i++)
    norm[i] /= magnitude;
}

//I chose to average the normals to achieve a better lighting effect
void averageNormal()
{
  int i, j, k;
  float magnitude;
  //averages the normals for each vertex
  for(i = 0; i < map->height; i++)
  {
    for(j = 0; j < map->width; j++)
    {

      //the first three if/ else if statements are for the edges, which don't require
      //averaging the normals of the surrounding 4 quads, since there aren't 4 surrounding quads
      if(i == 0 && j == 0)
      {
        averageNormals[i][j][0] = normals[0][0][0];
        averageNormals[i][j][1] = normals[0][0][1];
        averageNormals[i][j][2] = normals[0][0][2];
      }
      else if(i == 0 && j != 0)
      {
        averageNormals[i][j][0] = normals[0][j-1][0] + normals[0][j][0];
        averageNormals[i][j][1] = normals[0][j-1][1] + normals[0][j][1];
        averageNormals[i][j][2] = normals[0][j-1][2] + normals[0][j][2];
      }
      else if(i != 0 && j == 0)
      {
        averageNormals[i][j][0] = normals[i-1][0][0] + normals[i][0][0];
        averageNormals[i][j][1] = normals[i-1][0][1] + normals[i][0][1];
        averageNormals[i][j][2] = normals[i-1][0][2] + normals[i][0][2];
      }
      //for the rest of the vertices, average the four surrounding quad normals
      else
      {
        averageNormals[i][j][0] = normals[i-1][j-1][0] + normals[i][j-1][0]
        + normals[i-1][j][0] + normals[i][j][0];
        averageNormals[i][j][1] = normals[i-1][j-1][1] + normals[i][j-1][1]
        + normals[i-1][j][1] + normals[i][j][1];
        averageNormals[i][j][2] = normals[i-1][j-1][2] + normals[i][j-1][2]
        + normals[i-1][j][2] + normals[i][j][2];
      }

      //normalize the average normal vector
      magnitude = sqrt(pow(averageNormals[i][j][0],2) + 
        pow(averageNormals[i][j][1],2) + pow(averageNormals[i][j][2],2));
      for (k = 0; k < 3; k++)
        averageNormals[i][j][k] /= magnitude;
    }
  }
}

void drawObjects()
{
  int i;
  glDisable(GL_TEXTURE_2D); //I was told to disable textures for objects, even though they have texture coordinates

  //draws each object
  for(ob = first->next, i = 0; ob != last; ob = ob->next, i++)
  {
    glPushMatrix();
    glTranslatef((ob->xpos - 1)*XLEN, 
      map->grayValues[ob->xpos-1][ob->zpos-1] - (ob->minHeight - .001)*scaleFactor, (ob->zpos - 1)*ZLEN);
    glScalef(scaleFactor, scaleFactor, scaleFactor);

    glCallList(objectList[i] + ob->Counter/animationSpeed);

    //if the object is static, ob->Counter is 0, so it will always draw the same object, but if it is
    //animated, I use integer division to modify when the next object in the animation is drawn.  By
    //increasing animationSpeed, it takes more renders for ob->Counter/animationSpeed to actually increase.
    //Also, the CallList is sequential, as when it's an animated object, I call for a group of sequential lists,
    //based on how many files are loaded in the animation.
    ob->Counter++;
    if(ob->Counter > animationSpeed*ob->fileCount - 1)
      ob->Counter = 0;
    glPopMatrix();
  }
}

void drawObjectList(obj *curObject, int frame)
{
  int i;

  //draws the triangles for every line in the .mod file
  glBegin(GL_TRIANGLES);
  for(i = 0; i < curObject->modLines[frame]; i++)
  {
    glNormal3f(curObject->values[frame][i][0], curObject->values[frame][i][1], 
      curObject->values[frame][i][2]);
    //Guess I don't need to call texture coordinates if there is no texture :D
    //glTexCoord2f(curObject->values[frame][i][3], curObject->values[frame][i][4]);
    glVertex3f(curObject->values[frame][i][5], curObject->values[frame][i][6], 
      curObject->values[frame][i][7]);
  }
  glEnd();
}

void drawBillboards()
{
  int k;
  float angle;
  float lookAt[2]; //vector between the camera and the billboard, projected to the xz axis

  glEnable(GL_TEXTURE_2D);

  //draws all the billboards
  for(bb = firstBillboard->next; bb!= lastBillboard; bb = bb->next)
  {
    glBindTexture(GL_TEXTURE_2D, texture[bb->texNumber]);

    //draws each billboard from the line in the env. file
    for(k = 0; k < bb->numOfBoards; k++)
    {
      //gets the vector between the camera and the billboard, and calculates the angle
      lookAt[0] = (bb->xpos[k] - 1)*XLEN - player.getX(); //x component of lookAt vector
      lookAt[1] = (bb->zpos[k] - 1)*ZLEN - player.getZ(); //z component of lookAt vector
      angle = atan(lookAt[0]/lookAt[1])*180/PI;

      //this takes care of sign issues
      if (player.getZ() > (bb->zpos[k] - 1)*ZLEN)
        angle = 180 + angle;

      //move the billboard into position and scales it
      glPushMatrix();
      glTranslatef((bb->xpos[k] - 1)*XLEN, map->grayValues[bb->xpos[k] - 1][bb->zpos[k] - 1],
        (bb->zpos[k] - 1)*ZLEN);
      glScalef(scaleFactor, scaleFactor, scaleFactor);
      //note that the rotation angle is such that the billboard isn't facing straight at you, but at
      //a 45 degree angle, and two such rectangles are used.  It kind of looks like: viewer---->X
      //where the X represents the two crossing billboards and the arrow is the line of sight.
      glRotatef(angle + 45, 0, 1, 0);

      //draw the first rectangle, I have the rectangles be essentially the size of the quad
      glBegin(GL_QUADS);
      glTexCoord2f(1.0f, 0.0f);
      glNormal3f(0, 0, -1);
      glVertex3f(-(float)XLEN/2, 0, 0);

      glTexCoord2f(1.0f, 1.0f);
      glVertex3f(-(float)XLEN/2, XLEN*(float)bb->height/bb->width, 0);

      glTexCoord2f(0.0f, 1.0f);
      glVertex3f((float)XLEN/2, XLEN*(float)bb->height/bb->width, 0);

      glTexCoord2f(0.0f, 0.0f);
      glVertex3f((float)XLEN/2, 0, 0);
      glEnd();

      //rotate and draw the second rectangle
      glRotatef(-90, 0, 1, 0);
      glBegin(GL_QUADS);
      glTexCoord2f(1.0f, 0.0f);
      glVertex3f(-(float)XLEN/2, 0, 0);
      glTexCoord2f(1.0f, 1.0f);
      glVertex3f(-(float)XLEN/2, XLEN*(float)bb->height/bb->width, 0);
      glTexCoord2f(0.0f, 1.0f);
      glVertex3f((float)XLEN/2, XLEN*(float)bb->height/bb->width, 0);
      glTexCoord2f(0.0f, 0.0f);
      glVertex3f((float)XLEN/2, 0, 0);
      glEnd();
      glPopMatrix();
    }
  }
  glDisable(GL_TEXTURE_2D);
}

void drawTerrain()
{
  if(textureExists)
    glEnable(GL_TEXTURE_2D);
  else
    glDisable(GL_TEXTURE_2D);

  glCallList(terrainList);

  glDisable(GL_TEXTURE_2D);
}

void drawTerrainList()
{
  int x, z;

  //begin drawing the terrain
  glBegin(GL_QUADS);
  //for each quad, the normal vector is set, and depending on the existance of a texture,
  //either the texture coordinate or color coordinate is set, followed by the vertex itself
  for (x = 0; x < map->height - 1; x++)
  {
    for (z = 0; z < map->width - 1; z++)
    {			
      // draw vertex 0
      glNormal3fv(averageNormals[x][z]);
      if(!textureExists)
        glColor3f(0, 0, (map->grayValues[x][z] - minHeight)/(maxHeight - minHeight));
      else
      {
        if(tileTerrain == STRETCHED)
          glTexCoord2f((float)x/map->height, (float)z/map->width);
        else
          glTexCoord2f(0.0f, 0.0f);
      }
      glVertex3f(x*XLEN, map->grayValues[x][z], z*ZLEN);

      // draw vertex 1
      glNormal3fv(averageNormals[x][z+1]);
      if(!textureExists)
        glColor3f(0, 0, (map->grayValues[x][z+1] - minHeight)/(maxHeight - minHeight));
      else
      {
        if(tileTerrain == STRETCHED)
          glTexCoord2f((float)x/map->height, (float)(z+1)/map->width);
        else
          glTexCoord2f(0.0f, 1.0f);
      }
      glVertex3f(x*XLEN, map->grayValues[x][z+1], (z+1)*ZLEN);

      // draw vertex 2
      glNormal3fv(averageNormals[x+1][z+1]);
      if(!textureExists)
        glColor3f(0, 0, (map->grayValues[x+1][z+1] - minHeight)/(maxHeight - minHeight));
      else
      {
        if(tileTerrain == STRETCHED)
          glTexCoord2f((float)(x+1)/map->height, (float)(z+1)/map->width);
        else
          glTexCoord2f(1.0f, 1.0f);
      }
      glVertex3f((x+1)*XLEN, map->grayValues[x+1][z+1], (z+1)*ZLEN);

      // draw vertex 3
      glNormal3fv(averageNormals[x+1][z]);
      if(!textureExists)
        glColor3f(0, 0, (map->grayValues[x+1][z] - minHeight)/(maxHeight - minHeight));
      else
      {
        if(tileTerrain == STRETCHED)
          glTexCoord2f((float)(x+1)/map->height, (float)z/map->width);
        else
          glTexCoord2f(1.0f, 0.0f);
      }
      glVertex3f((x+1)*XLEN, map->grayValues[x+1][z], z*ZLEN);
    }
  }
  glEnd();
}

void drawWater()
{
  float sign;

  //sign tells you whether or not you are above or under the water,
  //I use it to show the water level when below the water
  sign = (player.getY() - waterHeight)/fabs((player.getY() - waterHeight));

  glColor4f(1.0f, 1.0f, 1.0f, 0.4f);	//basically sets alpha level to .4
  glNormal3f(0.0, sign, 0.0);		//normal depends on whether you are above or below water

  glEnable(GL_TEXTURE_2D);
  glDisable(GL_CULL_FACE);

  glPushMatrix();
  glTranslatef(0, waterHeight, 0);
  /*
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glVertex3f(0, 0, 0);
  glTexCoord2f(0.0, 1.0);
  glVertex3f(0, 0, map->width*ZLEN);
  glTexCoord2f(1.0, 1.0);
  glVertex3f(map->height*XLEN, 0, map->width*ZLEN);
  glTexCoord2f(1.0, 0.0);
  glVertex3f(map->height*XLEN, 0, 0);
  glEnd();

  */	
  glCallList(waterList);

  glPopMatrix();
  //calculates water oscillation
  if (waterHeight > waterHeightMax || waterHeight < waterHeightMin)
    waterSign *= -1;
  waterHeight += waterSign*waterOsc/waterSpeed;
  glEnable(GL_CULL_FACE);
  glDisable(GL_TEXTURE_2D);
}

//called in myinit to set up the call list for water
void drawWaterList()
{
  int x, z;

  glBegin(GL_QUADS);
  for (x = 0; x < map->height - 1; x++)
  {
    for (z = 0; z < map->width - 1; z++)
    {
      // draw vertex 0
      glTexCoord2f(0.0f, 0.0f);
      glVertex3f(x*XLEN, 0, z*ZLEN);
      // draw vertex 1
      glTexCoord2f(0.0f, 1.0f);
      glVertex3f(x*XLEN, 0, (z+1)*ZLEN);
      // draw vertex 2
      glTexCoord2f(1.0f, 1.0f);
      glVertex3f((x+1)*XLEN, 0, (z+1)*ZLEN);
      // draw vertex 3
      glTexCoord2f(1.0f, 0.0f);
      glVertex3f((x+1)*XLEN, 0, z*ZLEN);
    }
  }
  glEnd();
}

void drawSkySphere()
{
  GLUquadricObj *quadratic;	// Storage For Our Quadratic Objects ( NEW )

  GLboolean light_enabled = glIsEnabled(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);

  glColor3f(1, 1, 1);

  //translation makes the skydome center the center of the map
  glPushMatrix();
  glTranslatef(map->height * XLEN/2, 0.5*(maxHeight - minHeight), map->width * ZLEN/2);
  glRotatef((float)angle/skySpeed, 0, 1, 0);
  angle += skydomeAngle;
  if (angle > 360*skySpeed)		//prevents the angle value from spiraling out of control!
    angle -= 360*skySpeed;

  glDisable(GL_CULL_FACE);
  glDisable(GL_LIGHTING);

  glBindTexture(GL_TEXTURE_2D, texture[2]);	//texture array is filled before to include the sky texture

  quadratic=gluNewQuadric();			// Create A Pointer To The Quadric Object (Return 0 If No Memory) (NEW)

  if (light_enabled) gluQuadricNormals(quadratic, GLU_SMOOTH);	// Create Smooth Normals (NEW)
  else gluQuadricNormals(quadratic, GLU_NONE);

  gluQuadricTexture(quadratic, GL_TRUE);				// Create Texture Coords (NEW)

  // Draw A Sphere With given Radius and slice #s, I used
  // geometry to find the minimum radius such that all four
  // corners are within the sphere, then doubled it in case
  // the corners themselves were raised, plus I think it looks better
  gluSphere(quadratic, skyRadius, 32, 32);

  if (light_enabled) glEnable(GL_LIGHTING);

  glEnable(GL_CULL_FACE);
  glDisable(GL_TEXTURE_2D);

  glPopMatrix();
}

void drawSuns()
{
  float lookAt[3];
  float xzAngle;
  float yAngle;
  GLboolean light_enabled = glIsEnabled(GL_LIGHTING);
  glDisable(GL_CULL_FACE);
  glDisable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);

  glColor3f(1.0, 1.0, 1.0);

  //if both suns exist, only switch suns 1% of the time (otherwise it flickers too much!)
  if(sun1Exists && sun2Exists)
  {
    if(rand()%100 + 1 > 99)
    {
      if(whichSun == 3)
        whichSun = 4;
      else
        whichSun = 3;
    }
  }
  glBindTexture(GL_TEXTURE_2D, texture[whichSun]);

  lookAt[0] = map->height*XLEN - player.getX();			//x component of lookAt vector
  lookAt[1] = 3*(maxHeight - minHeight) - player.getY();	//y component of lookAt vector
  lookAt[2] = map->width*ZLEN - player.getZ();			//z component of lookAt vector
  xzAngle = atan(lookAt[0]/lookAt[2])*180/PI;			//calculate angle on xz plane
  if (player.getZ() > (map->width*ZLEN))					//accounts for sign issues with atan
    xzAngle += 180;

  //billboarding in the y direction
  yAngle = atan(lookAt[1]/sqrt(pow(lookAt[0], 2) + pow(lookAt[2], 2)))*180/PI;

  glPushMatrix();
  //moves and rotates the sun to have billboarding effect
  glTranslatef(map->height*XLEN, 3*(maxHeight - minHeight), map->width*ZLEN);
  glRotatef(xzAngle, 0, 1, 0);
  glRotatef(yAngle, -1, 0, 0);

  //draws the sun so that its size is a function of the map size
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glVertex3f(.2*map->height*XLEN,  -.2*map->height*XLEN, 0);

  glTexCoord2f(1.0, 0.0);
  glVertex3f(-.2*map->height*XLEN, -.2*map->height*XLEN, 0);

  glTexCoord2f(1.0, 1.0);
  glVertex3f(-.2*map->height*XLEN, .2*map->height*XLEN, 0);

  glTexCoord2f(0.0, 1.0);
  glVertex3f(.2*map->height*XLEN, .2*map->height*XLEN, 0);
  glEnd();
  glEnable(GL_CULL_FACE);
  if (light_enabled) 
    glEnable(GL_LIGHTING);
  glPopMatrix();

  glDisable(GL_TEXTURE_2D);
}

void drawRings()
{
  SphereRing::ringsPassed = 0;

  for(sphRing = firstRing->next; sphRing != lastRing; sphRing = sphRing->next)
  {
    sphRing->drawRing(FPS);
    if(sphRing->isPassed())
      SphereRing::ringsPassed++;
  }
}

void drawBox()
{
  if(glIsEnabled(GL_LIGHTING))
    glCallList(lightList[6]);
  box.draw(FPS);
}

//sets fog parameters, is called in myinit()
void drawFog()
{
  GLuint	fogMode  [ ] = { GL_EXP, GL_EXP2, GL_LINEAR };	// Storage For Three Types Of Fog
  GLfloat	fogColor [4] = {0.8519f, 0.8588f, 0.8882f, 1}; 

  glFogi(GL_FOG_MODE, fogMode[2]);		// Fog Mode
  glFogfv(GL_FOG_COLOR, fogColor);		// Set Fog Color
  glFogf(GL_FOG_DENSITY, 0.01f);			// How Dense Will The Fog Be
  glHint(GL_FOG_HINT, GL_NICEST);			// Fog Hint Value
  glFogf(GL_FOG_START, 0);				// Fog Start
  glFogf(GL_FOG_END  , skyRadius);		// Fog End
}

//sets the material properties for lighting
void setLight(light_t light)
{
  glMaterialfv( GL_FRONT/*_AND_BACK*/, GL_AMBIENT, light.ambient );
  glMaterialfv( GL_FRONT/*_AND_BACK*/, GL_DIFFUSE, light.diffuse );
  glMaterialfv( GL_FRONT/*_AND_BACK*/, GL_SPECULAR, light.specular );
  glMaterialfv( GL_FRONT/*_AND_BACK*/, GL_SPECULAR, light.emission );
  glMaterialf ( GL_FRONT/*_AND_BACK*/, GL_SHININESS, light.shininess );
}

void drawLight()
{
  //Position is set in camera function

  //light settings
  GLfloat LightAmbient[] = {0.5f, 0.5f, 0.5f, 0.5f}; 
  GLfloat LightDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
  GLfloat globalAmbient[] = {0.6f, 0.6f, 0.6f, 1.0f};

  glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);	
  glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);	

  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);	
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

  glEnable(GL_LIGHT0);

  //disabling this seemed to make the light look better to me, personally
  //	glColorMaterial(GL_FRONT_AND_BACK,GL_DIFFUSE);
  //	glEnable(GL_COLOR_MATERIAL);

  //I moved this to the camera function to have the light position stay the same
  //regardless of camera position, etc.
  //	glLightfv(GL_LIGHT0, GL_POSITION,LightPosition);
}

void draw2d()
{
  GLboolean light_enabled = glIsEnabled(GL_LIGHTING);

  //sets up the projection for drawing text on the screen
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();  
  glOrtho(0, width, height, 0, 0, 1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glDisable(GL_LIGHTING);							//disable light for drawing text
  glLineWidth(1.0);
  //reset the matrices
  glBegin(GL_LINES);
  glVertex3f(width/2 + 9, (float)height/2, 0.0f);
  glVertex3f(width/2 - 8, (float)height/2, 0.0f);
  glVertex3f((float)width/2, height/2 + 8, 0.0f);
  glVertex3f((float)width/2, height/2 - 9, 0.0f);
  glEnd();

  player.jetPack.drawBar();
  player.healthBar.drawBar();

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  if(light_enabled)
    glEnable(GL_LIGHTING);
}

//updates the camera position
void camera()
{
  //updates the light position, making it stationary relative to the terrain
  GLfloat LightPosition[]= {map->height*XLEN, 5*(maxHeight - minHeight), map->width*ZLEN, 1.0f};
  glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);

  player.updatePos();
}

float setHeight()
{
  int x, z;
  float d1;

  x = floor(player.getX()/XLEN);
  z = floor(player.getZ()/ZLEN);

  if(x == 127)
    x--;

  //left triangle
  if((z + ZLEN - player.getZ()/ZLEN)*(float)XLEN/ZLEN > (x + XLEN - player.getX()/XLEN))
  {
    d1 = (map->grayValues[x+1][z+1] - map->grayValues[x+1][z])/ZLEN * (player.getZ()/ZLEN - z)
      +(map->grayValues[x][z] - map->grayValues[x+1][z])/XLEN * (XLEN - (player.getX()/XLEN - x))
      + map->grayValues[x+1][z];
  }
  //right triangle
  else
  {
    d1 = (map->grayValues[x][z] - map->grayValues[x][z+1])/ZLEN * (ZLEN - (player.getZ()/ZLEN - z))
      +(map->grayValues[x+1][z+1] - map->grayValues[x][z+1])/XLEN * (player.getX()/XLEN - x)
      + map->grayValues[x][z+1];
  }

  d1 = d1 + player.getHeight();//6 - crouchFactor;

  return d1;
}

void jump(float FPS, float terrainHeight, int ringsPassed)
{
  jumpFactor += 1/FPS * yVel;
  yVel += 1/FPS * g;

  if (jumpFactor <= terrainHeight - jumpHeight)
  {
    if(yVel < -100)
    {
      player.healthBar.energyDown(FPS, (-yVel - 100)/500);
    }

    if(player.healthBar.getLength() == 0)
    {
      for(sphRing = firstRing->next; sphRing != lastRing; sphRing = sphRing->next)
      {
        sphRing->setList(4);
      }
      player.healthBar.energyUp(FPS, player.healthBar.getMaxLength());
      SphereRing::ringsPassed = 0;
    }
    if(SDL_GetMouseState(NULL, NULL)&SDL_BUTTON(1))
    {
      if(player.jetPack.getLength() != 0)
        yVel = 10;
      else
      {
        yVel = 0;
        jumped = false;
      }
    }
    else
    {
      jumped = false;
      jumpFactor = terrainHeight - jumpHeight;
    }

    if(SphereRing::ringsPassed != SphereRing::numOfRings)
    {
      if(SphereRing::ringsPassed > 0)
        failureSound = true;
      for(sphRing = firstRing->next; sphRing != lastRing; sphRing = sphRing->next)
      {
        sphRing->setList(4);
      }
      SphereRing::ringsPassed = 0;

      box.setExistence(0);
    }
    else
      box.setExistence(1);
  }
}

void display(SDL_Window *window)
{
  GLboolean light_enabled = glIsEnabled(GL_LIGHTING);
  GLboolean fog_enabled = glIsEnabled(GL_FOG);


  // clear screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if(skyExists)		//draws the sky
    drawSkySphere();

  //terrain drawing
  if(textureExists)
    glBindTexture(GL_TEXTURE_2D, texture[0]);
  if(light_enabled)
    glCallList(lightList[0]);
  drawTerrain();

  glEnable(GL_BLEND);									//enable blending
  glDepthMask(GL_FALSE);								//enable read-only depth buffer

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	//set the blend function for drawing the water
  if(waterExists)
  {
    if(glIsEnabled(GL_LIGHTING))
      glCallList(lightList[1]);
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    drawWater();
  }

  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);		//set the blend function for drawing the sun and billboards
  if(light_enabled)
    glCallList(lightList[3]);

  glDisable(GL_FOG);		//I don't want fog when drawing billboards and suns, as it throws off the blending
  drawBillboards();
  if(whichSun)										//draws sun(s) if any sun texture was successfully loaded
    drawSuns();

  glDepthMask(GL_TRUE);								//set back to normal depth buffer mode (writable)
  glDisable(GL_BLEND);								//disable blending

  //re-enable the light and fog if it is supposed to be enabled
  if(light_enabled) 
    glEnable(GL_LIGHTING);
  if(fog_enabled) 
    glEnable(GL_FOG);

  if(glIsEnabled(GL_LIGHTING))
    glCallList(lightList[2]);
  drawObjects();		//draw objects

  drawRings();

  if(box.isExistent())
    drawBox();

  camera();			//positions the camera correctly

  draw2d();

  glFlush();
  SDL_GL_SwapWindow(window);
}

//function to initialize the projection and some variables. 
void myinit(SDL_Window *window)
{
  int i, j;
  float pointa[3], pointb[3], pointc[3];
  obj *temp;
  SphereRing *temp2;

  //initialize the obj list
  first = (obj *)malloc(sizeof(obj));
  last = (obj *)malloc(sizeof(obj));
  first->previous = NULL;
  first->next = last;
  current = first;
  last->previous = current;
  last->next = NULL;

  //initialize the billboard list
  firstBillboard = (billboard *)malloc(sizeof(billboard));
  lastBillboard = (billboard *)malloc(sizeof(billboard));
  firstBillboard->previous = NULL;
  firstBillboard->next = lastBillboard;
  currentBillboard = firstBillboard;
  lastBillboard->previous = currentBillboard;
  lastBillboard->next = NULL;

  //initialize the billboard list
  firstRing = (SphereRing *)malloc(sizeof(SphereRing));
  lastRing = (SphereRing *)malloc(sizeof(SphereRing));
  firstRing->previous = NULL;
  firstRing->next = lastRing;
  currentRing = firstRing;
  lastRing->previous = currentRing;
  lastRing->next = NULL;

  read_environment(environmentFile); //read the environment file

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);		// clear to black
  glViewport(0, 0, width, height);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);


  skyRadius = sqrt(pow(map->height * XLEN, 2.0) + pow(map->width * ZLEN, 2.0)
    + pow(3*(maxHeight - minHeight), 2.0f));


  //initialize the projection
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(54, (float)width/height, 1, 2*skyRadius);
  glMatrixMode(GL_MODELVIEW);

  glShadeModel(GL_SMOOTH);					    //use smooth shading
  glEnable(GL_DEPTH_TEST);					    //hidden surface removal
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_CULL_FACE);							//do not calculate inside of polygons
  glEnable(GL_TEXTURE_2D);						//enable 2D texturing

  //transforms grayValues into actual height values
  for (i = 0; i < map->height; i++)
    for (j = 0; j < map->width; j++)
      map->grayValues[i][j] = (maxHeight - minHeight)*map->grayValues[i][j]/maxval
      + minHeight;

  //creates a 3D array to store normal vectors
  normals = (float ***)malloc((map->height)*sizeof(float**));
  for (i = 0; i < map->height; i++)
  {
    normals[i] = (float **)malloc((map->width)*sizeof(float*));
    for (j = 0; j < map->width; j++)
      normals[i][j] = (float *)malloc(3*sizeof(float));
  }

  //initialized to 0 for easy calculations when averaging
  for (i = 0; i < map->height; i++)
    for (j = 0; j < map->width; j++)
    {
      normals[i][j][0] = 0;
      normals[i][j][1] = 0;
      normals[i][j][2] = 0;
    }

    //fills the array with the normals
    for (i = 0; i < map->height - 1; i++)
    {
      for (j = 0; j < map->width - 1; j++)
      {
        //the normal is V0 X V1, where V0 is a vector represented by
        //pointb - pointa, and V1 is pointc - pointa
        pointa[0] = i * XLEN;
        pointa[1] = map->grayValues[i][j];
        pointa[2] = j * ZLEN;

        pointb[0] = (i + 1) * XLEN;
        pointb[1] = map->grayValues[i+1][j+1];
        pointb[2] = (j + 1) * ZLEN;

        pointc[0] = (i + 1) * XLEN;
        pointc[1] = map->grayValues[i+1][j];
        pointc[2] = j * ZLEN;

        getNormal(normals[i][j], pointa, pointb, pointc);
      }
    }

    //creates a 3D array to store normal average vectors for each vertex
    averageNormals = (float ***)malloc((map->height)*sizeof(float**));
    for(i = 0; i < map->height; i++)
    {
      averageNormals[i] = (float **)malloc((map->width)*sizeof(float*));
      for(j = 0; j < map->width; j++)
        averageNormals[i][j] = (float *)malloc(3*sizeof(float));
    }

    averageNormal();	//fill the array with the average normals

    //frees the memory allocated for float ***normals
    for(i = 0; i < map->height; i++)
    {
      for(j = 0; j < map->width; j++)
        free(normals[i][j]);
      free(normals[i]);
    }
    free(normals);

    //set material settings for the land and water
    landlight.ambient[0] = 0.2f;
    landlight.ambient[1] = 0.2f;
    landlight.ambient[2] = 0.2f;
    landlight.ambient[3] = 1.0f;
    landlight.diffuse[0] = 0.0f;
    landlight.diffuse[1] = 0.5f;
    landlight.diffuse[2] = 0.0f;
    landlight.diffuse[3] = 1.0f;
    landlight.specular[0] = 0.6f;
    landlight.specular[1] = 0.6f;
    landlight.specular[2] = 0.6f;
    landlight.specular[3] = 1.0f;
    landlight.emission[0] = 0.1f;
    landlight.emission[1] = 0.1f;
    landlight.emission[2] = 0.1f;
    landlight.emission[3] = 1.0f;
    landlight.shininess = 15.0f;

    waterlight.ambient[0] = 0.3f;
    waterlight.ambient[1] = 0.3f;
    waterlight.ambient[2] = 0.6f;
    waterlight.ambient[3] = 1.0f;
    waterlight.diffuse[0] = 0.0f;
    waterlight.diffuse[1] = 0.0f;
    waterlight.diffuse[2] = 1.0f;
    waterlight.diffuse[3] = 0.4f;
    waterlight.specular[0] = 0.0f;
    waterlight.specular[1] = 0.0f;
    waterlight.specular[2] = 1.0f;
    waterlight.specular[3] = 1.0f;
    waterlight.emission[0] = 0.0f;
    waterlight.emission[1] = 0.0f;
    waterlight.emission[2] = 1.0f;
    waterlight.emission[3] = 1.0f;
    waterlight.shininess = 30.0f;

    objectlight.ambient[0] = 1.0f;
    objectlight.ambient[1] = 0.75f;
    objectlight.ambient[2] = 0.5f;
    objectlight.ambient[3] = 1.0f;
    objectlight.diffuse[0] = 1.0f;
    objectlight.diffuse[1] = 0.75f;
    objectlight.diffuse[2] = 0.5f;
    objectlight.diffuse[3] = 1.0f;
    objectlight.specular[0] = 1.0f;
    objectlight.specular[1] = 1.0f;
    objectlight.specular[2] = 1.0f;
    objectlight.specular[3] = 1.0f;
    objectlight.emission[0] = 1.0f;
    objectlight.emission[1] = 1.0f;
    objectlight.emission[2] = 1.0f;
    objectlight.emission[3] = 1.0f;
    objectlight.shininess = 5.0f;

    billboardlight.ambient[0] = 0.4f;
    billboardlight.ambient[1] = 1.0f;
    billboardlight.ambient[2] = 0.4f;
    billboardlight.ambient[3] = 1.0f;
    billboardlight.diffuse[0] = 0.0f;
    billboardlight.diffuse[1] = 1.0f;
    billboardlight.diffuse[2] = 0.0f;
    billboardlight.diffuse[3] = 1.0f;
    billboardlight.specular[0] = 0.0f;
    billboardlight.specular[1] = 1.0f;
    billboardlight.specular[2] = 0.0f;
    billboardlight.specular[3] = 1.0f;
    billboardlight.emission[0] = 0.0f;
    billboardlight.emission[1] = 1.0f;
    billboardlight.emission[2] = 0.0f;
    billboardlight.emission[3] = 1.0f;
    billboardlight.shininess = 30.0f;

    ringlighta.ambient[0] = 0.784f;
    ringlighta.ambient[1] = 0.098f;
    ringlighta.ambient[2] = 0.0f;
    ringlighta.ambient[3] = 1.0f;
    ringlighta.diffuse[0] = 0.784f;
    ringlighta.diffuse[1] = 0.098f;
    ringlighta.diffuse[2] = 0.0f;
    ringlighta.diffuse[3] = 1.0f;
    ringlighta.specular[0] = 1.0f;
    ringlighta.specular[1] = 1.0f;
    ringlighta.specular[2] = 1.0f;
    ringlighta.specular[3] = 1.0f;
    ringlighta.emission[0] = 1.0f;
    ringlighta.emission[1] = 1.0f;
    ringlighta.emission[2] = 1.0f;
    ringlighta.emission[3] = 1.0f;
    ringlighta.shininess = 5.0f;

    ringlightb.ambient[0] = 0.0f;
    ringlightb.ambient[1] = 0.784f;
    ringlightb.ambient[2] = 0.0f;
    ringlightb.ambient[3] = 1.0f;
    ringlightb.diffuse[0] = 0.0f;
    ringlightb.diffuse[1] = 0.784f;
    ringlightb.diffuse[2] = 0.0f;
    ringlightb.diffuse[3] = 1.0f;
    ringlightb.specular[0] = 1.0f;
    ringlightb.specular[1] = 1.0f;
    ringlightb.specular[2] = 1.0f;
    ringlightb.specular[3] = 1.0f;
    ringlightb.emission[0] = 1.0f;
    ringlightb.emission[1] = 1.0f;
    ringlightb.emission[2] = 1.0f;
    ringlightb.emission[3] = 1.0f;
    ringlightb.shininess = 5.0f;

    boxlight.ambient[0] = 1.0f;
    boxlight.ambient[1] = 0.75f;
    boxlight.ambient[2] = 0.5f;
    boxlight.ambient[3] = 1.0f;
    boxlight.diffuse[0] = 1.0f;
    boxlight.diffuse[1] = 0.75f;
    boxlight.diffuse[2] = 0.5f;
    boxlight.diffuse[3] = 1.0f;
    boxlight.specular[0] = 1.0f;
    boxlight.specular[1] = 1.0f;
    boxlight.specular[2] = 1.0f;
    boxlight.specular[3] = 1.0f;
    boxlight.emission[0] = 1.0f;
    boxlight.emission[1] = 1.0f;
    boxlight.emission[2] = 1.0f;
    boxlight.emission[3] = 1.0f;
    boxlight.shininess = 5.0f;

    srand(time(NULL));	//seeds the random function

    waterHeightMax = waterHeight + waterOsc;	//set up water heights
    waterHeightMin = waterHeight - waterOsc;

    oldY = height/2;

    //determines what angle to look at initially
    terrainAngle = atan((float)(map->height - 1)*XLEN/((map->width - 1)*ZLEN)); 

    drawFog();			//initialize fog
    drawLight();		//initialize water

    //creates a call list for the terrain
    terrainList = glGenLists(1);
    glNewList(terrainList, GL_COMPILE);
    drawTerrainList();
    glEndList();

    //creates a call list for the water
    waterList = glGenLists(1);
    glNewList(waterList, GL_COMPILE);
    drawWaterList();
    glEndList();

    //creates call lists for each frame of each object
    temp = (obj *)malloc(sizeof(obj));
    objectList = (GLuint *)malloc(numOfObjects*sizeof(GLuint));
    for(temp = first->next, i = 0; temp != last; temp = temp->next, i++)
    {
      objectList[i] = glGenLists(temp->fileCount);
      for(j = 0; j < temp->fileCount; j++)
      {
        glNewList(objectList[i] + j, GL_COMPILE);
        drawObjectList(temp, j);
        glEndList();
      }
    }
    free(temp);

    //setting call lists for changing material properties
    lightList[0] = glGenLists(1);
    glNewList(lightList[0], GL_COMPILE);
    setLight(landlight);
    glEndList();

    lightList[1] = glGenLists(1);
    glNewList(lightList[1], GL_COMPILE);
    setLight(waterlight);
    glEndList();

    lightList[2] = glGenLists(1);
    glNewList(lightList[2], GL_COMPILE);
    setLight(objectlight);
    glEndList();

    lightList[3] = glGenLists(1);
    glNewList(lightList[3], GL_COMPILE);
    setLight(billboardlight);
    glEndList();

    lightList[4] = glGenLists(1);
    glNewList(lightList[4], GL_COMPILE);
    setLight(ringlighta);
    glEndList();

    lightList[5] = glGenLists(1);
    glNewList(lightList[5], GL_COMPILE);
    setLight(ringlightb);
    glEndList();

    lightList[6] = glGenLists(1);
    glNewList(lightList[6], GL_COMPILE);
    setLight(boxlight);
    glEndList();

    glEnable(GL_LIGHTING);
    SDL_WarpMouseInWindow(window, width/2, height/2);
    SDL_ShowCursor (SDL_DISABLE);

    player.jetPack.setPos(5, 5);
    player.healthBar.setPos(5, 5 + player.healthBar.getMaxHeight());
    //player.healthBar.setPos(5, 6 + height/40);

    temp2 = (SphereRing *)malloc(sizeof(SphereRing));
    for(temp2 = firstRing->next; temp2 != lastRing; temp2 = temp2->next)
      temp2->drawList();

    free(temp2);

    bb = (billboard *)malloc(sizeof(billboard));
    ob = (obj *)malloc(sizeof(obj));
    sphRing = (SphereRing *)malloc(sizeof(SphereRing));
}

int main(int argc, char **argv)
{
  Uint32 dt = 0, dtPrev = 0, index = 0, heightIndex = 0;
  int avgFrame[20] = {10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10};
  float elev[50];
  float heightTemp;
  float heightPrev;

  RECT windowRect;
  GetWindowRect(GetDesktopWindow(), &windowRect);
  width = windowRect.right - windowRect.left;
  height = windowRect.bottom - windowRect.top;
  printf("%d %d\n", width, height);

  int audio_rate = 22050;			//Frequency of audio playback
  Uint16 audio_format = MIX_DEFAULT_FORMAT; 	//Format of the audio we're playing
  int audio_channels = 2;			//2 channels = stereo
  int audio_buffers = 4096;		//Size of the audio buffers in memory

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    printf("Unable to initialize SDL: %s\n", SDL_GetError());
    return 1;
  }

  //Initialize SDL_mixer with our chosen audio settings
  if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) != 0) 
  {
    printf("Unable to initialize audio: %s\n", Mix_GetError());
    exit(1);
  }

  //Load our WAV file from disk
  Mix_Chunk *sound = Mix_LoadWAV("sound.wav");
  Mix_Music *music = Mix_LoadMUS("Rocketbelt.mp3");
  Mix_Chunk *jetpack = Mix_LoadWAV("Blowtorch.wav");
  Mix_Chunk *ding = Mix_LoadWAV("Ting.wav");

  if(sound == NULL)
    printf("Unable to load WAV file: %s\n", Mix_GetError());

  if(music == NULL)
    printf("Unable to load OGG file: %s\n", Mix_GetError());

  SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
  SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
  SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
  SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

  atexit(SDL_Quit);

  SDL_Window *window = SDL_CreateWindow("Jetpack", 0, 0, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN);
  if (window == NULL) 
  {
    printf("Unable to create Window: %s\n", SDL_GetError());
    return -1;
  }
  SDL_GLContext context = SDL_GL_CreateContext(window);
  if(context == 0)
  {
    printf("Unable to create OpenGL context: %s\n", SDL_GetError());
    return -1;
  }

  myinit(window);

  heightTemp = map->grayValues[0][0] * 50;

  for(int i = 0; i < 50; i++)
    elev[i] = map->grayValues[0][0];
  
  Mix_PlayMusic(music, -1);

  //Keep looping until the user closes the SDL window
  bool running = true;
  while(running) 
  {
    display(window);
    dtPrev = dt;
    dt = SDL_GetTicks();
    avgFrame[index] = dt - dtPrev;
    FPS = 0;
    index = (index + 1) % 20;
    for(int i = 0; i < 20; i++)
      FPS += avgFrame[i];
    FPS /= 20;
    FPS = 1000/FPS;

    //printf("FPS = %f\n", FPS);
    //Get the next event from the stack
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
      //What kind of event has occurred?
      switch(event.type){
      case SDL_KEYDOWN:	//A key has been pressed
        if(event.key.keysym.sym == SDLK_l)
        {
          if(glIsEnabled(GL_LIGHTING))
            glDisable(GL_LIGHTING);
          else
            glEnable(GL_LIGHTING);
        }
        if(event.key.keysym.sym == SDLK_f)
        {
          if(glIsEnabled(GL_FOG))
            glDisable(GL_FOG);
          else
            glEnable(GL_FOG);
        }
        if(event.key.keysym.sym == SDLK_q)
          running = false;
        if(event.key.keysym.sym == SDLK_SPACE)
          if(!jumped)
          {
            jumpHeight = player.getY();
            yVel = 50;
            jumped = true;
          }
          break;			
      case SDL_KEYUP:		//A key has been released
        break;
      case SDL_MOUSEMOTION:
        if((dy - event.motion.yrel)*fabs(mouseYsens) > 90)
          dy = 90/fabs(mouseYsens);
        else if((dy - event.motion.yrel)*fabs(mouseYsens) < -90)
          dy = -90/fabs(mouseYsens);
        else
          dy -= event.motion.yrel;					

        dx -= event.motion.xrel;
        if(event.motion.x < 15)
        {
          SDL_WarpMouseInWindow(window, width - 15, event.motion.y);
          dx += width - 30;
        }
        if(event.motion.x > width - 15)
        {
          SDL_WarpMouseInWindow(window, 15, event.motion.y);
          dx -= width - 30;
        }
        if(event.motion.y < 15)
        {
          SDL_WarpMouseInWindow(window, event.motion.x, height - 15);
          dy += height - 30;
        }
        if(event.motion.y > height - 15)
        {
          SDL_WarpMouseInWindow(window, event.motion.x, 15);
          dy -= height - 30;
        }					
        break;
      case SDL_QUIT:		//The user has closed the SDL window
        running = false;
        break;
      }
    }
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    if(keys[SDL_SCANCODE_W])
      player.moveForward();
    if(keys[SDL_SCANCODE_S])
      player.moveBackward();
    if(keys[SDL_SCANCODE_A])
      player.strafeLeft();
    if(keys[SDL_SCANCODE_D])
      player.strafeRight();
    if(keys[SDL_SCANCODE_LCTRL])
    {
      if(!jumped)
      {
        player.crouch();
        //crouchFactor = 2;
        speed = .4f;
      }
    }
    else
    {
      player.uncrouch();
      //crouchFactor = 0;
      speed = 1;
    }

    if(player.getX() < 0)
      player.setX(0);
    if(player.getX() > XLEN*(map->height - 1))
      player.setX(XLEN*(map->height - 1));
    if(player.getZ() < 0)
      player.setZ(0);
    if(player.getZ() > ZLEN*(map->width - 1))
      player.setZ(ZLEN*(map->width - 1));

    if(player.jetPack.getLength() == 0)
    {
      Mix_HaltChannel(1);
      jetPackSound = false;
    }

    if(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT))
    {
      player.jetPack.energyDown(FPS, 0);
      if(jetPackSound)
      {
        Mix_PlayChannel(1, jetpack, -1);
        jetPackSound = false;
      }
    }
    else
    {
      player.jetPack.energyUp(FPS, 0);
      Mix_HaltChannel(1);
      jetPackSound = true;
    }

    heightPrev = heightTemp/50;

    heightIndex = (heightIndex + 1) % 50;
    elev[heightIndex] = setHeight();
    heightTemp = 0;
    for(int i = 0; i < 50; i++)
      heightTemp += elev[i];

    if(!jumped)
    {
      jumpFactor = 0;
      player.setY(heightTemp/50);
    }
    else
    {
      jump(FPS, heightTemp/50, SphereRing::ringsPassed);
      player.setY(jumpHeight + jumpFactor);
    }

    if(player.getY() < heightTemp/50)
      player.setY(heightTemp/50);

    if(failureSound)
    {
      Mix_PlayChannel(0, sound, 0);
      failureSound = false;
    }

    if(passedSound)
    {
      Mix_PlayChannel(2, ding, 0);
      passedSound = false;
    }
  }

  Mix_CloseAudio();
  SDL_Quit();
  return 0;
}