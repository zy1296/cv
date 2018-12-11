/************************************************************
 * rotate-cube-new.cpp (A Sample Code for Shader-Based OpenGL ---
                                 for OpenGL version 3.1 and later)
 * Originally from Ed Angel's textbook "Interactive Computer Graphics" 6th Ed
              sample code "example3.cpp" of Chapter 4.
 * Moodified by Zhen Yao to include the use of a general rotation function
   Rotate(angle, x, y, z), where the vector (x, y, z) can have length != 1.0,
   and also to include the use of the function NormalMatrix(mv) to return the
   normal matrix (mat3) of a given model-view matrix mv (mat4).

   (The functions Rotate() and NormalMatrix() are added to the file "mat-new.h"
   by Zhen Yao, where a new and correct transpose function "transpose1()" and
   other related functions such as inverse(m) for the inverse of 3x3 matrix m are
   also added; see the file "mat-new.h".)

 * Extensively modified by Zhen Yao for the program structure and user
   interactions. See the function keyboard() for the keyboard actions.
   Also extensively re-structured by Zhen Yao to create and use the new
   function drawObj() so that it is easier to draw multiple objects. Now a floor
   and a rolling sphere are drawn.
 
** Perspective view of a color cube using LookAt() and Perspective()

** Colors are assigned and then the rasterizer interpolates
   those colors across the triangles.
**************************************************************/
#include "Angel-yjc.h"
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <stdio.h>
#include <algorithm>

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;


GLuint Angel::InitShader(const char* vShaderFile, const char* fShaderFile);

GLuint program;       /* shader program object id */
GLuint sphere_buffer1;   /* vertex buffer object id for sphere */
GLuint sphere_buffer2;   /* vertex buffer object id for sphere */
GLuint sphere_buffer3;   /* vertex buffer object id for sphere */
GLuint sphere_buffer4;   /* vertex buffer object id for sphere */
GLuint flat_buffer1;   /* vertex buffer object id for flat shading */
GLuint flat_buffer2;   /* vertex buffer object id for flat shading */
GLuint smooth_buffer1;   /* vertex buffer object id for smooth shading */
GLuint smooth_buffer2;   /* vertex buffer object id for smooth shading */
GLuint shade_buffer1;   /* vertex buffer object id for shading */
GLuint shade_buffer2;   /* vertex buffer object id for shading */
GLuint floor_buffer;  /* vertex buffer object id for floor */
GLuint axis_buffer; /* vertex buffer object id for axis */

// Projection transformation parameters
GLfloat  fovy = 45.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.5, zFar = 35.0;

GLfloat angle = 0.0; // rotation angle in degrees
vec4 init_eye(7.0, 3.0, -10.0, 1.0); // initial viewer position
vec4 eye = init_eye;               // current viewer position
point4 lightpos(-14.0,12.0,-3.0,1.0); //point light source position

bool isBegin = false; // true: started
int animationFlag = 0; // 1: animation; 0: non-animation. Toggled by key 'a' or 'A'
int opcode=1;

int sphereFlag = 1;   // 1: solid cube; 0: wireframe cube. Toggled by key 'c' or 'C'
int floorFlag = 1;  // 1: solid floor; 0: wireframe floor. Toggled by key 'f' or 'F'
int triangleFlag=1;  // To test whether the data in the sphere.8 is right (8 when alwasy right)
int directionFlag=1; // 1: rolling from A to B; 2: rolling from B to C; 3: rolling from C to A.
int shadowFlag=1; // 1: shadow; 0: non-shadow
int lightingFlag=1; // 1: enable lighting; 0: disable lighting
int shadingFlag=0; //0: non-shading; 1: flat shading; 2:smoothFlag
int fogFlag = 0;   // 0: no fog; 1: linear fog; 2: exponential fog; 3: exponential square fog
int objFlag=2;// 1: objects don't need to shade; 2: objects need to shade
int lightsourceFlag=0;//0: no positional light; 1: spot light; 2:point source
int acc_R = 0; //used to avoid sudden change

mat4 M = {vec4(1,0,0,0),
    vec4(0,1,0,0),
    vec4(0,0,1,0),
    vec4(0,0,0,1)}; // identity matrix

//shading transformation matrix used to render the shadow
//This is what part A needs.
mat4 matN(lightpos[1],0,0,0,
    -lightpos[0],0,-lightpos[2],-1,
    0,0,lightpos[1],0,
    0,0,0,lightpos[1]); // identity matrix

const int sphere_NumVertices1 = 768;
const int sphere_NumVertices2 = 3072;

#if 1
point4 sphere_points1[sphere_NumVertices1];
vec3   flat_normals1[sphere_NumVertices1];
vec3   smooth_normals1[sphere_NumVertices1];
color4 sphere_colors1[sphere_NumVertices1];

point4 sphere_points2[sphere_NumVertices2];
vec3   flat_normals2[sphere_NumVertices2];
vec3   smooth_normals2[sphere_NumVertices2];
color4 sphere_colors2[sphere_NumVertices2];
#endif

//shading buffer
point4 shade_points1[sphere_NumVertices1];
color4 shade_colors1[sphere_NumVertices1];
point4 shade_points2[sphere_NumVertices2];
color4 shade_colors2[sphere_NumVertices2];

const int floor_NumVertices = 6; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
point4 floor_points[floor_NumVertices]; // positions for all vertices
color4 floor_colors[floor_NumVertices]; // colors for all vertices
vec3 floor_normals[floor_NumVertices];

const int axis_NumVertices = 6;
point4 axis_points[axis_NumVertices]; // positions for all vertices
color4 axis_colors[axis_NumVertices]; // colors for all vertices

// Model-view and projection matrices uniform location
GLuint  model_view, projection;

