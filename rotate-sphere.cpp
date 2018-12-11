#include "Angel-yjc.h"
#pragma comment(lib,"glew32.lib")
#include "math.h"
#include <iostream>
#include <fstream>

using namespace std;

typedef Angel::vec3  color3;
typedef Angel::vec3  point3;
typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

#define PI 3.14159


GLuint program;         /* shader program object id */
GLuint program2;
GLuint x_buffer;        /* vertex buffer object id for x axis */
GLuint y_buffer;        /* vertex buffer object id for y axis */
GLuint z_buffer;        /* vertex buffer object id for z axis */
GLuint sphere_buffer;   /* vertex buffer object id for sphere */
GLuint floor_buffer;    /* vertex buffer object id for floor */

GLuint shadow_buffer;   /* vertex buffer object id for shadow */
GLuint smooth_buffer;
GLuint flat_buffer;
GLuint floor_lighting_buffer;    /* vertex buffer object id for lighting floor */



/* --------- Projection transformation parameters ---------*/

GLfloat     fovy = 45.0;   // Field-of-view in Y direction angle (in degrees)
GLfloat     aspect;        // Viewport aspect ratio
GLfloat     zNear = 0.5, zFar = 35.0;
GLfloat     angle = 0.0;    // rotation angle in degrees
vec4        init_eye( 7.0, 3.0, -10.0, 1.0);// initial viewer position
vec4        eye = init_eye;    // current viewer position
vec4        A(  3, 1,  5, 1);
vec4        B( -1, 1, -4, 1);
vec4        C(3.5, 1,-2.5,1);
vec4        OY( 0, 1,  0, 1);

int animationFlag = 0;      // 1: animation; 0: non-animation. Toggled by key 'a' or 'A'
int sphereFlag = 0;         // 1: solid sphere; 0: wireframe sphere. Toggled by key 's' or 'S'
int floorFlag = 1;          // 1: solid floor; 0: wireframe floor. Toggled by key 'f' or 'F'
int beginFlag = 0;          // 1: begin used; 0: begin unused

int triangle_num = 0;
float speed = 2.0;

const int sphere_NumVertices = 1500*3;
#if 0
point3 sphere_points[sphere_NumVertices]; // positions for all vertices
color3 sphere_colors[sphere_NumVertices]; // colors for all vertices
#endif
#if 1
point3 sphere_points[5000];
color3 sphere_colors[5000];
#endif

const int shadow_NumVertices = 1500*3;
#if 0
point3 shadow_points[shadow_NumVertices]; // positions for all vertices
color3 shadow_colors[shadow_NumVertices]; // colors for all vertices
#endif
#if 1
point3 shadow_points[5000];
color3 shadow_colors[5000];
#endif

point4 shade_points[5000];
vec3   flat_shade[5000];
vec3   smooth_shade[5000];



const int floor_NumVertices = 6;        //(1 face)*(2 triangles/face)*(3 vertices/triangle)
point4 floor_points[floor_NumVertices]; // positions for all vertices
color4 floor_colors[floor_NumVertices]; // colors for all vertices
vec3 floor_colors[floor_NumVertices]; // colors for all vertices

point4 floor_lighting_points[floor_NumVertices]; // positions for all vertices
vec3   floor_lighting_colors[floor_NumVertices]; // colors for all vertices

point3 floor_vertices[4] = {
    point3(  5, 0,  8, 1),
    point3(  5, 0, -4, 1),
    point3( -5, 0, -4, 1),
    point3( -5, 0,  8, 1)
};

point4 floor_lighting_vertices[4] = {
    point4(  5., 0.,  8., 1.),
    point4(  5., 0., -4., 1.),
    point4( -5., 0., -4., 1.),
    point4( -5., 0.,  8., 1.)
};


// RGBA colors
color3 vertex_colors[9] = {
    color3( 0.0, 0.0, 0.0),  // black
    color3( 1.0, 0.0, 0.0),  // red
    color3( 1.0, 1.0, 0.0),  // yellow
    color3( 0.0, 1.0, 0.0),  // green
    color3( 0.0, 0.0, 1.0),  // blue
    color3( 1.0, 0.0, 1.0),  // magenta
    color3( 1.0, 1.0, 1.0),  // white
    color3( 0.0, 1.0, 1.0),  // cyan
    color3( 1.0, 0.84,  0)   // golden yellow
    
};


