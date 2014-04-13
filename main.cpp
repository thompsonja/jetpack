//Author        : thompsonja
//Original Date : 8/07/06

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

#ifdef _WIN32
#include <Windows.h>
#include <direct.h>
#else
#include <unistd.h>
#endif
#include "SDL.h"
#include "FreeImage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glut.h>
#include <time.h>
#include <math.h>
#include "User.h"
#include "SphereRing.h"
#include "Box.h"
#include "Model.h"
#include "Billboard.h"
#include "Image.h"
#include "Light.h"
#include "Renderer.h"
#include "Environment.h"
#include "Audio.h"
#include <vector>
#include <map>
#include <memory>

bool stretchTerrainTexture = false;
int width=1280, height=960;			//initial window values
float FPS = 100;

bool failureSound = false;
bool passedSound = false;

//Call lists
GLuint terrainList;
GLuint waterList;
GLuint *objectList;
GLuint landLightList;
GLuint waterLightList;
GLuint objectLightList;
GLuint billboardLightList;
GLuint ringLightingPassedList;
GLuint ringLightingNotPassedList;
GLuint boxLightList;

//mouse variables
float mouseXsens = -.1f; //mouse sensitivities: increase to increase
float mouseYsens = -.1f; //sensitivity, negative values invert look
float mouseX = 0;        //stores x value when mouse is clicked
float mouseY = 0;        //stores y value when mouse is clicked
float oldMouseX = 0;     //stores change in x when mouse is unclicked
float oldMouseY = 0;     //stores change in y when mouse is unclicked
int oldY;
float dx = width/2;					//change in mouse x coordinate, used as an amount
//to rotate around y axis (up) to pan the screen
float dy = height/2;				//change in mouse y coordinate, used as an amount
//to rotate around x axis to look higher up or down

//variables affecting how fast things are animated (alter these according to system speed)
int animationSpeed = 5; //affects animation speed for objects, etc. (lower = faster)
int skySpeed = 50;
int waterSpeed = 100; //how fast water oscillates

float jumpFactor = 0;
float jumpHeight; //height in which the user jumps
float g = -90;
float yVel = 0;

//water variables
float waterHeightMax;
float waterHeightMin;				//min and max water heights, aka +/- waterOsc
int waterSign = 1;

//other variables
float terrainAngle;	//represents the initial angle to look at, for a square map this should be pi/4 radians

float*** averageNormals;			//holds average normal vector for each vertex

float viewer[3] = {0, 50, 0};		//position of camera

char *environmentFile = "environment.txt"; //the environment file to be read in

Light landlight, waterlight, objectlight, billboardlight, ringlighta, ringlightb, boxlight;

User player;
Box box(8, 3, 8, Point3D(10, 10, 10));

std::shared_ptr<Renderer> renderer;
Environment environment;

// This is a temporary function performing box/player logic
void TempUpdatePlayerBox(double dt)
{
  float x = (float)box.GetPosition().GetX();
  float y = (float)box.GetPosition().GetY();
  float z = (float)box.GetPosition().GetZ();

  float xlen = box.GetXlen();
  float ylen = box.GetYlen();
  float zlen = box.GetZlen();

  if(fabs(player.GetPosition().GetY() - (y + ylen/2)) < player.GetHeight())
  {
    if(fabs(player.GetZ() - z) < zlen/2 && fabs(player.GetX() - x) < xlen/2)
    {
      if(yVel <= 0)
      {
        player.SetJumping(false);
        player.SetOnBox(true);
        player.SetY(y + ylen/2 + player.GetHeight());
        yVel = 0;
        if(box.GetSide() == 0)
          player.SetX(player.GetX() + 30*dt);
        if(box.GetSide() == 1)
          player.SetZ(player.GetZ() + 30*dt);
        if(box.GetSide() == 2)
          player.SetX(player.GetX() - 30*dt);
        if(box.GetSide() == 3)
          player.SetZ(player.GetZ() - 30*dt);
      }
    }
    else if(player.IsOnBox())
    {
      player.SetJumping(true);
      player.SetY(y + ylen/2 + player.GetHeight());
      player.SetOnBox(false);
      jumpHeight = y + ylen/2 + player.GetHeight();
    }
    else
    {
      if(fabs(player.GetZ() - z) < zlen/2)
      {
        if((player.GetX() - x > - xlen/2 - player.GetRad()) && (player.GetX() - x < 0))
        {
          player.SetX(x - xlen/2 - player.GetRad());
        }
        else if(player.GetX() - x < xlen/2 + player.GetRad())
        {
          if(player.GetX() - x > 0)
            player.SetX(x + xlen/2 + player.GetRad());
        }
      }
      if(fabs(player.GetX() - x) < xlen/2)
      {
        if((player.GetZ() - z > - zlen/2 - player.GetRad()) && (player.GetZ() - z < 0))
        {
          player.SetZ(z - zlen/2 - player.GetRad());
        }
        else if(player.GetZ() - z < zlen/2 + player.GetRad())
        {
          if(player.GetZ() - z > 0)
            player.SetZ(z + zlen/2 + player.GetRad());
        }
      }
    }
  }
}