/*----- Shader Lighting Parameters -----*/
// In World frame.
// Needs to transform it to Eye Frame
// before sending it to the shader(s).
color4 sphere_material_ambient(0.2, 0.2,  0.2, 1.0 );
color4 sphere_material_diffuse(1.0, 0.84, 0.0, 1.0 );
color4 sphere_material_specular(1.0, 0.84, 0.0, 1.0 );

color4 ground_material_ambient(0.2, 0.2, 0.2, 1.0 );
color4 ground_material_diffuse(0.0, 1.0, 0.0, 1.0 );
color4 ground_material_specular(0.0, 0.0, 0.0, 1.0 );
color4 light_color      (1.0, 1.0, 1.0, 1.0);
//part c: distant light
color4 light_ambient1(0.0, 0.0, 0.0, 1.0);
color4 light_diffuse1(0.8, 0.8, 0.8, 1.0);
color4 light_specular1(0.2, 0.2, 0.2, 1.0);
//vec4   light_direction  (0.1, 0.0, -1.0, 0.0);

color4 light_ambient2(0.0, 0.0, 0.0, 1.0);
color4 light_diffuse2(1.0, 1.0, 1.0, 1.0);
color4 light_specular2(1.0, 1.0, 1.0, 1.0);

color4 sphere_ambient_product  = light_ambient1 * sphere_material_ambient;
color4 sphere_diffuse_product  = light_diffuse1 * sphere_material_diffuse;
color4 sphere_specular_product = light_specular1 * sphere_material_specular;
color4 ground_ambient_product  = light_ambient1 * ground_material_ambient;
color4 ground_diffuse_product  = light_diffuse1 * ground_material_diffuse;
color4 ground_specular_product = light_specular1 * ground_material_specular;

float const_att = 2.0;
float linear_att = 0.01;
float quad_att = 0.001;
float material_shininess = 125.0;
//part c: distant light
vec4 light_position = point4(0.1, 0.0, -1.0, 0.0);

//void SetUp_Lighting_Uniform_Vars(mat4 mv);
void SetUp_Sphere_Uniform_Vars();
void SetUp_Ground_Uniform_Vars();
void SetUp_Spot_Uniform_Vars(mat4 mv);
void SetUp_Point_Uniform_Vars(mat4 mv);


//----------------------------------------------------------------------------
int Index = 0; //  This must be a global variable since quad() is called
               //      multiple times and Index should then go up to 36 for
               //      the 36 vertices and colors

// readfile(): let the user select which sphere to generate(8 triangles or 128 triangles)
void readfile()
{
    int n;
    float pt1,pt2,pt3;
    int pointindex=0;
    printf("Please select one sphere to generate:\n");
    printf("1: With 256 triangles\n");
    printf("2: With 1024 triangles\n");
    scanf("%d", &opcode); //get the x value and y value and radius
    if (opcode==1)
    {
        //look for the scailing factor
        std::fstream file;
        file.open("sphere.256.txt",std::ios::in);
        file>>n;
        if (n==256)
        {
            for(int i=0;i<n;i++)
            {
                file>>triangleFlag;
                if (triangleFlag==3)
                {
                    file>>pt1>>pt2>>pt3;
                    sphere_points1[pointindex]=point4(pt1,pt2,pt3,1.0);
                    shade_points1[pointindex]=point4(pt1,pt2,pt3,1.0);
                    smooth_normals1[pointindex]=normalize(vec3(pt1,pt2,pt3));
                    pointindex++;
                    file>>pt1>>pt2>>pt3;
                    sphere_points1[pointindex]=point4(pt1,pt2,pt3,1.0);
                    shade_points1[pointindex]=point4(pt1,pt2,pt3,1.0);
                    smooth_normals1[pointindex]=normalize(vec3(pt1,pt2,pt3));
                    pointindex++;
                    file>>pt1>>pt2>>pt3;
                    sphere_points1[pointindex]=point4(pt1,pt2,pt3,1.0);
                    shade_points1[pointindex]=point4(pt1,pt2,pt3,1.0);
                    smooth_normals1[pointindex]=normalize(vec3(pt1,pt2,pt3));
                    pointindex++;
                    vec4 u = sphere_points1[pointindex-2] - sphere_points1[pointindex-3];
                    vec4 v = sphere_points1[pointindex-1] - sphere_points1[pointindex-3];
                    flat_normals1[pointindex-3] = normalize( cross(u, v) );
                    flat_normals1[pointindex-2] = normalize( cross(u, v) );
                    flat_normals1[pointindex-1] = normalize( cross(u, v) );
                }
                else
                {
                    printf("Error!");
                }
            }
        }
        else
        {
            printf("Error!");
        }
        //file close
        file.close();
    }
    else if (opcode==2)
    {
        //look for the scailing factor
        std::fstream file;
        file.open("sphere.1024.txt",std::ios::in);
        file>>n;
        if (n==1024)
        {
            for(int i=0;i<n;i++)
            {
                file>>triangleFlag;
                if (triangleFlag==3)
                {
                    file>>pt1>>pt2>>pt3;
                    sphere_points2[pointindex]=point4(pt1,pt2,pt3,1.0);
                    shade_points2[pointindex]=point4(pt1,pt2,pt3,1.0);
                    smooth_normals2[pointindex]=normalize(vec3(pt1,pt2,pt3));
                    pointindex++;
                    file>>pt1>>pt2>>pt3;
                    sphere_points2[pointindex]=point4(pt1,pt2,pt3,1.0);
                    shade_points2[pointindex]=point4(pt1,pt2,pt3,1.0);
                    smooth_normals2[pointindex]=normalize(vec3(pt1,pt2,pt3));
                    pointindex++;
                    file>>pt1>>pt2>>pt3;
                    sphere_points2[pointindex]=point4(pt1,pt2,pt3,1.0);
                    shade_points2[pointindex]=point4(pt1,pt2,pt3,1.0);
                    smooth_normals2[pointindex]=normalize(vec3(pt1,pt2,pt3));
                    pointindex++;
                    vec4 u3 = sphere_points2[pointindex-2] - sphere_points2[pointindex-3];
                    vec4 v3 = sphere_points2[pointindex-1] - sphere_points2[pointindex-2];
                    flat_normals2[pointindex-3] = normalize(cross(u3, v3));
                    flat_normals2[pointindex-2] = normalize(cross(u3, v3));
                    flat_normals2[pointindex-1] = normalize(cross(u3, v3));
                }
                else
                {
                    printf("Error!");
                }
            }
        }
        else
        {
            printf("Error!");
        }
        //file close
        file.close();
    }
    else
    {
    printf("The opcode is wrong!");
    }
}
void quad1()
{
    sphere_colors1[Index] = color4( 1.0, 0.84, 0,1.0);
    shade_colors1[Index] = color4( 0.25, 0.25, 0.25,0.65);
    Index++;
}
// quad2(): generate triangles and assign colors to it with the file of 128 spheres
void quad2()
{
    sphere_colors2[Index] = color4( 1.0, 0.84, 0,1.0);
    shade_colors2[Index] = color4( 0.25, 0.25, 0.25,0.65);
    Index++;
}
//----------------------------------------------------------------------------
// generate triangles and judge if it is 8 triangles or 128 triangles
void generatesphere()
{
    if (opcode==1)
    {
        for (int i=0;i<768;i++)
        {
            quad1();
        }
    }
    else if (opcode==2)
    {
        for (int i=0;i<3072;i++)
        {
            quad2();
        }
    }
}