const int axis_NumVertices = 3; //(3 vertices/triangle)
point3 x_points[3] = {point3(0,0,0), point3(5,0,0), point3(10,0,0)};
color3 x_colors[3] = {vertex_colors[1], vertex_colors[1], vertex_colors[1]};
point3 y_points[3] = {point3(0,0,0), point3(0,5,0), point3(0,10,0)};
color3 y_colors[3] = {vertex_colors[5], vertex_colors[5], vertex_colors[5]};
point3 z_points[3] = {point3(0,0,0), point3(0,0,5), point3(0,0,10)};
color3 z_colors[3] = {vertex_colors[4], vertex_colors[4], vertex_colors[4]};


/*------- Shader Lighting Parameters -------*/

point4 light_position   (-14.0, 12.0, -3.0, 1.0);
color3 shadow_color     (0.25, 0.25, 0.25);

color4 sphere_material_ambient  (0.2, 0.2,  0.2, 1.0 );
color4 sphere_material_diffuse  (1.0, 0.84, 0.0, 1.0 );
color4 sphere_material_specular (1.0, 0.84, 0.0, 1.0 );

color4 floor_material_ambient   (0.2, 0.2, 0.2, 1.0 );
color4 floor_material_diffuse   (0.0, 1.0, 0.0, 1.0 );
color4 floor_material_specular  (0.0, 0.0, 0.0, 1.0 );

color4 light_color      (1.0, 1.0, 1.0, 1.0);
color4 light_ambient1   (0.0, 0.0, 0.0, 1.0);
color4 light_diffuse1   (0.8, 0.8, 0.8, 1.0);
color4 light_specular1  (0.2, 0.2, 0.2, 1.0);
vec4   light_direction  (0.1, 0.0, -1.0, 0.0);

color4 light_ambient2   (0.0, 0.0, 0.0, 1.0);
color4 light_diffuse2   (1.0, 1.0, 1.0, 1.0);
color4 light_specular2  (1.0, 1.0, 1.0, 1.0);


color4 sphere_ambient_product  = light_ambient1 * sphere_material_ambient;
color4 sphere_diffuse_product  = light_diffuse1 * sphere_material_diffuse;
color4 sphere_specular_product = light_specular1 * sphere_material_specular;
color4 floor_ambient_product  = light_ambient1 * floor_material_ambient;
color4 floor_diffuse_product  = light_diffuse1 * floor_material_diffuse;
color4 floor_specular_product = light_specular1 * floor_material_specular;

float const_att         = 2.0;
float linear_att        = 0.01;
float quad_att          = 0.001;
float material_shininess= 125.0;

int shadowFlag          = 1;         // 1: enable shadow; 0: no shadow
int lightingFlag        = 0;
int flatshadingFlag     = 0;
int smoothshadingFlag   = 0;
int pointlightFlag      = 0;
int spotlightFlag       = 0;


GLuint  model_view;  // model-view matrix uniform shader variable location
GLuint  projection;  // projection matrix uniform shader variable location

GLuint  model_view2;  // model-view matrix uniform shader variable location
GLuint  projection2;  // projection matrix uniform shader variable location



void SetUp_Point_Light_Var(mat4 mv)
{
    vec4 light_position_eye = mv * light_position;
    glUniform4fv(glGetUniformLocation(program2, "LightPosition"), 1, light_position_eye);
    glUniform1f (glGetUniformLocation(program2, "ConstAtt"),      const_att);
    glUniform1f (glGetUniformLocation(program2, "LinearAtt"),     linear_att);
    glUniform1f (glGetUniformLocation(program2, "QuadAtt"),       quad_att);
    glUniform4fv(glGetUniformLocation(program2, "LightAmbient"),  1, light_ambient2);
    glUniform4fv(glGetUniformLocation(program2, "LightDiffuse"),  1, light_diffuse2);
    glUniform4fv(glGetUniformLocation(program2, "LightSpecular"), 1, light_specular2);
    glUniform1f (glGetUniformLocation(program2, "PointFlag"),    pointlightFlag);
    glUniform1f (glGetUniformLocation(program2, "SpotFlag"),     0.0);
}



