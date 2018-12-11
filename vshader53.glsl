/* 
File Name: "vshader53.glsl":
Vertex shader:
  - Per vertex shading for a single point light source;
    distance attenuation is Yet To Be Completed.
  - Entire shading computation is done in the Eye Frame.
*/

#version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

in  vec4 vPosition;
in  vec3 vNormal;
in  vec4 vColor;
out vec4 color;
//out vec4 colorshadow;

uniform vec4 lightambient, lightdiffuse, lightspecular;
uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform vec4 materialambient;
uniform vec4 materialdiffuse;
uniform vec4 materialspecular;
uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat3 Normal_Matrix;
uniform vec4 LightPosition;   // Must be in Eye Frame
uniform vec4 LightPosition2;   // Must be in Eye Frame
uniform float Shininess;
uniform vec4  spot_direction;

uniform float objectFlag;
uniform float lightsourceFlag;
uniform float ConstAtt;  // Constant Attenuation
uniform float LinearAtt; // Linear Attenuation
uniform float QuadAtt;   // Quadratic Attenuation

void main()
{
    float spotex = 15;
    float spotangle = cos(20.0 * 3.1415 / 180);
    vec4 global_ambient = vec4(1.0, 1.0, 1.0, 1.0);

    // Transform vertex  position into eye coordinates
    vec3 pos = (ModelView * vPosition).xyz;
	
    vec3 L = normalize(  LightPosition.xyz - pos  );
    vec3 E = normalize( -pos );
    vec3 H = normalize( L + E );
    vec3 L2 = normalize( LightPosition2.xyz - pos );
    vec3 E2 = normalize( -pos );
    vec3 H2 = normalize( L2 + E2 );

    // Transform vertex normal into eye coordinates
    vec3 N = normalize(Normal_Matrix * vNormal);

    // YJC Note: N must use the one pointing *toward* the viewer
    //     ==> If (N dot E) < 0 then N must be changed to -N
    //
    if ( dot(N, E) < 0 ) N = -N;
    
/*--- To Do: Compute attenuation ---*/
float attenuation1 = 1.0;

    // Compute terms in the illumination equation
    vec4 ambient = AmbientProduct;
    vec4 globalambient = global_ambient * materialambient;
    
    float d1 = max( dot(L, N), 0.0 );
    vec4  diffuse = d1 * DiffuseProduct;
    
    float s1 = pow( max(dot(N, H), 0.0), Shininess );
    vec4  specular = s1 * SpecularProduct;
    
    if( dot(L, N) < 0.0 ) {
        specular = vec4(0.0, 0.0, 0.0, 1.0);
    }
    
    float dis = length(LightPosition2.xyz - pos);
    float attenuation2 = 1.0 / float(ConstAtt + LinearAtt * dis + QuadAtt * dis * dis);
    float spot_attenuation;
    // Compute terms in the illumination equation
    vec4  ambient2 = lightambient * materialambient;
    
    float d = max( dot(L2, N), 0.0 );
    vec4  diffuse2 = d * lightdiffuse * materialdiffuse;
    
    float s = pow( max(dot(N, H2), 0.0), Shininess );
    vec4  specular2 = s * lightspecular * materialspecular;
    
    if( dot(L2, N) < 0.0 )
        specular2 = vec4(0.0, 0.0, 0.0, 1.0);
    
    vec3 s2 = -L2;
    vec3 lf = normalize( spot_direction.xyz - LightPosition2.xyz );
    if ( (dot(s2,lf)) < spotangle)
        spot_attenuation = 0;
    else
        spot_attenuation = pow(dot(s2, lf), spotex);
    
/*--- attenuation below must be computed properly ---*/
    vec4 pointlight = attenuation2 * (ambient2 + diffuse2 + specular2);
    
    vec4 spotlight = spot_attenuation * pointlight;
    
    gl_Position = Projection * ModelView * vPosition;

    //point source shading
    if (lightsourceFlag==2)
        color = globalambient + ambient + diffuse + specular + pointlight;
    else if (lightsourceFlag==1)
        color = globalambient + ambient + diffuse + specular + spotlight;
    else if (lightsourceFlag==0)
        color = globalambient + ambient + diffuse + specular;
    
    if(objectFlag==1)//original sphere
        color = vColor;
}