// generate the floor of 2 triangles: 6 vertices and 6 colors
void floor()
{
    floor_colors[0] = color4( 0, 1, 0,1.0); floor_points[0] = point4(5.0,-0.1,8.0,1.0);
    floor_colors[1] = color4( 0, 1, 0,1.0); floor_points[1] = point4(5.0,-0.1,-4.0,1.0);
    floor_colors[2] = color4( 0, 1, 0,1.0); floor_points[2] = point4(-5.0,-0.1,-4.0,1.0);

    floor_colors[3] = color4( 0, 1, 0,1.0); floor_points[3] = point4(-5.0,-0.1,-4.0,1.0);
    floor_colors[4] = color4( 0, 1, 0,1.0); floor_points[4] = point4(-5.0,-0.1,8.0,1.0);
    floor_colors[5] = color4( 0, 1, 0,1.0); floor_points[5] = point4(5.0,-0.1,8.0,1.0);
    vec4 u1 = floor_points[0] - floor_points[1];
    vec4 v1 = floor_points[2] - floor_points[1];
    for (int init=0;init<6;init++)
    {
        floor_normals[init] = normalize(cross(u1, v1));
    }
}

void axis() {
    axis_colors[0] = color4(1.0, 0.0, 0.0,1.0); axis_points[0] = point4(0.0, 0.0, 0.0,1.0);
    axis_colors[1] = color4(1.0, 0.0, 0.0,1.0); axis_points[1] = point4(10.0, 0.0, 0.0,1.0);
    axis_colors[2] = color4(1.0, 0.0, 1.0,1.0); axis_points[2] = point4(0.0, 0.0, 0.0,1.0);
    axis_colors[3] = color4(1.0, 0.0, 1.0,1.0); axis_points[3] = point4(0.0, 10.0, 0.0,1.0);
    axis_colors[4] = color4(0.0, 0.0, 1.0,1.0); axis_points[4] = point4(0.0, 0.0, 0.0,1.0);
    axis_colors[5] = color4(0.0, 0.0, 1.0,1.0); axis_points[5] = point4(0.0, 0.0, 10.0,1.0);
}
//----------------------------------------------------------------------------
// OpenGL initialization
void init()
{
    generatesphere();
#if 1
    // Create and initialize a vertex buffer object for cube, to be used in display()
    glGenBuffers(1, &sphere_buffer1);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_NumVertices1 + sizeof(color4) * sphere_NumVertices1 + sizeof(vec3) *  sphere_NumVertices1, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * sphere_NumVertices1, sphere_points1);
    glBufferSubData(GL_ARRAY_BUFFER,sizeof(point4) * sphere_NumVertices1, sizeof(color4) *  sphere_NumVertices1,sphere_colors1);
    glBufferSubData(GL_ARRAY_BUFFER,sizeof(point4) * sphere_NumVertices1 + sizeof(color4) *  sphere_NumVertices1, sizeof(vec3) * sphere_NumVertices1, flat_normals1);
    
    //buffers used for flat shading with 2 spheres
    glGenBuffers(1, &flat_buffer1);
    glBindBuffer(GL_ARRAY_BUFFER, flat_buffer1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_NumVertices1 + sizeof(color4) * sphere_NumVertices1 + sizeof(vec3) *  sphere_NumVertices1, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * sphere_NumVertices1, sphere_points1);
    glBufferSubData(GL_ARRAY_BUFFER,sizeof(point4) * sphere_NumVertices1, sizeof(color4) *  sphere_NumVertices1,sphere_colors1);
    glBufferSubData(GL_ARRAY_BUFFER,sizeof(point4) * sphere_NumVertices1 + sizeof(color4) *  sphere_NumVertices1, sizeof(vec3) * sphere_NumVertices1, flat_normals1);
    
    glGenBuffers(1, &flat_buffer2);
    glBindBuffer(GL_ARRAY_BUFFER, flat_buffer2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_NumVertices2 + sizeof(color4) * sphere_NumVertices2 + sizeof(vec3) *  sphere_NumVertices2, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * sphere_NumVertices2, sphere_points2);
    glBufferSubData(GL_ARRAY_BUFFER,sizeof(point4) * sphere_NumVertices2,sizeof(color4) * sphere_NumVertices2,sphere_colors2);
    glBufferSubData(GL_ARRAY_BUFFER,sizeof(point4) * sphere_NumVertices2 + sizeof(color4) *  sphere_NumVertices2, sizeof(vec3) * sphere_NumVertices2, flat_normals2);

    
    //buffers used for smooth shading with 2 spheres
    glGenBuffers(1, &smooth_buffer1);
    glBindBuffer(GL_ARRAY_BUFFER, smooth_buffer1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_NumVertices1 + sizeof(color4) * sphere_NumVertices1 + sizeof(vec3) *  sphere_NumVertices1, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * sphere_NumVertices1, sphere_points1);
    glBufferSubData(GL_ARRAY_BUFFER,sizeof(point4) * sphere_NumVertices1, sizeof(color4) *  sphere_NumVertices1,sphere_colors1);
    glBufferSubData(GL_ARRAY_BUFFER,sizeof(point4) * sphere_NumVertices1 + sizeof(color4) *  sphere_NumVertices1, sizeof(vec3) * sphere_NumVertices1, smooth_normals1);
    
    glGenBuffers(1, &smooth_buffer2);
    glBindBuffer(GL_ARRAY_BUFFER, smooth_buffer2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_NumVertices2 + sizeof(color4) * sphere_NumVertices2 + sizeof(vec3) * sphere_NumVertices2, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * sphere_NumVertices2, sphere_points2);
    glBufferSubData(GL_ARRAY_BUFFER,sizeof(point4) * sphere_NumVertices2, sizeof(color4) *  sphere_NumVertices2,sphere_colors2);
    glBufferSubData(GL_ARRAY_BUFFER,sizeof(point4) * sphere_NumVertices2 + sizeof(color4) *  sphere_NumVertices2, sizeof(vec3) * sphere_NumVertices2, smooth_normals2);
    
    
    // Create and initialize a vertex buffer object for shading, to be used in display()
    glGenBuffers(1, &shade_buffer1);
    glBindBuffer(GL_ARRAY_BUFFER, shade_buffer1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point4)*sphere_NumVertices1 + sizeof(color4)*sphere_NumVertices1, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * sphere_NumVertices1, shade_points1);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_NumVertices1, sizeof(color4) * sphere_NumVertices1, shade_colors1);
    
    // Create and initialize a vertex buffer object for cube, to be used in display()
    glGenBuffers(1, &sphere_buffer2);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_NumVertices2 + sizeof(color4) * sphere_NumVertices2 + sizeof(vec3) *  sphere_NumVertices2, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * sphere_NumVertices2, sphere_points2);
    glBufferSubData(GL_ARRAY_BUFFER,sizeof(point4) * sphere_NumVertices2, sizeof(color4) *  sphere_NumVertices2,sphere_colors2);
    glBufferSubData(GL_ARRAY_BUFFER,sizeof(point4) * sphere_NumVertices2 + sizeof(color4) *  sphere_NumVertices2, sizeof(vec3) * sphere_NumVertices2, smooth_normals2);
    
    // Create and initialize a vertex buffer object for shading, to be used in display()
    glGenBuffers(1, &shade_buffer2);
    glBindBuffer(GL_ARRAY_BUFFER, shade_buffer2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point4)*sphere_NumVertices2 + sizeof(color4)*sphere_NumVertices2, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * sphere_NumVertices2, shade_points2);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_NumVertices2, sizeof(color4) * sphere_NumVertices2, shade_colors2);