void SetUp_Spot_Light_Var(mat4 mv)
{
    float spot_exponent = 15.0;
    float spot_angle    = cos(20.0 * PI / 180);
    
    vec4 light_position_eye = mv * light_position;
    point4 spot_direction = mv * vec4(-6.0, 0.0, -4.5, 1.0);

    glUniform4fv(glGetUniformLocation(program2, "LightPosition"), 1, mv*light_position);
    glUniform1f (glGetUniformLocation(program2, "ConstAtt"),     const_att);
    glUniform1f (glGetUniformLocation(program2, "LinearAtt"),    linear_att);
    glUniform1f (glGetUniformLocation(program2, "QuadAtt"),      quad_att);
    glUniform4fv(glGetUniformLocation(program2, "LightAmbient"),  1, light_ambient2);
    glUniform4fv(glGetUniformLocation(program2, "LightDiffuse"),  1, light_diffuse2);
    glUniform4fv(glGetUniformLocation(program2, "LightSpecular"), 1, light_specular2);
    glUniform1f (glGetUniformLocation(program2, "PointFlag"),    0.0);
    glUniform1f (glGetUniformLocation(program2, "SpotFlag"),     spotlightFlag);
    
    glUniform4fv(glGetUniformLocation(program2, "spot_direction"), 1, spot_direction);
    glUniform1f (glGetUniformLocation(program2, "spot_exp"),      spot_exponent);
    glUniform1f (glGetUniformLocation(program2, "spot_ang"),      spot_angle);
}



void SetUp_Sphere_Material_Var()
{
    glUniform4fv(glGetUniformLocation(program2, "AmbientProduct"),  1, sphere_ambient_product );
    glUniform4fv(glGetUniformLocation(program2, "DiffuseProduct"),  1, sphere_diffuse_product );
    glUniform4fv(glGetUniformLocation(program2, "SpecularProduct"), 1, sphere_specular_product );
    glUniform4fv(glGetUniformLocation(program2, "MaterialAmbient"), 1, sphere_material_ambient);
    glUniform4fv(glGetUniformLocation(program2, "MaterialDiffuse"), 1, sphere_material_diffuse);
    glUniform4fv(glGetUniformLocation(program2, "MaterialSpecular"), 1, sphere_material_specular);
    glUniform1f (glGetUniformLocation(program2, "Shininess"), material_shininess);
}



void SetUp_Floor_Material_Var()
{
    glUniform4fv(glGetUniformLocation(program2, "AmbientProduct"),  1, floor_ambient_product );
    glUniform4fv(glGetUniformLocation(program2, "DiffuseProduct"),  1, floor_diffuse_product );
    glUniform4fv(glGetUniformLocation(program2, "SpecularProduct"), 1, floor_specular_product );
    glUniform4fv(glGetUniformLocation(program2, "MaterialAmbient"), 1, floor_material_ambient);
    glUniform4fv(glGetUniformLocation(program2, "MaterialDiffuse"), 1, floor_material_diffuse);
    glUniform4fv(glGetUniformLocation(program2, "MaterialSpecular"), 1, floor_material_specular);
    glUniform1f (glGetUniformLocation(program2, "Shininess"), 1.0);
}



//----------------------------------------------------------------------------
int Index = 0; // YJC: This must be a global variable since quad() is called
               //      multiple times and Index should then go up to 36 for
               //      the 36 vertices and colors



//ball(): assign color to the vertices
void colorBallShadow()
{
    while(Index < triangle_num * 3)
    {
        sphere_colors[Index] = vertex_colors[8];
        shadow_colors[Index] = shadow_color;
        Index ++;
    }
}


void getFlatData()
{
    for (int i = 0; i<triangle_num; i++)
    {
        vec4 t1(sphere_points[3*i][0]  , sphere_points[3*i][1]  , sphere_points[3*i][2]  , 1.0);
        vec4 t2(sphere_points[3*i+1][0], sphere_points[3*i+1][1], sphere_points[3*i+1][2], 1.0);
        vec4 t3(sphere_points[3*i+2][0], sphere_points[3*i+2][1], sphere_points[3*i+2][2], 1.0);
    
        vec3 t_normal       = normalize(cross(t2-t1, t3-t2));
        flat_shade[3*i]     = t_normal;
        flat_shade[3*i+1]   = t_normal;
        flat_shade[3*i+2]   = t_normal;
    }
}


void getSmoothData()
{
    for (int i = 0; i< triangle_num*3; i++)
    {
        vec3 p = vec3(shade_points[i][0], shade_points[i][1], shade_points[i][2]);
        smooth_shade[i] = normalize(p);
    }
}



//-------------------------------
// generate 2 triangles: 6 vertices and 6 colors
void floor()
{
    floor_colors[0] = vertex_colors[3]; floor_points[0] = floor_vertices[0];
    floor_colors[1] = vertex_colors[3]; floor_points[1] = floor_vertices[1];
    floor_colors[2] = vertex_colors[3]; floor_points[2] = floor_vertices[2];
  
    floor_colors[3] = vertex_colors[3]; floor_points[3] = floor_vertices[2];
    floor_colors[4] = vertex_colors[3]; floor_points[4] = floor_vertices[3];
    floor_colors[5] = vertex_colors[3]; floor_points[5] = floor_vertices[0];
}