void quit()
{
  delete objectList;
  for(int i = 0; i < environment.map->height; i++)
  {
    for(int j = 0; j < environment.map->width; j++)
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
void averageNormal(float ***normals)
{
  //averages the normals for each vertex
  for(int i = 0; i < environment.map->height; i++)
  {
    for(int j = 0; j < environment.map->width; j++)
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
      float magnitude = sqrt(pow(averageNormals[i][j][0],2) + 
        pow(averageNormals[i][j][1],2) + pow(averageNormals[i][j][2],2));
      for (int k = 0; k < 3; k++)
        averageNormals[i][j][k] /= magnitude;
    }
  }
}

void drawObjects()
{
  glDisable(GL_TEXTURE_2D); //I was told to disable textures for objects, even though they have texture coordinates

  //draws each object
  for(unsigned int i = 0; i < environment.models.size(); i++)
  {
    const Point3D &currentPosition = environment.models[i]->GetPosition();
    glPushMatrix();
    glTranslatef((currentPosition.GetX() - 1)*environment.XLEN, 
      environment.map->grayValues[(int)currentPosition.GetX()-1][(int)currentPosition.GetZ()-1] - (currentPosition.GetY() - .001)*environment.scaleFactor, (currentPosition.GetZ() - 1)*environment.ZLEN);
    glScalef(environment.scaleFactor, environment.scaleFactor, environment.scaleFactor);

    glCallList(objectList[i] + environment.models[i]->GetCurrentFrame()/animationSpeed);

    environment.models[i]->IncrementFrame(animationSpeed);
    glPopMatrix();
  }
}

void drawObjectList(int modelNum, int frame)
{
  auto mod = environment.models[modelNum];

  //draws the triangles for every line in the .mod file
  glBegin(GL_TRIANGLES);
  for(int i = 0; i < mod->GetLinesAtFrame(frame); i++)
  {
    const Point3D &normal = mod->GetNormal(frame, i);
    const Point2D &texture = mod->GetTexture(frame, i);
    const Point3D &vertex = mod->GetVertex(frame, i);
    glNormal3f(normal.GetX(), normal.GetY(), normal.GetZ());
    //Guess I don't need to call texture coordinates if there is no texture :D
    //glTexCoord2f(texture.GetX(), texture.GetY());
    glVertex3f(vertex.GetX(), vertex.GetY(), vertex.GetZ());
  }
  glEnd();
}

void drawBillboards()
{
  glEnable(GL_TEXTURE_2D);
  //draws all the billboards
  for(auto &bb : environment.billboards)
  {
    glBindTexture(GL_TEXTURE_2D, environment.billboardTextures[bb]);
    bb->Draw(1 / FPS, environment.XLEN, environment.ZLEN, environment.scaleFactor, environment.map, player.GetPosition());
  }
  glDisable(GL_TEXTURE_2D);
}

void drawTerrain()
{
  if(environment.textureExists)
    glEnable(GL_TEXTURE_2D);
  else
    glDisable(GL_TEXTURE_2D);

  glCallList(terrainList);

  glDisable(GL_TEXTURE_2D);
}

void drawTerrainVertex(int x, int z, float texCoord1, float texCoord2)
{
  glNormal3fv(averageNormals[x][z]);
  if(!environment.textureExists)
    glColor3f(0, 0, (environment.map->grayValues[x][z] - environment.minHeight)/(environment.maxHeight - environment.minHeight));
  else if(stretchTerrainTexture)
    glTexCoord2f((float)x/environment.map->height, (float)z/environment.map->width);
  else
    glTexCoord2f(texCoord1, texCoord2);
  glVertex3f(x*environment.XLEN, environment.map->grayValues[x][z], z*environment.ZLEN);
}

void drawTerrainList()
{
  //begin drawing the terrain
  glBegin(GL_QUADS);
  //for each quad, the normal vector is set, and depending on the existance of a texture,
  //either the texture coordinate or color coordinate is set, followed by the vertex itself
  for (int x = 0; x < environment.map->height - 1; x++)
  {
    for (int z = 0; z < environment.map->width - 1; z++)
    {	
      drawTerrainVertex(x,z,0,0);     // draw vertex 0
      drawTerrainVertex(x,z+1,0,1);   // draw vertex 1
      drawTerrainVertex(x+1,z+1,1,1); // draw vertex 2
      drawTerrainVertex(x+1,z,1,0);   // draw vertex 3
    }
  }
  glEnd();
}

void drawWater()
{
  //sign tells you whether or not you are above or under the water,
  //I use it to show the water level when below the water
  float sign = (player.GetY() - environment.waterHeight)/fabs((player.GetY() - environment.waterHeight));

  glColor4f(1.0f, 1.0f, 1.0f, 0.4f);	//basically sets alpha level to .4
  glNormal3f(0.0, sign, 0.0);		//normal depends on whether you are above or below water

  glEnable(GL_TEXTURE_2D);
  glDisable(GL_CULL_FACE);

  glPushMatrix();
  glTranslatef(0, environment.waterHeight, 0);
  glCallList(waterList);

  glPopMatrix();
  //calculates water oscillation
  if (environment.waterHeight > waterHeightMax || environment.waterHeight < waterHeightMin)
    waterSign *= -1;
  environment.waterHeight += waterSign*environment.waterOsc/waterSpeed;
  glEnable(GL_CULL_FACE);
  glDisable(GL_TEXTURE_2D);
}

//called in Initialize to set up the call list for water
void drawWaterList()
{
  glBegin(GL_QUADS);
  for (int x = 0; x < environment.map->height - 1; x++)
  {
    for (int z = 0; z < environment.map->width - 1; z++)
    {
      // draw vertex 0
      glTexCoord2f(0.0f, 0.0f);
      glVertex3f(x*environment.XLEN, 0, z*environment.ZLEN);
      // draw vertex 1
      glTexCoord2f(0.0f, 1.0f);
      glVertex3f(x*environment.XLEN, 0, (z+1)*environment.ZLEN);
      // draw vertex 2
      glTexCoord2f(1.0f, 1.0f);
      glVertex3f((x+1)*environment.XLEN, 0, (z+1)*environment.ZLEN);
      // draw vertex 3
      glTexCoord2f(1.0f, 0.0f);
      glVertex3f((x+1)*environment.XLEN, 0, z*environment.ZLEN);
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
  glTranslatef(environment.map->height * environment.XLEN/2, 0.5*(environment.maxHeight - environment.minHeight), environment.map->width * environment.ZLEN/2);
  glRotatef((float)environment.angle/skySpeed, 0, 1, 0);
  environment.angle += environment.skydomeAngle;
  if (environment.angle > 360*skySpeed)		//prevents the angle value from spiraling out of control!
    environment.angle -= 360*skySpeed;

  glDisable(GL_CULL_FACE);
  glDisable(GL_LIGHTING);

  glBindTexture(GL_TEXTURE_2D, environment.skyTexture); //texture array is filled before to include the sky texture

  quadratic=gluNewQuadric();			// Create A Pointer To The Quadric Object (Return 0 If No Memory) (NEW)

  if (light_enabled) gluQuadricNormals(quadratic, GLU_SMOOTH);	// Create Smooth Normals (NEW)
  else gluQuadricNormals(quadratic, GLU_NONE);

  gluQuadricTexture(quadratic, GL_TRUE);				// Create Texture Coords (NEW)

  // Draw A Sphere With given Radius and slice #s, I used
  // geometry to find the minimum radius such that all four
  // corners are within the sphere, then doubled it in case
  // the corners themselves were raised, plus I think it looks better
  gluSphere(quadratic, environment.skyRadius, 32, 32);

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

  auto map = environment.map;
  int XLEN = environment.XLEN;
  int ZLEN = environment.ZLEN;

  //if both suns exist, only switch suns 1% of the time (otherwise it flickers too much!)
  if(environment.sun1Exists && environment.sun2Exists)
  {
    if(rand()%500 + 1 > 499)
    {
      if(environment.currentSun == &environment.sunTexture1)
        environment.currentSun = &environment.sunTexture2;
      else
        environment.currentSun = &environment.sunTexture1;
    }
  }
  glBindTexture(GL_TEXTURE_2D, *environment.currentSun);

  lookAt[0] = map->height*XLEN - player.GetX();			//x component of lookAt vector
  lookAt[1] = 3*(environment.maxHeight - environment.minHeight) - player.GetY();	//y component of lookAt vector
  lookAt[2] = map->width*ZLEN - player.GetZ();			//z component of lookAt vector
  xzAngle = atan(lookAt[0]/lookAt[2])*180/M_PI;			//calculate angle on xz plane
  if (player.GetZ() > (map->width*ZLEN))					//accounts for sign issues with atan
    xzAngle += 180;

  //billboarding in the y direction
  yAngle = atan(lookAt[1]/sqrt(pow(lookAt[0], 2) + pow(lookAt[2], 2)))*180/M_PI;

  glPushMatrix();
  //moves and rotates the sun to have billboarding effect
  glTranslatef(map->height*XLEN, 3*(environment.maxHeight - environment.minHeight), map->width*ZLEN);
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

//updates the camera position
void UpdateCamera()
{
  //updates the light position, making it stationary relative to the terrain
  GLfloat LightPosition[]= {environment.map->height*environment.XLEN, 5*(environment.maxHeight - environment.minHeight), environment.map->width*environment.ZLEN, 1.0f};
  glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
}

float setHeight()
{
  auto map = environment.map;
  int ZLEN = environment.ZLEN;
  int XLEN = environment.XLEN;

  float d1 = 0;

  int x = floor(player.GetX()/environment.XLEN);
  int z = floor(player.GetZ()/environment.ZLEN);

  if(x == 127)
    x--;

  //left triangle
  if((z + ZLEN - player.GetZ()/ZLEN)*(float)XLEN/ZLEN > (x + XLEN - player.GetX()/XLEN))
  {
    d1 = (map->grayValues[x+1][z+1] - map->grayValues[x+1][z])/ZLEN * (player.GetZ()/ZLEN - z)
      +(map->grayValues[x][z] - map->grayValues[x+1][z])/XLEN * (XLEN - (player.GetX()/XLEN - x))
      + map->grayValues[x+1][z];
  }
  //right triangle
  else
  {
    d1 = (map->grayValues[x][z] - map->grayValues[x][z+1])/ZLEN * (ZLEN - (player.GetZ()/ZLEN - z))
      +(map->grayValues[x+1][z+1] - map->grayValues[x][z+1])/XLEN * (player.GetX()/XLEN - x)
      + map->grayValues[x][z+1];
  }

  return d1 + player.GetHeight();
}

void jump(float FPS, float terrainHeight)
{
  jumpFactor += 1/FPS * yVel;
  yVel += 1/FPS * g;

  if (jumpFactor <= terrainHeight - jumpHeight)
  {
    if(yVel < -100)
    {
      player.healthBar.DecreaseEnergy((-yVel-100)/500);
    }

    if(player.healthBar.IsEmpty())
    {
      for(auto &ring : environment.rings)
      {
        ring->SetPassed(false);
      }
      player.healthBar.IncreaseEnergy(1.0);
    }
    if(SDL_GetMouseState(NULL, NULL)&SDL_BUTTON(1))
    {
      if(!player.jetPack.IsEmpty())
        yVel = 10;
      else
      {
        yVel = 0;
        player.SetJumping(false);
      }
    }
    else
    {
      player.SetJumping(false);
      jumpFactor = terrainHeight - jumpHeight;
    }

    int ringsPassed = 0;
    for(auto &ring : environment.rings)
    {
      if(ring->isPassed())
      {
        ringsPassed++;
      }
    }
    bool allRingsPassed = ringsPassed == environment.rings.size();
    if(!allRingsPassed)
    {
      if(ringsPassed > 0)
      {
        failureSound = true;
      }
      for(auto &ring : environment.rings)
      {
        ring->SetPassed(false);
      }
    }
    failureSound = !allRingsPassed && ringsPassed > 0;
    box.setExistence(allRingsPassed);
  }
}

void display(SDL_Window *window)
{
  GLboolean light_enabled = glIsEnabled(GL_LIGHTING);
  GLboolean fog_enabled = glIsEnabled(GL_FOG);

  // clear screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if(environment.skyExists)		//draws the sky
    drawSkySphere();

  //terrain drawing
  if(environment.textureExists)
    glBindTexture(GL_TEXTURE_2D, environment.terrainTexture);
  if(light_enabled)
    glCallList(landLightList);
  drawTerrain();

  glEnable(GL_BLEND);									//enable blending
  glDepthMask(GL_FALSE);								//enable read-only depth buffer

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	//set the blend function for drawing the water
  if(environment.waterExists)
  {
    if(glIsEnabled(GL_LIGHTING))
      glCallList(waterLightList);
    glBindTexture(GL_TEXTURE_2D, environment.waterTexture);
    drawWater();
  }

  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);		//set the blend function for drawing the sun and billboards
  if(light_enabled)
    glCallList(billboardLightList);

  glDisable(GL_FOG);		//I don't want fog when drawing billboards and suns, as it throws off the blending
  drawBillboards();
  if(environment.currentSun != NULL) //draws sun(s) if any sun texture was successfully loaded
    drawSuns();

  glDepthMask(GL_TRUE);								//set back to normal depth buffer mode (writable)
  glDisable(GL_BLEND);								//disable blending

  //re-enable the light and fog if it is supposed to be enabled
  if(light_enabled) 
    glEnable(GL_LIGHTING);
  if(fog_enabled) 
    glEnable(GL_FOG);

  if(glIsEnabled(GL_LIGHTING))
    glCallList(objectLightList);
  drawObjects();  //draw objects

  for(auto &ring : environment.rings)
  {
    bool newlyPassed = ring->UpdatePassedStatus(player.GetPosition(), environment.XLEN, environment.ZLEN);
    if(newlyPassed) passedSound = true;
    ring->UpdateRotation(1/FPS);
  }

  renderer->Render3D(1 / FPS, player.GetPosition());

  if(box.Exists())
  {
    box.UpdatePosition(1 / FPS);
    TempUpdatePlayerBox(1 / FPS);
  }

  UpdateCamera();	//positions the camera correctly

  player.UpdatePos(dx*mouseXsens, dy*mouseYsens);

  renderer->Render2D(1 / FPS);

  glFlush();
  SDL_GL_SwapWindow(window);
}

//function to initialize the projection and some variables. 
void Initialize(SDL_Window *window)
{
  auto map = environment.map;
  int XLEN = environment.XLEN;
  int ZLEN = environment.ZLEN;

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);		// clear to black
  glViewport(0, 0, width, height);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

  environment.skyRadius = sqrt(pow(map->height * XLEN, 2.0) + pow(map->width * ZLEN, 2.0) + pow(3*(environment.maxHeight - environment.minHeight), 2.0f));

  //initialize the projection
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(54, (float)width/height, 1, 2*environment.skyRadius);
  glMatrixMode(GL_MODELVIEW);

  glShadeModel(GL_SMOOTH);					    //use smooth shading
  glEnable(GL_DEPTH_TEST);					    //hidden surface removal
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_CULL_FACE);							//do not calculate inside of polygons
  glEnable(GL_TEXTURE_2D);						//enable 2D texturing

  //transforms grayValues into actual height values
  for (int i = 0; i < map->height; i++)
  {
    for (int j = 0; j < map->width; j++)
    {
      map->grayValues[i][j] = (environment.maxHeight - environment.minHeight)*map->grayValues[i][j]/map->maxval + environment.minHeight;
    }
  }

  //creates a 3D array to store normal vectors
  float ***normals = new float**[map->height]; //like terrain, but holds normal vector for each quad
  averageNormals = new float**[map->height]; //creates a 3D array to store normal average vectors for each vertex
  for (int i = 0; i < map->height; i++)
  {
    normals[i] = new float*[map->width];
    averageNormals[i] = new float*[map->width];
    for (int j = 0; j < map->width; j++)
    {
      normals[i][j] = new float[3];
      averageNormals[i][j] = new float[3];

      normals[i][j][0] = 0;
      normals[i][j][1] = 0;
      normals[i][j][2] = 0;

      float pointa[3], pointb[3], pointc[3];

      if(i < map->height - 1 && j < map->width - 1)
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
  }

  averageNormal(normals);	//fill the array with the average normals

  //frees the memory allocated for float ***normals
  for(int i = 0; i < map->height; i++)
  {
    for(int j = 0; j < map->width; j++)
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

  waterHeightMax = environment.waterHeight + environment.waterOsc;	//set up water heights
  waterHeightMin = environment.waterHeight - environment.waterOsc;

  oldY = height/2;

  //determines what angle to look at initially
  terrainAngle = atan((float)(map->height - 1)*XLEN/((map->width - 1)*ZLEN)); 

  renderer->InitFog(environment.skyRadius);
  renderer->InitLight();

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
  objectList = new GLuint[environment.models.size()];
  for(unsigned int i = 0; i < environment.models.size(); i++)
  {
    objectList[i] = glGenLists(environment.models[i]->GetNumFrames());
    for(int j = 0; j <environment. models[i]->GetNumFrames(); j++)
    {
      glNewList(objectList[i] + j, GL_COMPILE);
      drawObjectList(i, j);
      glEndList();
    }
  }

  //setting call lists for changing material properties
  landLightList = glGenLists(1);
  glNewList(landLightList, GL_COMPILE);
  landlight.SetGlLight();
  glEndList();

  waterLightList = glGenLists(1);
  glNewList(waterLightList, GL_COMPILE);
  waterlight.SetGlLight();
  glEndList();

  objectLightList = glGenLists(1);
  glNewList(objectLightList, GL_COMPILE);
  objectlight.SetGlLight();
  glEndList();

  billboardLightList = glGenLists(1);
  glNewList(billboardLightList, GL_COMPILE);
  billboardlight.SetGlLight();
  glEndList();

  ringLightingNotPassedList = glGenLists(1);
  glNewList(ringLightingNotPassedList, GL_COMPILE);
  ringlighta.SetGlLight();
  glEndList();

  ringLightingPassedList = glGenLists(1);
  glNewList(ringLightingPassedList, GL_COMPILE);
  ringlightb.SetGlLight();
  glEndList();

  boxLightList = glGenLists(1);
  glNewList(boxLightList, GL_COMPILE);
  boxlight.SetGlLight();
  glEndList();

  glEnable(GL_LIGHTING);
  SDL_WarpMouseInWindow(window, width/2, height/2);
  SDL_ShowCursor (SDL_DISABLE);

  renderer->ringLightingNotPassedList = ringLightingNotPassedList;
  renderer->ringLightingPassedList = ringLightingPassedList;
  renderer->box = &box;

  for(auto &ring : environment.rings)
  {
    renderer->AddSphereRing(ring);
  }
}

void ProcessInput()
{
}

void UpdateWorld()
{
}

void Render()
{
}

int main(int argc, char **argv)
{
  chdir("Resources");

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

  if (SDL_Init(SDL_INIT_VIDEO) != 0) 
  {
    printf("Unable to initialize SDL: %s\n", SDL_GetError());
    return 1;
  }

  Audio audioEngine;
  audioEngine.Initialize();

  //read_environment(environmentFile); //read the environment file

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

  environment.Parse(environmentFile);

  auto map = environment.map;
  int XLEN = environment.XLEN;
  int ZLEN = environment.ZLEN;

  renderer = std::make_shared<Renderer>(width, height, XLEN, ZLEN);
  renderer->healthBar = &player.healthBar;
  renderer->jetpackBar = &player.jetPack;

  Initialize(window);

  heightTemp = map->grayValues[0][0] * 50;

  for(int i = 0; i < 50; i++)
    elev[i] = map->grayValues[0][0];

  audioEngine.PlayMusic(Audio::MUSIC_BACKGROUND, -1);

  //Keep looping until the user closes the SDL window
  bool running = true;
  while(running) 
  {
    dtPrev = dt;
    dt = SDL_GetTicks();
    avgFrame[index] = dt - dtPrev;
    FPS = 0;
    index = (index + 1) % 20;
    for(int i = 0; i < 20; i++)
      FPS += avgFrame[i];
    FPS /= 20;
    FPS = 1000/FPS;

    ProcessInput();

    UpdateWorld();

    display(window);


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
          if(!player.IsJumping())
          {
            jumpHeight = player.GetY();
            yVel = 50;
            player.SetJumping(true);
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
      player.MoveForward(1.0 / FPS, dx*fabs(mouseXsens)*M_PI/180, dy*fabs(mouseYsens)*M_PI/180);
    if(keys[SDL_SCANCODE_S])
      player.MoveBackward(1.0 / FPS, dx*fabs(mouseXsens)*M_PI/180, dy*fabs(mouseYsens)*M_PI/180);
    if(keys[SDL_SCANCODE_A])
      player.StrafeLeft(1.0 / FPS, dx*fabs(mouseXsens)*M_PI/180, dy*fabs(mouseYsens)*M_PI/180);
    if(keys[SDL_SCANCODE_D])
      player.StrafeRight(1.0 / FPS, dx*fabs(mouseXsens)*M_PI/180, dy*fabs(mouseYsens)*M_PI/180);
    if(keys[SDL_SCANCODE_LCTRL])
    {
      if(!player.IsJumping())
      {
        player.Crouch();
      }
    }
    else
    {
      player.Uncrouch();
    }

    if(player.GetX() < 0)
      player.SetX(0);
    if(player.GetX() > XLEN*(map->height - 1))
      player.SetX(XLEN*(map->height - 1));
    if(player.GetZ() < 0)
      player.SetZ(0);
    if(player.GetZ() > ZLEN*(map->width - 1))
      player.SetZ(ZLEN*(map->width - 1));

    if(player.jetPack.IsEmpty())
    {
      audioEngine.StopSound(1);
    }

    if(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT))
    {
      player.jetPack.DecreaseEnergy(0.3/FPS);
      if(!player.jetPack.IsEmpty())
      {
        yVel += 300/FPS * .48;
        if(!player.IsJumping())
        {
          player.SetJumping(true);
          jumpHeight = player.GetY();
          yVel = 10;
        }
        audioEngine.PlaySoundEffect(Audio::SFX_JETPACK, -1, 1);
      }
    }
    else
    {
      player.jetPack.IncreaseEnergy(0.3/FPS);
      Mix_HaltChannel(1);
    }

    heightPrev = heightTemp/50;

    heightIndex = (heightIndex + 1) % 50;
    elev[heightIndex] = setHeight();
    heightTemp = 0;
    for(int i = 0; i < 50; i++)
      heightTemp += elev[i];

    if(!player.IsJumping())
    {
      jumpFactor = 0;
      player.SetY(heightTemp/50);
    }
    else
    {
      jump(FPS, heightTemp/50);
      player.SetY(jumpHeight + jumpFactor);
    }

    if(player.GetY() < heightTemp/50)
      player.SetY(heightTemp/50);

    if(failureSound)
    {
      audioEngine.PlaySoundEffect(Audio::SFX_FAIL, 0, 0);
      failureSound = false;
    }

    if(passedSound)
    {
      audioEngine.PlaySoundEffect(Audio::SFX_RING_PASSED, 0, 2);
      passedSound = false;
    }
  }

  Mix_CloseAudio();
  SDL_Quit();
  return 0;
}