#endif

    floor();
 // Create and initialize a vertex buffer object for floor, to be used in display()
    glGenBuffers(1, &floor_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point4) * floor_NumVertices + sizeof(color4) * floor_NumVertices + sizeof(vec3) * floor_NumVertices, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * floor_NumVertices, floor_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * floor_NumVertices, sizeof(color4) * floor_NumVertices, floor_colors);
    glBufferSubData(GL_ARRAY_BUFFER,sizeof(point4) * floor_NumVertices + sizeof(color4) *  floor_NumVertices, sizeof(vec3) * floor_NumVertices, floor_normals);

    axis();
    // Create and initialize a vertex buffer object for floor, to be used in display()
    glGenBuffers(1, &axis_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, axis_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point4)*axis_NumVertices + sizeof(color4)*axis_NumVertices, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * axis_NumVertices, axis_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * axis_NumVertices, sizeof(color4) * axis_NumVertices, axis_colors);
    
 // Load shaders and create a shader program (to be used in display())
    program = InitShader("vshader53.glsl", "fshader53.glsl");

    glClearColor( 0.529, 0.807, 0.92, 0.0);
    glLineWidth(2.0);
}

//----------------------------------------------------------------------
// SetUp_Lighting_Uniform_Vars(mat4 mv):
// Set up lighting parameters that are uniform variables in shader.
//
// Note: "LightPosition" in shader must be in the Eye Frame.
//       So we use parameter "mv", the model-view matrix, to transform
//       light_position to the Eye Frame.
//----------------------------------------------------------------------
void SetUp_Sphere_Uniform_Vars()
{
    glUniform4fv(glGetUniformLocation(program, "AmbientProduct"), 1, sphere_ambient_product );
    glUniform4fv(glGetUniformLocation(program, "DiffuseProduct"), 1, sphere_diffuse_product );
    glUniform4fv(glGetUniformLocation(program, "SpecularProduct"), 1, sphere_specular_product );
    glUniform4fv(glGetUniformLocation(program, "materialambient"), 1, sphere_material_ambient);
    glUniform4fv(glGetUniformLocation(program, "materialdiffuse"), 1, sphere_material_diffuse);
    glUniform4fv(glGetUniformLocation(program, "materialspecular"), 1, sphere_material_specular);
    glUniform1f (glGetUniformLocation(program, "Shininess"), material_shininess);
}