void lighting_floor()
{
    vec4 u = floor_lighting_vertices[0] - floor_lighting_vertices[1];
    vec4 v = floor_lighting_vertices[2] - floor_lighting_vertices[1];
    cout << u << endl; //( 0, 0, 12, 0 )
    cout << v << endl; // ( -10, 0, 0, 0 )
    vec3 normal = normalize(cross(v, u));
    
    floor_lighting_colors[0] = normal;
    floor_lighting_colors[1] = normal;
    floor_lighting_colors[2] = normal;
    
    floor_lighting_colors[3] = normal;
    floor_lighting_colors[4] = normal;
    floor_lighting_colors[5] = normal;
    
    floor_lighting_points[0] = floor_lighting_vertices[0];
    floor_lighting_points[1] = floor_lighting_vertices[1];
    floor_lighting_points[2] = floor_lighting_vertices[2];
    
    floor_lighting_points[3] = floor_lighting_vertices[2];
    floor_lighting_points[4] = floor_lighting_vertices[3];
    floor_lighting_points[5] = floor_lighting_vertices[0];
}



//----------------------------------------------------------------------------
// let user input filemame and load data
void loadData()
{
    FILE *fp;
    char filename[50];
    char buff[25];
    float triangle_para[3];
    int p;
    /* input filename */
    cout << "Please input filename:" << endl;
    cin >> filename;
    fp = fopen(filename, "r");
    /* get number of triangles */
    fgets(buff, 255, fp);
    triangle_num = atoi(buff);
    /* get parameters of triangles */
    for (int i = 0; i < triangle_num; i++)
    {
        fscanf(fp, "%d\n", &p);
    
        fscanf(fp, "%f %f %f\n", &triangle_para[0], &triangle_para[1], &triangle_para[2]);
        sphere_points[3*i] = point3(triangle_para[0], triangle_para[1], triangle_para[2]);
        shadow_points[3*i] = point3(triangle_para[0], triangle_para[1], triangle_para[2]);
        shade_points[3*i]  = point4(triangle_para[0], triangle_para[1], triangle_para[2], 1);

        fscanf(fp, "%f %f %f\n", &triangle_para[0], &triangle_para[1], &triangle_para[2]);
        sphere_points[3*i+1] = point3(triangle_para[0], triangle_para[1], triangle_para[2]);
        shadow_points[3*i+1] = point3(triangle_para[0], triangle_para[1], triangle_para[2]);
        shade_points[3*i+1]  = point4(triangle_para[0], triangle_para[1], triangle_para[2], 1);
    
        fscanf(fp, "%f %f %f\n", &triangle_para[0], &triangle_para[1], &triangle_para[2]);
        sphere_points[3*i+2] = point3(triangle_para[0], triangle_para[1], triangle_para[2]);
        shadow_points[3*i+2] = point3(triangle_para[0], triangle_para[1], triangle_para[2]);
        shade_points[3*i+2]  = point4(triangle_para[0], triangle_para[1], triangle_para[2], 1);
    }
    
    fclose(fp);
}



//----------------------------------------------------------------------------
// OpenGL initialization
void init()
{
    loadData();
    colorBallShadow();
    getFlatData();
    getSmoothData();
    floor();
    lighting_floor();
/*
#if 0 //YJC: The following is not needed
    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
#endif
 */
    
    
// Create and initialize a vertex buffer object for sphere, to be used in display()
    glGenBuffers(1, &sphere_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(point3) * sphere_NumVertices + sizeof(color3) * sphere_NumVertices,
                 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point3) * sphere_NumVertices, sphere_points);
    glBufferSubData(GL_ARRAY_BUFFER,
                    sizeof(point3) * sphere_NumVertices,
                    sizeof(color3) * sphere_NumVertices,
                    sphere_colors);
    
    
// Create and initialize a vertex buffer object for shadow, to be used in display()
    glGenBuffers(1, &shadow_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, shadow_buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(point3) * shadow_NumVertices + sizeof(color3) * shadow_NumVertices,
                 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point3) * shadow_NumVertices, shadow_points);
    glBufferSubData(GL_ARRAY_BUFFER,
                    sizeof(point3) * shadow_NumVertices,
                    sizeof(color3) * shadow_NumVertices,
                    shadow_colors);
    
 