void SetUp_Ground_Uniform_Vars()
{
    glUniform4fv(glGetUniformLocation(program, "AmbientProduct"), 1, ground_ambient_product );
    glUniform4fv(glGetUniformLocation(program, "DiffuseProduct"), 1, ground_diffuse_product );
    glUniform4fv(glGetUniformLocation(program, "SpecularProduct"), 1, ground_specular_product );

    glUniform4fv(glGetUniformLocation(program, "materialambient"), 1, ground_material_ambient);
    glUniform4fv(glGetUniformLocation(program, "materialdiffuse"), 1, ground_material_diffuse);
    glUniform4fv(glGetUniformLocation(program, "materialspecular"), 1, ground_material_specular);
    glUniform1f(glGetUniformLocation(program, "Shininess"), 100.0);
}

void SetUp_Regular_Uniform_Vars(mat4 mv)
{
    glUniform4fv(glGetUniformLocation(program, "LightPosition"), 1, light_position);
    glUniform1f(glGetUniformLocation(program, "ConstAtt"), const_att);
    glUniform1f(glGetUniformLocation(program, "LinearAtt"), linear_att);
    glUniform1f(glGetUniformLocation(program, "QuadAtt"), quad_att);
    glUniform1f(glGetUniformLocation(program, "objectFlag"), objFlag);
    glUniform1f(glGetUniformLocation(program, "lightsourceFlag"), lightsourceFlag);
}

void SetUp_Point_Uniform_Vars(mat4 mv)
{
    glUniform4fv(glGetUniformLocation(program, "LightPosition"), 1, light_position);
    glUniform4fv(glGetUniformLocation(program, "LightPosition2"), 1, mv * lightpos);
    glUniform1f (glGetUniformLocation(program, "ConstAtt"), const_att);
    glUniform1f (glGetUniformLocation(program, "LinearAtt"), linear_att);
    glUniform1f (glGetUniformLocation(program, "QuadAtt"), quad_att);
    glUniform4fv(glGetUniformLocation(program, "lightambient"), 1, light_ambient2);
    glUniform4fv(glGetUniformLocation(program, "lightdiffuse"), 1, light_diffuse2);
    glUniform4fv(glGetUniformLocation(program, "lightspecular"), 1, light_specular2);
    glUniform1f (glGetUniformLocation(program, "objectFlag"), objFlag);
    glUniform1f (glGetUniformLocation(program, "lightsourceFlag"), lightsourceFlag);
}

void SetUp_Spot_Uniform_Vars(mat4 mv)
{
    point4 spot_direction = mv * vec4(-6.0, 0.0, -4.5, 1.0);
    glUniform4fv(glGetUniformLocation(program, "LightPosition"), 1, light_position);
    glUniform4fv(glGetUniformLocation(program, "LightPosition2"), 1, mv * lightpos);
    glUniform1f (glGetUniformLocation(program, "ConstAtt"), const_att);
    glUniform1f (glGetUniformLocation(program, "LinearAtt"), linear_att);
    glUniform1f (glGetUniformLocation(program, "QuadAtt"), quad_att);
    glUniform4fv(glGetUniformLocation(program, "lightambient"), 1, light_ambient2);
    glUniform4fv(glGetUniformLocation(program, "lightdiffuse"), 1, light_diffuse2);
    glUniform4fv(glGetUniformLocation(program, "lightspecular"), 1, light_specular2);
    glUniform1f (glGetUniformLocation(program, "lightsourceFlag"), lightsourceFlag);
    glUniform1f (glGetUniformLocation(program, "objectFlag"), objFlag);
    glUniform4fv(glGetUniformLocation(program, "spot_direction"), 1, spot_direction);
}

//----------------------------------------------------------------------------
// drawObj(buffer, num_vertices):
//   draw the object that is associated with the vertex buffer object "buffer"
//   and has "num_vertices" vertices.
//
void drawObj(GLuint buffer, int num_vertices)
{
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    GLuint vNormal;
    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );

    GLuint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point4) * num_vertices) );

    vNormal = glGetAttribLocation( program, "vNormal" );
    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(sizeof(point4) * num_vertices + sizeof(color4) * num_vertices));
    //glDrawArrays(GL_TRIANGLES, 0, num_vertices);
    if (buffer == axis_buffer)
        glDrawArrays(GL_LINES, 0, num_vertices);
    else
        glDrawArrays(GL_TRIANGLES, 0, num_vertices);
    
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);
    glDisableVertexAttribArray(vNormal);
}

//----------------------------------------------------------------------------
void display( void )
{
    glEnable(GL_DEPTH_TEST);
    objFlag=1;
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glUseProgram(program); // Use the shader program

    model_view = glGetUniformLocation(program, "ModelView" );
    projection = glGetUniformLocation(program, "Projection" );

/*---  Set up and pass on Projection matrix to the shader ---*/
    mat4  p = Perspective(fovy, aspect, zNear, zFar);
    glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

/*---  Set up and pass on Model-View matrix to the shader ---*/
    // eye is a global variable of vec4 set to init_eye and updated by keyboard()
    vec4 at(0.0, 0.0, 0.0, 1.0);
    vec4 up(0.0, 1.0, 0.0, 0.0);
    mat4 mv = LookAt(eye, at, up);
    mat4 mv1, mv2;
    
    //disable writing to z-buffer
    glDepthMask(GL_FALSE);

    if (shadingFlag!=0)
    {
        objFlag=2;
    }
    if (lightsourceFlag==2)
        SetUp_Point_Uniform_Vars(mv);
    else if (lightsourceFlag==1)
        SetUp_Spot_Uniform_Vars(mv);
    else if (lightsourceFlag==0)
        SetUp_Regular_Uniform_Vars(mv);
    
    SetUp_Ground_Uniform_Vars();
    
    /*----- Set Up the Model-View matrix for the cube -----*/
#if 1 // The following is to verify that Rotate() about (0,2,0) is RotateY():
    // Commenting out Rotate() and un-commenting RotateY()
    // gives the same result.
    // The set-up below gives a new scene (scene 2), using Correct LookAt().
    
    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
    if (floorFlag == 1) // Filled floor
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else              // Wireframe floor
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    if (shadingFlag!=0)
    {
        mat3 normal_matrix1 = NormalMatrix(mv, 1);
        glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"), 1, GL_TRUE, normal_matrix1);
        drawObj(floor_buffer, floor_NumVertices);  // draw the cube
    }
    else
    {
        glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
        drawObj(floor_buffer, floor_NumVertices);  // draw the cube
    }

    objFlag=1;
    glUniform1f(glGetUniformLocation(program, "objectFlag"), objFlag);
    drawObj(axis_buffer, axis_NumVertices);
    //enable writing to z-buffer
    glDepthMask(GL_TRUE);

    //render sphere
    if (directionFlag==1)
    {
        //To save the previous rotation when changing the direction
        if (acc_R > 0) {
            M= Rotate(acc_R, 9, 0, -1)* M;
            acc_R = 0;
        }
      mv1 = mv * Translate(3.0 - (angle*10*3.142/(9.014*360)), 1.0, 5.0 - (angle*15*3.142/(9.014*360))) * Rotate(angle, -7.5, 0.0, 5.0) * M;
    }
    else if (directionFlag==2)//The function is in the condition of rolling sphere from B to C
    {
        //To save the previous rotation when changing the direction
        if (acc_R > 0) {
            M = Rotate(acc_R, -7.5, 0, 5)* M;
            acc_R = 0;
        }
        mv1 = mv * Translate(-2.0 + (angle*8*3.142/(4.272*360)), 1.0, -2.5 - (angle*3*3.142/(4.272*360))) * Rotate(angle, -1.5, 0.0, -4.0) * M;
    }
    else if (directionFlag==3)//The function is in the condition of rolling sphere from C to A
    {
        //To save the previous rotation when changing the direction
        if (acc_R > 0) {
            M = Rotate(acc_R, -1.5, 0, -4)* M;
            acc_R = 0;
        }
        mv1 = mv * Translate(2.0 + (angle*2*3.142/(9.055*360)), 1.0, -4.0 + (angle*18*3.142/(9.055*360))) * Rotate(angle, 9.0, 0.0, -1.0) * M;
    }
#endif
    if (sphereFlag == 1) // Filled floor
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else              // Wireframe floor
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    if (opcode==1)
    {
        if (shadowFlag==1)
        {
            if (directionFlag==1)
                mv2 = LookAt(eye, at, up) * matN * Translate(3.0 - (angle*10*3.142/(9.014*360)), 1.0, 5.0 - (angle*15*3.142/(9.014*360))) * Rotate(angle, -7.5, 0.0, 5.0) * M;
            else if(directionFlag==2)
                mv2 = LookAt(eye, at, up) * matN * Translate(-2.0 + (angle*8*3.142/(4.272*360)), 1.0, -2.5 - (angle*3*3.142/(4.272*360))) * Rotate(angle, -1.5, 0.0, -4.0) * M;
            else
                mv2 = LookAt(eye, at, up) * matN * Translate(2.0 + (angle*2*3.142/(9.055*360)), 1.0, -4.0 + (angle*18*3.142/(9.055*360))) * Rotate(angle, 9.0, 0.0, -1.0) * M;
            glUniformMatrix4fv(model_view, 1, GL_TRUE, mv2); // GL_TRUE: matrix is row-major
            drawObj(shade_buffer1, sphere_NumVertices1);  // draw the shadow
        }
        /*
         objFlag=2;
        //disable writing to frame buffer
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glUniformMatrix4fv(model_view, 1, GL_TRUE, LookAt(eye, at, up)); // GL_TRUE: matrix is row-major
        if (floorFlag == 1) // Filled floor
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        else              // Wireframe floor
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        drawObj(floor_buffer, floor_NumVertices);  // draw the floor
        objFlag=1;
        glUniformMatrix4fv(model_view, 1, GL_TRUE, LookAt(eye, at, up)); // GL_TRUE: matrix is row-major
        drawObj(axis_buffer, axis_NumVertices);
        //enable writing to frame buffer
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        */
        if (sphereFlag == 1) // Filled floor
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        else              // Wireframe floor
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        
        glUniformMatrix4fv(model_view, 1, GL_TRUE, mv1); // GL_TRUE: matrix is row-major
        if (lightsourceFlag==1)
            SetUp_Spot_Uniform_Vars(mv);
        else if (lightsourceFlag==2)
            SetUp_Point_Uniform_Vars(mv);
        else if (lightsourceFlag==0)
        {
            objFlag=1;
            SetUp_Regular_Uniform_Vars(mv);
        }
        SetUp_Sphere_Uniform_Vars();
        
        //render sphere
        // Set up the Normal Matrix from the model-view matrix
        if (shadingFlag==1)
        {
            objFlag=2;
            glUniform1f (glGetUniformLocation(program, "objectFlag"), objFlag);
            glUniform1f (glGetUniformLocation(program, "lightsourceFlag"), lightsourceFlag);
            
            mat3 normal_matrix = NormalMatrix(mv1, 1);
            glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"), 1, GL_TRUE, normal_matrix);
            drawObj(flat_buffer1, sphere_NumVertices1);  // draw the cube
        }
        else if(shadingFlag==2)
        {
            objFlag=2;
            glUniform1f (glGetUniformLocation(program, "objectFlag"), objFlag);
            glUniform1f (glGetUniformLocation(program, "lightsourceFlag"), lightsourceFlag);
            
            mat3 normal_matrix2 = NormalMatrix(mv1, 1);
            glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"), 1, GL_TRUE, normal_matrix2);
            //glUniformMatrix4fv(model_view, 1, GL_TRUE, mv1); // GL_TRUE: matrix is row-major
            drawObj(smooth_buffer1, sphere_NumVertices1);  // draw the cube
        }
        else if(shadingFlag==0)
        {
            glUniformMatrix4fv(model_view, 1, GL_TRUE, mv1); // GL_TRUE: matrix is row-major
            drawObj(sphere_buffer1, sphere_NumVertices1);  // draw the cube
        }
    }
    else if (opcode==2)
    {
        if (shadowFlag==1)
        {
            if (directionFlag==1)
                mv2 = LookAt(eye, at, up) * matN * Translate(3.0 - (angle*10*3.142/(9.014*360)), 1.0, 5.0 - (angle*15*3.142/(9.014*360))) * Rotate(angle, -7.5, 0.0, 5.0) * M;
            else if(directionFlag==2)
                mv2 = LookAt(eye, at, up) * matN * Translate(-2.0 + (angle*8*3.142/(4.272*360)), 1.0, -2.5 - (angle*3*3.142/(4.272*360))) * Rotate(angle, -1.5, 0.0, -4.0) * M;
            else
                mv2 = LookAt(eye, at, up) * matN * Translate(2.0 + (angle*2*3.142/(9.055*360)), 1.0, -4.0 + (angle*18*3.142/(9.055*360))) * Rotate(angle, 9.0, 0.0, -1.0) * M;
            glUniformMatrix4fv(model_view, 1, GL_TRUE, mv2); // GL_TRUE: matrix is row-major
            drawObj(shade_buffer2, sphere_NumVertices2); //draw the shadow
        }
        /*
        objFlag=2;
        //disable writing to frame buffer
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glUniformMatrix4fv(model_view, 1, GL_TRUE, LookAt(eye, at, up)); // GL_TRUE: matrix is row-major
        if (floorFlag == 1) // Filled floor
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        else              // Wireframe floor
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        drawObj(floor_buffer, floor_NumVertices);  // draw the floor
        glUniformMatrix4fv(model_view, 1, GL_TRUE, LookAt(eye, at, up)); // GL_TRUE: matrix is row-major
        drawObj(axis_buffer, axis_NumVertices);
        //enable writing to frame buffer
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
         */
        
        if (sphereFlag == 1) // Filled floor
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        else              // Wireframe floor
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        
        if (lightsourceFlag==1)
        {
            objFlag=2;
            SetUp_Spot_Uniform_Vars(mv);
        }
        else if (lightsourceFlag==2)
        {
            objFlag=2;
            SetUp_Point_Uniform_Vars(mv);
        }
        else if (lightsourceFlag==0)
        {
            objFlag=1;
            SetUp_Regular_Uniform_Vars(mv);
        }
        SetUp_Sphere_Uniform_Vars();
        
        //render sphere
        // Set up the Normal Matrix from the model-view matrix
        if (shadingFlag==1)
        {
            objFlag=2;
            glUniform1f (glGetUniformLocation(program, "objectFlag"), objFlag);
            glUniform1f (glGetUniformLocation(program, "lightsourceFlag"), lightsourceFlag);
            
            mat3 normal_matrix = NormalMatrix(mv1, 1);
            glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"), 1, GL_TRUE, normal_matrix );
            glUniformMatrix4fv(model_view, 1, GL_TRUE, mv1); // GL_TRUE: matrix is row-major
            drawObj(flat_buffer2, sphere_NumVertices2);  // draw the cube
        }
        else if(shadingFlag==2)
        {
            objFlag=2;
            glUniform1f (glGetUniformLocation(program, "objectFlag"), objFlag);
            glUniform1f (glGetUniformLocation(program, "lightsourceFlag"), lightsourceFlag);
            
            mat3 normal_matrix = NormalMatrix(mv1, 1);
            glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"), 1, GL_TRUE, normal_matrix );
            glUniformMatrix4fv(model_view, 1, GL_TRUE, mv1); // GL_TRUE: matrix is row-major
            drawObj(smooth_buffer2, sphere_NumVertices2);  // draw the cube
        }
        else if(shadingFlag==0)
        {
            glUniformMatrix4fv(model_view, 1, GL_TRUE, mv1); // GL_TRUE: matrix is row-major
            drawObj(sphere_buffer2, sphere_NumVertices2);  // draw the cube
        }
    }
    
    if (eye[1] < 0.0)
    {
        if (shadingFlag!=0)
        {
            objFlag=2;
        }
        if (lightsourceFlag==2)
        SetUp_Point_Uniform_Vars(mv);
        else if (lightsourceFlag==1)
        SetUp_Spot_Uniform_Vars(mv);
        else if (lightsourceFlag==0)
        SetUp_Regular_Uniform_Vars(mv);
        
        SetUp_Ground_Uniform_Vars();
 
        glDepthMask(GL_FALSE);
        glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
        if (floorFlag == 1) // Filled floor
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        else              // Wireframe floor
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        
        if (shadingFlag==1)
        {
            mat3 normal_matrix = NormalMatrix(mv, 1);
            glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"), 1, GL_TRUE, normal_matrix);
            drawObj(floor_buffer, floor_NumVertices);  // draw the cube
        }
        else if(shadingFlag==2)
        {
            mat3 normal_matrix = NormalMatrix(mv, 1);
            glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"), 1, GL_TRUE, normal_matrix);
            drawObj(floor_buffer, floor_NumVertices);  // draw the cube
        }
        else if(shadingFlag==0)
        {
            glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
            drawObj(floor_buffer, floor_NumVertices);  // draw the cube
        }
        //enable writing to z-buffer
        glDepthMask(GL_TRUE);
    }
    //To judge if it needs to change the direction
    if (directionFlag==1)
    {
        if ((angle*3.142/180) > sqrt(5 * 5 + 7.5*7.5)){
            directionFlag=2;
            acc_R = angle;
            angle = 0;
        }
    }
    else if (directionFlag==2)
    {
        if ((angle*3.142/180) > sqrt(4 * 4 + 1.5*1.5)){
            directionFlag=3;
            acc_R = angle;
            angle = 0;
        }
    }
    else
    {
        if ((angle*3.142/180) > sqrt(1 * 1 + 9*9)){
            directionFlag=1;
            acc_R = angle;
            angle = 0;
        }
    }
    glutSwapBuffers();
}