// Create and initialize a vertex buffer object for flat, to be used in display()
    glGenBuffers(1, &flat_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, flat_buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(point4) * sphere_NumVertices + sizeof(vec3 ) * sphere_NumVertices,
                 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * sphere_NumVertices, shade_points);
    glBufferSubData(GL_ARRAY_BUFFER,
                    sizeof(point4) * sphere_NumVertices,
                    sizeof(vec3  ) * sphere_NumVertices,
                    flat_shade);
    
    
// Create and initialize a vertex buffer object for smooth, to be used in display()
    glGenBuffers(1, &smooth_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, smooth_buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(point4) * sphere_NumVertices + sizeof(vec3  ) * sphere_NumVertices,
                 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * sphere_NumVertices, shade_points);
    glBufferSubData(GL_ARRAY_BUFFER,
                    sizeof(point4) * sphere_NumVertices,
                    sizeof(vec3  ) * sphere_NumVertices,
                    smooth_shade);
    
    
    
// Create and initialize a vertex buffer object for floor, to be used in display()
    glGenBuffers(1, &floor_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors),
                 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_points), floor_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points), sizeof(floor_colors), floor_colors);
    
    
// Create and initialize a vertex buffer object for lighting floor, to be used in display()
    glGenBuffers(1, &floor_lighting_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, floor_lighting_buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(floor_lighting_points) + sizeof(floor_lighting_colors),
                 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_lighting_points), floor_lighting_points);
    glBufferSubData(GL_ARRAY_BUFFER,
                    sizeof(floor_lighting_points),
                    sizeof(floor_lighting_colors),
                    floor_lighting_colors);

 
    
// Create and initialize a vertex buffer object for x, to be used in display()
    glGenBuffers(1, &x_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, x_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(x_points) + sizeof(x_colors),
                 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(x_points), x_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(x_points), sizeof(x_colors),x_colors);

    
// Create and initialize a vertex buffer object for y, to be used in display()
    glGenBuffers(1, &y_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, y_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(y_points) + sizeof(y_colors),
                 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(y_points), y_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(y_points), sizeof(y_colors), y_colors);
 
    
// Create and initialize a vertex buffer object for z, to be used in display()
    glGenBuffers(1, &z_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, z_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(z_points) + sizeof(z_colors),
                 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(z_points), z_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(z_points), sizeof(z_colors), z_colors);
  
  
 // Load shaders and create a shader program (to be used in display())
 
    glEnable( GL_DEPTH_TEST );
    glClearColor( 0.529, 0.807, 0.92, 1.0 );
    glLineWidth( 1.5 );
    
    program     = InitShader("vshader42.glsl", "fshader42.glsl");
    program2    = InitShader("vshader53.glsl", "fshader53.glsl");
    
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

    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    GLuint vColor = glGetAttribLocation(program, "vColor"); 
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(sizeof(point3) * num_vertices) );
    // the offset is the (total) size of the previous vertex attribute array(s)

    /* Draw a sequence of geometric objs (triangles) from the vertex buffer
       (using the attributes specified in each enabled vertex attribute array) */
    glDrawArrays(GL_TRIANGLES, 0, num_vertices);

    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);
}


void drawObj2(GLuint buffer, int num_vertices)
{
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    
    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint vPosition = glGetAttribLocation(program2, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    
    GLuint vNormal = glGetAttribLocation(program2, "vNormal");
    glEnableVertexAttribArray(vNormal);
    glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(num_vertices * sizeof(point4)));
    // the offset is the (total) size of the previous vertex attribute array(s)
    
    /* Draw a sequence of geometric objs (triangles) from the vertex buffer
     (using the attributes specified in each enabled vertex attribute array) */
    glDrawArrays(GL_TRIANGLES, 0, num_vertices);
    
    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vNormal);
}


mat4 BallToShadow = mat4(12.0,  0.,  0.,  0.,
                          14.,  0.,  3., -1.,
                           0.,  0., 12.,  0.,
                           0.,  0.,  0., 12.);

mat4 AccumRotate = mat4(  1,  0,  0,  0,
                          0,  1,  0,  0,
                          0,  0,  1,  0,
                          0,  0,  0,  1);

mat4    TransL;
mat4    Ball;
int     routeFlag = 0;  // 0: A to B  1: B to C  2: C to B
vec3    rotate_axis;
float   d;


void DrawXYZ(mat4 mv0)
{
    /*----- Set up the Mode-View matrix for x y z-----*/
    glUseProgram(program);
    model_view = glGetUniformLocation(program, "model_view" );
    projection = glGetUniformLocation(program, "projection" );
    
    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv0); // GL_TRUE: matrix is row-major
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    drawObj(x_buffer, axis_NumVertices);
    drawObj(y_buffer, axis_NumVertices);
    drawObj(z_buffer, axis_NumVertices);
    
}


void DrawFloor(mat4 mv0, mat4 p)
{
    /*----- Set up the Mode-View matrix for the floor -----*/
        // The set-up below gives a new scene (scene 2), using Correct LookAt() function
    
    if (lightingFlag == 0) // original floor
    {
        glUseProgram(program);
        model_view = glGetUniformLocation(program, "model_view" );
        projection = glGetUniformLocation(program, "projection" );
    
        glUniformMatrix4fv(projection, 1, GL_TRUE, p);
        glUniformMatrix4fv(model_view, 1, GL_TRUE, mv0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        drawObj(floor_buffer, floor_NumVertices);  // draw the floor
    }
    
    else // lighting shadow
    {
        glUseProgram(program2);
        model_view2 = glGetUniformLocation(program2, "ModelView2" );
        projection2 = glGetUniformLocation(program2, "Projection2" );
    
        glUniformMatrix4fv(projection2, 1, GL_TRUE, p);
    

        if (pointlightFlag==1)  SetUp_Point_Light_Var(mv0);
        else                    SetUp_Spot_Light_Var(mv0);
        SetUp_Floor_Material_Var();
    
        // SetUp_Floor_Lighting_Uniform_Vars(mv0);
    
        glUniformMatrix4fv(model_view2, 1, GL_TRUE, mv0 );
        mat3 normal_matrix = NormalMatrix(mv0, 1);
        glUniformMatrix3fv(glGetUniformLocation(program2, "Normal_Matrix"), 1, GL_TRUE, normal_matrix);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        drawObj2(floor_lighting_buffer, floor_NumVertices);
    
    }
}


void DrawSphere(mat4 mv0, mat4 p)
{
    mat4    mv = mv0;
    /*----- Set Up the Model-View matrix for the sphere -----*/
    
        // get sphere matrix
    d = (2 * PI * angle) / 360;
    float total_path = length(B - A) + length(C - B) + length(A - C);
    float temp_d;
        // A to B
    if ((routeFlag == 0 && d < length(B - A)) || (routeFlag == 2 && d >= length(A - C)) )
    {   if (routeFlag == 2)
        {   AccumRotate = Rotate(angle - speed, rotate_axis[0], rotate_axis[1], rotate_axis[2]) * AccumRotate;
            angle = 0.0; routeFlag = 0; d = (2 * PI * angle) / 360;
        }
        rotate_axis = cross(OY, B - A);
        TransL = Translate(A + d * normalize(B - A));
    }
        // B to C
    else if ((routeFlag == 1 && d < length(C - B)) || (routeFlag == 0 && d >= length(B - A)) )
    {   if (routeFlag == 0)
        {   AccumRotate = Rotate(angle - speed, rotate_axis[0], rotate_axis[1], rotate_axis[2]) * AccumRotate;
            angle = 0.0; routeFlag = 1; d = (2 * PI * angle) / 360;
        }
        rotate_axis = cross(OY, C - B);
        TransL = Translate(B + d * normalize(C - B));
    }
        // C to A
    else if ((routeFlag == 2 && d < length(A - C))|| (routeFlag == 1 && d >= length(C - B)) )
    {   if (routeFlag == 1)
        {   AccumRotate = Rotate(angle - speed, rotate_axis[0], rotate_axis[1], rotate_axis[2]) * AccumRotate;
            angle = 0.0; routeFlag = 2; d = (2 * PI * angle) / 360;
        }
        rotate_axis = cross(OY, A - C);
        TransL = Translate(C + d * normalize(A - C));
    }
    
    
    Ball = TransL * Scale (1.0, 1.0, 1.0)
                  * Rotate(angle, rotate_axis[0], rotate_axis[1], rotate_axis[2])
                  * AccumRotate;
    mv = mv * Ball;
    
    
    
    
    // draw sphere
    if ((flatshadingFlag == 0 && smoothshadingFlag == 0) || (sphereFlag == 0))
    {
        glUseProgram(program);
        
        model_view = glGetUniformLocation(program, "model_view" );
        projection = glGetUniformLocation(program, "projection" );
        glUniformMatrix4fv(projection, 1, GL_TRUE, p);
        glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);
        
        if (sphereFlag == 1)    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Filled sphere
        else                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);// Wireframe sphere
        
        drawObj(sphere_buffer, sphere_NumVertices);  // draw the sphere
    }
    
    else
    {
        if (flatshadingFlag == 1)
        {
            glUseProgram(program2);
        
            model_view2 = glGetUniformLocation(program2, "ModelView2" );
            projection2 = glGetUniformLocation(program2, "Projection2" );
            glUniformMatrix4fv(projection2, 1, GL_TRUE, p);
            glUniformMatrix4fv(model_view2, 1, GL_TRUE, mv);
        
            if (pointlightFlag==1)  SetUp_Point_Light_Var(mv0);
            else                    SetUp_Spot_Light_Var(mv0);
            SetUp_Sphere_Material_Var();
        
            mat3 normal_matrix = NormalMatrix(mv, 1);
            glUniformMatrix3fv(glGetUniformLocation(program2, "Normal_Matrix"), 1, GL_TRUE, normal_matrix);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            drawObj2(flat_buffer,  sphere_NumVertices);
        }
        
        else if (smoothshadingFlag == 1)
        {
            glUseProgram(program2);
        
            model_view2 = glGetUniformLocation(program2, "ModelView2" );
            projection2 = glGetUniformLocation(program2, "Projection2" );
            glUniformMatrix4fv(projection2, 1, GL_TRUE, p);
            glUniformMatrix4fv(model_view2, 1, GL_TRUE, mv);

            if (pointlightFlag==1)  SetUp_Point_Light_Var(mv0);
            else                    SetUp_Spot_Light_Var(mv0);
            SetUp_Sphere_Material_Var();
        
            mat3 normal_matrix = NormalMatrix(mv, 1);
            glUniformMatrix3fv(glGetUniformLocation(program2, "Normal_Matrix"), 1, GL_TRUE, normal_matrix);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            drawObj2(smooth_buffer, sphere_NumVertices);
        }
    }
}