//---------------------------------------------------------------------------
void idle (void)
{
    //angle += 0.02;
    angle += 0.02;    //YJC: change this value to adjust the cube rotation speed.
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void keyboard(unsigned char key, int x, int y)
{
    switch(key) {
        case 033: // Escape Key
        case 'q': case 'Q':
        exit( EXIT_SUCCESS );
        break;
        
        //press the keys to change the different axises
        case 'X': eye[0] += 1.0; break;
        case 'x': eye[0] -= 1.0; break;
        case 'Y': eye[1] += 1.0; break;
        case 'y': eye[1] -= 1.0; break;
        case 'Z': eye[2] += 1.0; break;
        case 'z': eye[2] -= 1.0; break;
        
        case 'b': case 'B': // Start the animation
        animationFlag = 1;
        glutIdleFunc(idle);
        isBegin = true;
        break;
        
        case 'c': case 'C': // Toggle between filled and wireframe sphere
        sphereFlag = 1 -  sphereFlag;
        break;
        
        case 'f': case 'F': // Toggle between filled and wireframe floor
        floorFlag = 1 -  floorFlag;
        break;
    
        case ' ':  // reset to initial viewer/eye position
        eye = init_eye;
        break;
    }
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    aspect = (GLfloat) width  / (GLfloat) height;
    glutPostRedisplay();
}

void myMouse(int button, int state, int x, int y) {
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN){
        if (isBegin == true) {
            animationFlag = 1 - animationFlag;     // Toggle between animation and non-animation
            if (animationFlag == 1)
            glutIdleFunc(idle);
            else
            glutIdleFunc(NULL);
        }
    }
    
}