void DrawShadow(mat4 mv0, mat4 p)
{
    mat4    mv = mv0;
    /*----- Set up the Mode-View matrix for the shadow-----*/
    if (shadowFlag == 1)
    {
        glUseProgram(program);
        model_view = glGetUniformLocation(program, "model_view" );
        projection = glGetUniformLocation(program, "projection" );

        mv = mv0 * BallToShadow * Ball;
            //glUniformMatrix4fv(model_view2, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
        glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);
        if (sphereFlag == 1)    // Filled shadow
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        else                    // Wireframe shadow
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        drawObj(shadow_buffer, shadow_NumVertices);
            //drawObj2(shadow_buffer, shadow_NumVertices);  // draw the floor
    }
}



//----------------------------------------------------------------------------
void display( void )
{

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    //glUseProgram(program); // Use the shader program


    
/*---  Set up and pass on Projection matrix to the shader ---*/
    mat4    p = Perspective(fovy, aspect, zNear, zFar);
    //glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

    
/*---  Set up and pass on Model-View matrix to the shader ---*/
    // eye is a global variable of vec4 set to init_eye and updated by keyboard()
    vec4    at(0.0, 0.0, 0.0, 1.0);
    vec4    up(0.0, 1.0, 0.0, 0.0);
    mat4    mv0 = LookAt(eye, at, up);
 
    glEnable(GL_DEPTH_TEST); // always enable test

    
    if (eye[1] >= 0.0)// when viewer is above plane,
    {
        glDepthMask(GL_FALSE); // read anything into buffer but floor
        DrawFloor(mv0, p);
        glDepthMask(GL_TRUE);
    
        DrawSphere(mv0, p);
        DrawShadow(mv0, p);
        DrawXYZ(mv0);
    }
    else // when viewer is under the plane
    {
        DrawSphere(mv0, p);
    
        glDepthMask(GL_FALSE);
        DrawShadow(mv0, p);
        glDepthMask(GL_TRUE);
    
        DrawXYZ(mv0);
        DrawFloor(mv0, p);
    }
    glutSwapBuffers();
}



//---------------------------------------------------------------------------
void idle (void)
{
    //angle += 0.02;
    angle += speed;    //YJC: change this value to adjust the cube rotation speed.
    glutPostRedisplay();
}



//----------------------------------------------------------------------------
void keyboard(unsigned char key, int x, int y)
{
    switch(key)
    {
        case 'X': eye[0] += 1.0; break;
        case 'x': eye[0] -= 1.0; break;
        case 'Y': eye[1] += 1.0; break;
        case 'y': eye[1] -= 1.0; break;
        case 'Z': eye[2] += 1.0; break;
        case 'z': eye[2] -= 1.0; break;

        case 'b': case 'B': // Toggle between animation and non-animation
            if (beginFlag == 0)
            {
                animationFlag = 1;
                beginFlag = 1;
            }
            if (animationFlag == 1) glutIdleFunc(idle);
            else                    glutIdleFunc(NULL);
            break;
        
        case 's': case 'S': // Toggle between filled and wireframe sphere
            sphereFlag = 1 -  sphereFlag;
            break;
        
        case 'f': case 'F': // Toggle between filled and wireframe floor
            floorFlag = 1 -  floorFlag;
            break;
    }
    
    glutPostRedisplay();
}



//----------------------------------------------------------------------------
void myMouse(int button, int state, int x, int y)
{
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN && beginFlag == 1)
    {
        animationFlag = 1 - animationFlag;
        if (animationFlag == 1)
            glutIdleFunc(idle);
        else
            glutIdleFunc(NULL);
    }
}



//----------------------------------------------------------------------------
void top_menu(int id)
{
    switch(id)
    {
        case 1: // back to init position
            eye = init_eye;
            break;
        case 2: // wireframe
            sphereFlag = 1-sphereFlag;
            break;
        case 3: // quit
            exit(0);
            break;
    }
    glutPostRedisplay();
}

//----------------------------------------------------------------------------
void shadow_menu(int id)
{
    switch(id)
    {
        case 1: // No
            shadowFlag = 0;
            break;
        case 2: // Yes
            shadowFlag = 1;
            break;
    }
    glutPostRedisplay();
}

//----------------------------------------------------------------------------
void shade_menu(int id)
{
    switch(id)
    {
        case 1: // flat shading
            if (lightingFlag == 1)
            {
                flatshadingFlag  = 1;
                smoothshadingFlag= 0;
            }
            sphereFlag           = 1;
            break;
    
        case 2: // smooth shading
            if (lightingFlag == 1)
            {
                flatshadingFlag  = 0;
                smoothshadingFlag= 1;
            }
            sphereFlag           = 1;
            break;
    }
    glutPostRedisplay();
}


//----------------------------------------------------------------------------
void lighting_menu(int id)
{
    switch(id)
    {
        case 1: // No
            lightingFlag        = 0;
            break;
        case 2: // Yes
            lightingFlag        = 1;
            break;
    }
    glutPostRedisplay();
}


//----------------------------------------------------------------------------
void light_source_menu(int id)
{
    switch(id)
    {
        case 1: // spot light
            spotlightFlag       = 1;
            pointlightFlag      = 0;
            break;
        case 2: // point
            spotlightFlag       = 0;
            pointlightFlag      = 1;
            break;
    }
    glutPostRedisplay();
}


//----------------------------------------------------------------------------
void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    aspect = (GLfloat) width  / (GLfloat) height;
    Perspective(fovy, aspect, zNear, zFar);
    glutPostRedisplay();
}



//----------------------------------------------------------------------------
void add_menu()
{
    int shadow_id = glutCreateMenu(shadow_menu);
    glutAddMenuEntry("No", 1);
    glutAddMenuEntry("Yes", 2);
    
    int shade_id = glutCreateMenu(shade_menu);
    glutAddMenuEntry("flat shading", 1);
    glutAddMenuEntry("smooth shading", 2);
    
    int lighting_id = glutCreateMenu(lighting_menu);
    glutAddMenuEntry("No", 1);
    glutAddMenuEntry("Yes", 2);
    
    int light_source_id = glutCreateMenu(light_source_menu);
    glutAddMenuEntry("spot light", 1);
    glutAddMenuEntry("point source", 2);
    
    glutCreateMenu(top_menu);
    glutAddMenuEntry("Default View Point", 1);
    glutAddMenuEntry("Wire Frame Sphere", 2);
    glutAddMenuEntry("Quit", 3);
    
    glutAddSubMenu("Shadow", shadow_id);
    glutAddSubMenu("Shading", shade_id);
    glutAddSubMenu("Enable Lighting", lighting_id);
    glutAddSubMenu("Light Source", light_source_id);
    glutAttachMenu(GLUT_LEFT_BUTTON);
}



//----------------------------------------------------------------------------
int main(int argc, char **argv)
{
    int err;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(512, 512);
    glutCreateWindow("Rolling Sphere with Shading");

/* Call glewInit() and error checking */
    err = glewInit();
    if (GLEW_OK != err)
    {
        printf("Error: glewInit failed: %s\n", (char*) glewGetErrorString(err));
        exit(1);
    }
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(NULL);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(myMouse);
    
    init();
    add_menu(); // sub menu

    glutMainLoop();
    return 0;
}