//To add a menu when right clicking the mouse

void shadow(int id) {
    switch (id)
    {
        case 7:
            shadowFlag=1;
            break;
        case 8:
            shadowFlag=0;
            break;
    }
    glutPostRedisplay();
}

void enablelighting(int id) {
    switch (id)
    {
        case 9:
            lightingFlag = 1;
            break;
        case 10:
            lightingFlag = 0;
            break;
    }
    glutPostRedisplay();
}

void shading(int id) {
    switch (id)
    {
        case 11:
            
            shadingFlag = 1;
            if (lightingFlag==0)
                shadingFlag=0;
            glutIdleFunc(idle);
            break;
        case 12:
            shadingFlag = 2;
            if (lightingFlag==0)
                shadingFlag=0;
            break;
    }
    glutPostRedisplay();
}

void lightsource(int id) {
    switch (id)
    {
        case 13:
            lightsourceFlag = 1;
            break;
        case 14:
            lightsourceFlag = 2;
            break;
    }
    glutPostRedisplay();
}

void fog(int id) {
    switch (id)
    {
        case 15:
            fogFlag = 0;
            break;
        case 16:
            fogFlag = 1;
            break;
        case 17:
            fogFlag = 2;
            break;
        case 18:
            fogFlag = 3;
            break;
    }
    glutPostRedisplay();
}

void myMenu(int id) {
    switch (id)
    {
        case 1:
            eye = init_eye;
            animationFlag = 1;
            glutIdleFunc(idle);
            break;
        case 2:
            sphereFlag=0;
            break;
        case 3:
            exit(0);
            break;
    }
    glutPostRedisplay();
}

//----------------------------------------------------------------------------
int main( int argc, char **argv )
{
    readfile();
    glutInit(&argc, argv);
#ifdef __APPLE__ // Enable core profile of OpenGL 3.2 on macOS.
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
#else
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
    glutInitWindowSize(512, 512);
    glutCreateWindow("Color sphere");

#ifdef __APPLE__ // on macOS
    // Core profile requires to create a Vertex Array Object (VAO).
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
#else           // on Linux or Windows, we still need glew
    /* Call glewInit() and error checking */
    int err = glewInit();
    if (GLEW_OK != err)
    { 
        printf("Error: glewInit failed: %s\n", (char*) glewGetErrorString(err)); 
        exit(1);
    }
#endif

    // Get info of GPU and supported OpenGL version
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("OpenGL version supported %s\n", glGetString(GL_VERSION));
    
    glutDisplayFunc(display);
    //std::cout<<"acdcd"<<std::endl;
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
    
    //To add a menu when right clicking the mouse
    int sub_menu1 = glutCreateMenu(shadow);
    glutAddMenuEntry("Yes", 7);
    glutAddMenuEntry("No", 8);
    
    int sub_menu2 = glutCreateMenu(enablelighting);
    glutAddMenuEntry("Yes", 9);
    glutAddMenuEntry("No", 10);
    
    int sub_menu3 = glutCreateMenu(shading);
    glutAddMenuEntry("flat shading", 11);
    glutAddMenuEntry("smooth shading", 12);
    
    int sub_menu4 = glutCreateMenu(lightsource);
    glutAddMenuEntry("spot light", 13);
    glutAddMenuEntry("point source", 14);
    
    int sub_menu5 = glutCreateMenu(fog);
    glutAddMenuEntry("no fog", 15);
    glutAddMenuEntry("linear", 16);
    glutAddMenuEntry("exponential", 17);
    glutAddMenuEntry("exponential square", 18);
    
    glutMouseFunc(myMouse);
    glutCreateMenu(myMenu);
    glutAddMenuEntry("Default View Point", 1);
    glutAddSubMenu("Shadow", sub_menu1);
    glutAddSubMenu("Enable Lighting", sub_menu2);
    glutAddMenuEntry("Wire Frame Sphere", 2);
    glutAddSubMenu("Shading", sub_menu3);
    glutAddSubMenu("Light Source", sub_menu4);
    glutAddSubMenu("Fog Options", sub_menu5);
    glutAddMenuEntry("Quit", 3);
    glutAttachMenu (GLUT_LEFT_BUTTON) ;

    init();
    glutMainLoop();
    return 0;
}
