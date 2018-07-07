//per vf
//headers
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#import <QuartzCore/CVDisplayLink.h>

#import <OpenGL/gl3.h>
#import <OpenGL/gl3ext.h>
#import "vmath.h"

#include <vector>

//'C' style global function declaration
CVReturn MyDisplayLinkCallback(CVDisplayLinkRef, const CVTimeStamp *,const CVTimeStamp *,CVOptionFlags,CVOptionFlags *,void *);

//global variables
FILE *gpFile=NULL;

std::vector<GLfloat> gSphereVertices;
std::vector<GLfloat> gSphereNormals;
std::vector<GLfloat> gSphereTexcoords;
std::vector<GLushort> gSphereElements;

GLfloat fovy = 30.0;
GLuint gAnimateToggle, gLightToggle;

GLuint gNumElements;
GLuint gNumVertices;

GLfloat light_ambient[]={0.5f,0.5f,0.5f,1.0f};
GLfloat light_defused[]={1.0f,1.0f,1.0f,1.0f};
GLfloat light_specular[]={1.0f,1.0f,1.0f,1.0f};
GLfloat light_position[]={0.0f,0.0f,1.0f,0.0f};

GLfloat s1_material_ambient[]={0.0215f,0.1745f,0.0215f,1.0f};
GLfloat s1_material_diffuse[]={0.07568f,0.61424f,0.07568f,1.0f};
GLfloat s1_material_specular[]={0.633f,0.727811f,0.633f,1.0f};
GLfloat s1_material_shininess=0.6f * 128;

GLfloat s2_material_ambient[]={0.135f,0.2225f,0.1575f,1.0f};
GLfloat s2_material_diffuse[]={0.54f,0.89f,0.63f,1.0f};
GLfloat s2_material_specular[]={0.316228f,0.316228f,0.316228f,1.0f};
GLfloat s2_material_shininess=0.1f * 128;

GLfloat s3_material_ambient[]={0.05375f,0.05f,0.06625f,1.0f};
GLfloat s3_material_diffuse[]={0.18275f,0.17f,0.22525f,1.0f};
GLfloat s3_material_specular[]={0.332741f,0.328634f,0.346435f,1.0f};
GLfloat s3_material_shininess=0.3f * 128;

GLfloat s4_material_ambient[]={0.25f,0.20725f,0.20725f,1.0f};
GLfloat s4_material_diffuse[]={1.0f,0.829f,0.829f,1.0f};
GLfloat s4_material_specular[]={0.296648f,0.296648f,0.296648f,1.0f};
GLfloat s4_material_shininess=0.088f * 128;

GLfloat s5_material_ambient[]={0.1745f,0.01175f,0.01175f,1.0f};
GLfloat s5_material_diffuse[]={0.61424f,0.04136f,0.04136f,1.0f};
GLfloat s5_material_specular[]={0.727811f,0.626959f,0.626959f,1.0f};
GLfloat s5_material_shininess=0.6f * 128;

GLfloat s6_material_ambient[]={0.1f,0.18725f,0.1745f,1.0f};
GLfloat s6_material_diffuse[]={0.396f,0.74151f,0.69102f,1.0f};
GLfloat s6_material_specular[]={0.297254f,0.30829f,0.306678f,1.0f};
GLfloat s6_material_shininess=0.1f * 128;

GLfloat s7_material_ambient[]={0.329412f,0.223529f,0.027451f,1.0f};
GLfloat s7_material_diffuse[]={0.780392f,0.568627f,0.113725f,1.0f};
GLfloat s7_material_specular[]={0.992157f,0.941176f,0.807843f,1.0f};
GLfloat s7_material_shininess=0.21794872f * 128;

GLfloat s8_material_ambient[]={0.2125f,0.1275f,0.054f,1.0f};
GLfloat s8_material_diffuse[]={0.714f,0.4284f,0.18144f,1.0f};
GLfloat s8_material_specular[]={0.393548f,0.271906f,0.166721f,1.0f};
GLfloat s8_material_shininess=0.2f * 128;

GLfloat s9_material_ambient[]={0.25f,0.25f,0.25f,1.0f};
GLfloat s9_material_diffuse[]={0.4f,0.4f,0.4f,1.0f};
GLfloat s9_material_specular[]={0.774597f,0.774597f,0.774597f,1.0f};
GLfloat s9_material_shininess=0.6f * 128;

GLfloat s10_material_ambient[]={0.19125f,0.0735f,0.0225f,1.0f};
GLfloat s10_material_diffuse[]={0.7038f,0.27048f,0.0828f,1.0f};
GLfloat s10_material_specular[]={0.256777f,0.137622f,0.086014f,1.0f};
GLfloat s10_material_shininess=0.1f * 128;

GLfloat s11_material_ambient[]={0.24725f,0.1995f,0.0745f,1.0f};
GLfloat s11_material_diffuse[]={0.75164f,0.60648f,0.22648f,1.0f};
GLfloat s11_material_specular[]={0.628281f,0.555802f,0.366065f,1.0f};
GLfloat s11_material_shininess=0.4f * 128;

GLfloat s12_material_ambient[]={0.19225f,0.19225f,0.19225f,1.0f};
GLfloat s12_material_diffuse[]={0.50754f,0.50754f,0.50754f,1.0f};
GLfloat s12_material_specular[]={0.508273f,0.508273f,0.508273f,1.0f};
GLfloat s12_material_shininess=0.4f * 128;

GLfloat s13_material_ambient[]={0.0f,0.0f,0.0f,1.0f};
GLfloat s13_material_diffuse[]={0.01f,0.01f,0.01f,1.0f};
GLfloat s13_material_specular[]={0.50f,0.50f,0.50f,1.0f};
GLfloat s13_material_shininess=0.25f * 128;

GLfloat s14_material_ambient[]={0.0f,0.1f,0.06f,1.0f};
GLfloat s14_material_diffuse[]={0.0f,0.50980392f,0.50980392f,1.0f};
GLfloat s14_material_specular[]={0.50196078f,0.50196078f,0.50196078f,1.0f};
GLfloat s14_material_shininess=0.25f * 128;

GLfloat s15_material_ambient[]={0.0f,0.0f,0.0f,1.0f};
GLfloat s15_material_diffuse[]={0.1f,0.35f,0.1f,1.0f};
GLfloat s15_material_specular[]={0.45f,0.55f,0.45f,1.0f};
GLfloat s15_material_shininess=0.25f * 128;

GLfloat s16_material_ambient[]={0.0f,0.0f,0.0f,1.0f};
GLfloat s16_material_diffuse[]={0.5f,0.0f,0.0f,1.0f};
GLfloat s16_material_specular[]={0.7f,0.6f,0.6f,1.0f};
GLfloat s16_material_shininess=0.25f * 128;

GLfloat s17_material_ambient[]={0.0f,0.0f,0.0f,1.0f};
GLfloat s17_material_diffuse[]={0.55f,0.55f,0.55f,1.0f};
GLfloat s17_material_specular[]={0.70f,0.70f,0.70f,1.0f};
GLfloat s17_material_shininess=0.25f * 128;

GLfloat s18_material_ambient[]={0.0f,0.0f,0.0f,1.0f};
GLfloat s18_material_diffuse[]={0.5f,0.5f,0.0f,1.0f};
GLfloat s18_material_specular[]={0.60f,0.60f,0.50f,1.0f};
GLfloat s18_material_shininess=0.25f * 128;

GLfloat s19_material_ambient[]={0.02f,0.02f,0.02f,1.0f};
GLfloat s19_material_diffuse[]={0.01f,0.01f,0.01f,1.0f};
GLfloat s19_material_specular[]={0.4f,0.4f,0.4f,1.0f};
GLfloat s19_material_shininess=0.078125f * 128;

GLfloat s20_material_ambient[]={0.0f,0.05f,0.05f,1.0f};
GLfloat s20_material_diffuse[]={0.4f,0.5f,0.5f,1.0f};
GLfloat s20_material_specular[]={0.04f,0.7f,0.7f,1.0f};
GLfloat s20_material_shininess=0.078125f * 128;

GLfloat s21_material_ambient[]={0.0f,0.05f,0.0f,1.0f};
GLfloat s21_material_diffuse[]={0.4f,0.5f,0.4f,1.0f};
GLfloat s21_material_specular[]={0.04f,0.7f,0.04f,1.0f};
GLfloat s21_material_shininess=0.078125f * 128;

GLfloat s22_material_ambient[]={0.05f,0.0f,0.0f,1.0f};
GLfloat s22_material_diffuse[]={0.5f,0.4f,0.4f,1.0f};
GLfloat s22_material_specular[]={0.7f,0.04f,0.04f,1.0f};
GLfloat s22_material_shininess=0.078125f * 128;

GLfloat s23_material_ambient[]={0.05f,0.05f,0.05f,1.0f};
GLfloat s23_material_diffuse[]={0.5f,0.5f,0.5f,1.0f};
GLfloat s23_material_specular[]={0.7f,0.7f,0.7f,1.0f};
GLfloat s23_material_shininess=0.078125f * 128;

GLfloat s24_material_ambient[]={0.05f,0.05f,0.0f,1.0f};
GLfloat s24_material_diffuse[]={0.5f,0.5f,0.4f,1.0f};
GLfloat s24_material_specular[]={0.7f,0.7f,0.04f,1.0f};
GLfloat s24_material_shininess=0.078125f * 128;

GLfloat angleRotate = 0.0;

float zAxis = -15.0f;
float gRotateRedAngle = 0.0f;
bool gbXPressed=false;
bool gbYPressed=false;
bool gbZPressed=false;

GLfloat angleRedLight = 0.0f;
GLfloat angleGreenLight = 0.0f;
GLfloat angleBlueLight = 0.0f;

enum
{
 VDG_ATTRIBUTE_VERTEX=0,
    VDG_ATTRIBUTE_COLOR=1,
    VDG_ATTRIBUTE_NORMAL=2,
    VDG_ATTRIBUTE_TEXTURE0=3,
    
};

//interface declarations
@interface AppDelegate : NSObject<NSApplicationDelegate, NSWindowDelegate>

@end

@interface GLView : NSOpenGLView

@end

//entry-point function
int main(int argc, const char * argv[])
{
    //code
    NSAutoreleasePool *pPool=[[NSAutoreleasePool alloc]init];
    
    NSApp=[NSApplication sharedApplication];
    
    [NSApp setDelegate:[[AppDelegate alloc]init]];
    
    [NSApp run];
    
    [pPool release];
    
    return(0);
}

//interface implementation
@implementation AppDelegate
{
    @private
    NSWindow *window;
    GLView *glView;
}
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    //code
    //log file
    NSBundle *mainBundle=[NSBundle mainBundle];
    NSString *appDirName=[mainBundle bundlePath];
    NSString *parentDirPath=[appDirName stringByDeletingLastPathComponent];
    NSString *logFileNameWithPath=[NSString stringWithFormat:@"%@/log.txt",parentDirPath];
    const char *pszLogFileNameWithPath=[logFileNameWithPath cStringUsingEncoding:NSASCIIStringEncoding];
    gpFile=fopen(pszLogFileNameWithPath,"w");
    if(gpFile==NULL)
    {
        printf("Can not create log file.\nExitting...\n");
        [self release];
        [NSApp terminate:self];
    }
    fprintf(gpFile,"Program is started successfully...\n");
    
    //window
    NSRect win_rect;
    win_rect=NSMakeRect(0.0, 0.0, 800.0, 600.0);
    
    //create simple window
    window=[[NSWindow alloc] initWithContentRect:win_rect
                                       styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                                        NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable
                                         backing:NSBackingStoreBuffered
                                           defer:NO];
    
    [window setTitle:@"macOS OpenGL Window"];
    [window center];
    
    glView=[[GLView alloc]initWithFrame:win_rect];
    
    [window setContentView:glView];
    [window setDelegate:self];
    [window makeKeyAndOrderFront:self];
}

- (void)applicationWillTerminate:(NSNotification *)notification
{
    //code
    fprintf(gpFile, "Program is terminated successfully.\n");
    if(gpFile)
    {
        fclose(gpFile);
        gpFile=NULL;
    }
}

- (void)windowWillClose:(NSNotification *)notification
{
    //code
    [NSApp terminate:self];
}

- (void)dealloc
{
    //code
    [glView release];
    
    [window release];
    
    [super dealloc];
}
@end

@implementation GLView
{
    @private
    CVDisplayLinkRef displayLink;
    
    //for shaders
    GLuint gVertexShaderObject;
    GLuint gFragmentShaderObject;
    GLuint gShaderProgramObject;
    
    //vertex array and buffer objects
    GLuint gVao_sphere;
    GLuint gVboSpherePosition;
    GLuint gVboSphereNormal;
    GLuint gVbo_sphere_element;
    
    //uniforms
    GLuint gModelMatrixUniform, gViewMatrixUniform, gProjectionMatrixUniform;
    GLuint gLKeyPressedUniform;
    
    GLuint gLdUniform;
    GLuint gLaUniform;
    GLuint gLsUniform;
    GLuint gLpUniform;
    
    GLuint gKdUniform;
    GLuint gKaUniform;
    GLuint gKsUniform;
    GLuint gKShininessUniform;
    
    //projection matricex
    vmath::mat4 gPerspectiveProjectionMatrix;
    
    //key handling
    BOOL gbVKeyPressed;
    BOOL gbFKeyPressed;
    
    
}
-(id)initWithFrame:(NSRect)frame;
{
    //code
    self=[super initWithFrame:frame];
    if(self)
    {
        [[self window]setContentView:self];
        
        NSOpenGLPixelFormatAttribute attrs[]=
        {
            //must specify the 4.1 core profile to use OpenGL 4.1
            NSOpenGLPFAOpenGLProfile,
            NSOpenGLProfileVersion4_1Core,
            //specify the display ID to associate the GL context with (main display for now)
            NSOpenGLPFAScreenMask,CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay),
            NSOpenGLPFANoRecovery,
            NSOpenGLPFAAccelerated,
            NSOpenGLPFAColorSize, 24,
            NSOpenGLPFADepthSize, 24,
            NSOpenGLPFAAlphaSize, 8,
            NSOpenGLPFADoubleBuffer,
            0
        };
        
        NSOpenGLPixelFormat *pixelFormat=[[[NSOpenGLPixelFormat alloc]initWithAttributes:attrs]autorelease];
        
        if(pixelFormat==nil)
        {
            fprintf(gpFile, "No Valid OpenGL Pixel Format Is Available. Exitting...");
            [self release];
            [NSApp terminate:self];
        }
        
        NSOpenGLContext *glContext=[[[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil]autorelease];
        
        [self setPixelFormat:pixelFormat];
        
        [self setOpenGLContext:glContext]; //it automatically releases the older context, if present, and sets the newer one
    }
    
    gbVKeyPressed=true;
    gbFKeyPressed=false;
    
    return(self);
}

-(CVReturn)getFrameForTime:(const CVTimeStamp *)pOutputTime
{
    //code
    NSAutoreleasePool *pool=[[NSAutoreleasePool alloc]init];
    
    [self drawView];
    
    [pool release];
    
    return(kCVReturnSuccess);
}

-(void)prepareOpenGL
{
    //code
    //OpenGL Info
    fprintf(gpFile, "OpenGL Version: %s\n", glGetString(GL_VERSION));
    fprintf(gpFile, "GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    [[self openGLContext]makeCurrentContext];
    
    GLint swapInt=1;
    [[self openGLContext]setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
    
    
    
    //vertices, colors, shader attribs, vbo, vao initialization
    getSphereVertxData(0.5, 32, 16);
    
    std::vector<GLfloat>::iterator v = gSphereVertices.begin();
    std::vector<GLfloat>::iterator n = gSphereNormals.begin();
    std::vector<GLfloat>::iterator t = gSphereTexcoords.begin();
    std::vector<GLushort>::iterator i = gSphereElements.begin();
    
    gNumElements = gSphereElements.size();
    
    fprintf(gpFile, "Sphere Vertices Size: %lu\n", gSphereVertices.size());
    int count = 0;
    for(std::vector<GLfloat>::const_iterator i = gSphereVertices.begin(); i!=gSphereVertices.end(); i++)
    {
        fprintf(gpFile, "%f,", *i);
        if(++count>=3)
        {
            fprintf(gpFile, "\n");
            count=0;
        }
    }
    
    fprintf(gpFile, "Sphere Normals size: %lu\n", gSphereNormals.size());
    fprintf(gpFile, "Sphere Texcoords size: %lu\n", gSphereTexcoords.size());
    fprintf(gpFile, "Sphere Indices size: %lu\n", gSphereElements.size());
    
    //configure vao
    glGenVertexArrays(1, &gVao_sphere);
    glBindVertexArray(gVao_sphere);
    
    glGenBuffers(1, &gVboSpherePosition);
    glBindBuffer(GL_ARRAY_BUFFER, gVboSpherePosition);
    glBufferData(GL_ARRAY_BUFFER, gSphereVertices.size() * sizeof(GLfloat), &gSphereVertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glGenBuffers(1, &gVboSphereNormal);
    glBindBuffer(GL_ARRAY_BUFFER, gVboSphereNormal);
    glBufferData(GL_ARRAY_BUFFER, gSphereNormals.size() * sizeof(GLfloat), &gSphereNormals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(VDG_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(VDG_ATTRIBUTE_NORMAL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glGenBuffers(1, &gVbo_sphere_element);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, gSphereElements.size() * sizeof(GLushort), &gSphereElements[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    
    
    glBindVertexArray(0);
    
    //
    //glShadeModel(GL_SMOOTH);
    glClearDepth(1.0f);
    
    //enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    //depth test to do
    glDepthFunc(GL_LEQUAL);
    
    //we will always cull back faces for better performance
    //glEnable(GL_CULL_FACE);
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    //set background color
    glClearColor(0.1f, 0.2f, 0.3f, 0.0f); //blue
    
    gPerspectiveProjectionMatrix = vmath::mat4::identity();
    
    gAnimateToggle = 0;
    gLightToggle = 0;
    
    gbVKeyPressed=true;
    gbFKeyPressed=false;
    
    //video refresh rate linking
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, self);
    CGLContextObj cglContext=(CGLContextObj) [[self openGLContext]CGLContextObj];
    CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj) [[self pixelFormat]CGLPixelFormatObj];
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
    CVDisplayLinkStart(displayLink);
}

-(void)rotate
{
    
    gRotateRedAngle = gRotateRedAngle + 0.1f;
    if (gRotateRedAngle >= 360) {
        gRotateRedAngle = 0.0f;
    }
    
    float angleRedRadian = (float) (3.14f * gRotateRedAngle / 180.0f);
    
    if (gbXPressed==true) {
        light_position[0] = 0.0f;
        light_position[1] = 100.0f * ((float) cos(angleRedRadian) / 2.0f - (float) sin(angleRedRadian) / 2.0f);
        light_position[2] = 100.0f * ((float) cos(angleRedRadian) / 2.0f + (float) sin(angleRedRadian) / 2.0f);
        light_position[3] = 1.0f;
    } else if (gbYPressed==true) {
        light_position[0] =100.0f * ((float) cos(angleRedRadian)/2.0f + (float) sin(angleRedRadian)/2.0f);
        light_position[1] =0.0f;
        light_position[2] =100.0f * ((float) cos(angleRedRadian)/2.0f - (float) sin(angleRedRadian)/2.0f);
        light_position[3] =1.0f;
    } else if (gbZPressed==true) {
        light_position[0] =100.0f * ((float) cos(angleRedRadian)/2.0f + (float) sin(angleRedRadian)/2.0f);
        light_position[1] =100.0f * ((float) cos(angleRedRadian)/2.0f - (float) sin(angleRedRadian)/2.0f);
        light_position[2] =0.0f;
        light_position[3] =1.0f;
    }
    
    
    
}


-(void) initShaders
{
    //create shader objects
    //vertex shader
    gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    
    //provide source code to shader
    const GLchar *vertexShaderSourceCodePV=
    "#version 410"\
    "\n"\
    "in vec4 vPosition;						"\
				"in vec3 vNormal;"\
				"uniform mat4 u_model_matrix;"\
				"uniform mat4 u_view_matrix;"\
				"uniform mat4 u_projection_matrix;"\
				"uniform int u_LKeyPressed;"\
				"uniform vec3 u_La;"\
				"uniform vec3 u_Ld;"\
				"uniform vec3 u_Ls;"\
				"uniform vec4 u_Lp;"\
				"uniform vec3 u_Ka;"\
				"uniform vec3 u_Kd;"\
				"uniform vec3 u_Ks;"\
				"uniform float u_KShininess;"\
				"out vec3 phong_ads_color;"\
				"void main(void)						"\
				"{										"\
				"if(u_LKeyPressed == 1)"\
				"{"\
				"vec4 eyeCoordinates = u_view_matrix * u_model_matrix * vPosition;"\
				"vec3 transformed_normals = normalize(mat3(u_view_matrix * u_model_matrix) * vNormal);"\
				"vec3 light_direction = normalize(vec3(u_Lp) - eyeCoordinates.xyz);"\
				"float tn_dot_ld=max(dot(transformed_normals, light_direction),0.0);"\
				"vec3 ambient = u_La * u_Ka;"\
				"vec3 diffuse = u_Ld * u_Kd * tn_dot_ld;"\
				"vec3 reflection_vector = reflect(-light_direction, transformed_normals);"\
				"vec3 viewer_vector = normalize(-eyeCoordinates.xyz);"\
				"vec3 specular = u_Ls * u_Ks * pow(max(dot(reflection_vector, viewer_vector),0.0), u_KShininess);"\
				"phong_ads_color=ambient+diffuse+specular;"\
				"}"\
				"else"\
				"{"\
				"phong_ads_color=vec3(1.0,1.0,1.0);"\
				"}"\
				"gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;	"\
				"}										";
    
    const GLchar *vertexShaderSourceCodePF=
    "#version 410"\
    "\n"\
    "in vec4 vPosition;						"\
				"in vec3 vNormal;"\
				"uniform mat4 u_model_matrix;"\
				"uniform mat4 u_view_matrix;"\
				"uniform mat4 u_projection_matrix;"\
				"uniform int u_LKeyPressed;"\
				"uniform vec3 u_Lp;"\
				"out vec3 transformed_normals;"\
				"out vec3 light_direction;"\
				"out vec3 viewer_vector;"\
				"void main(void)						"\
				"{										"\
				"if(u_LKeyPressed == 1)"\
				"{"\
				"vec4 eyeCoordinates = u_view_matrix * u_model_matrix * vPosition;"\
				"transformed_normals = mat3(u_view_matrix * u_model_matrix) * vNormal;"\
				"light_direction = vec3(u_Lp) - eyeCoordinates.xyz;"\
				"viewer_vector = -eyeCoordinates.xyz;"\
				"}"\
				"gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;	"\
				"}										";
    
    if(gbVKeyPressed==true){
        glShaderSource(gVertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCodePV,NULL);
    }else if(gbFKeyPressed==true){
        glShaderSource(gVertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCodePF,NULL);
    }
    
    
    //compile shader
    glCompileShader(gVertexShaderObject);
    
    //get compilation status and log the information
    GLint iInfoLogLength = 0;
    GLint iShaderCompiledStatus = 0;
    char* szInfoLog = NULL;
    
    //get compilation status
    glGetShaderiv(gVertexShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
    if(iShaderCompiledStatus==GL_FALSE)
    {
        //extract log information
        glGetShaderiv(gVertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0)
        {
            szInfoLog = (char *) malloc(iInfoLogLength);
            if(szInfoLog!=NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(gVertexShaderObject, iInfoLogLength, &written, szInfoLog);
                fprintf(gpFile, "Vertex Shader Compilation Log : %s\n", szInfoLog);
                
                free(szInfoLog);
                [self release];
                [NSApp terminate:self];
            }
        }
    }
    
    //fragment shader
    //re-initialize
    iInfoLogLength = 0;
    iShaderCompiledStatus = 0;
    szInfoLog = NULL;
    
    //create fragment shader object
    gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    
    //shader source code
    const GLchar* fragmentShaderSourceCodePV =
    "#version 410"\
    "\n"\
    "in vec3 phong_ads_color;"\
				"out vec4 FragColor;" \
				"void main(void)" \
				"{" \
				"FragColor=vec4(phong_ads_color, 1.0);" \
				"}";
    
    const GLchar* fragmentShaderSourceCodePF =
    "#version 410"\
    "\n"\
    "in vec3 transformed_normals;"\
				"in vec3 light_direction;"\
				"in vec3 viewer_vector;"\
				"out vec4 FragColor;" \
				"uniform vec3 u_La;"\
				"uniform vec3 u_Ld;"\
				"uniform vec3 u_Ls;"\
				"uniform vec3 u_Ka;"\
				"uniform vec3 u_Kd;"\
				"uniform vec3 u_Ks;"\
				"uniform float u_KShininess;"\
				"uniform int u_LKeyPressed;"
				"void main(void)" \
				"{" \
				"vec3 phong_ads_color;"\
				"if(u_LKeyPressed==1)"\
				"{"\
				"vec3 normalized_transformed_normals=normalize(transformed_normals);"\
				"vec3 normalized_light_direction=normalize(light_direction);"\
				"vec3 normalized_viewer_vector=normalize(viewer_vector);"\
				"vec3 ambient=u_La * u_Ka;"\
				"float tn_dot_id = max(dot(normalized_transformed_normals,normalized_light_direction),0.0);"\
				"vec3 diffuse = u_Ld * u_Kd * tn_dot_id;"\
				"vec3 reflection_vector=reflect(-normalized_light_direction, normalized_transformed_normals);"\
				"vec3 specular = u_Ls * u_Ks * pow(max(dot(reflection_vector, normalized_viewer_vector),0.0), u_KShininess);"\
				"phong_ads_color=ambient+diffuse+specular;"\
				"}"\
				"else"\
				"{"\
				"phong_ads_color=vec3(1.0,1.0,1.0);"\
				"}"\
				"FragColor=vec4(phong_ads_color, 1.0);" \
				"}";
    
    //attach source code
    gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    if(gbVKeyPressed==true){
        glShaderSource(gFragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCodePV,NULL);
    }else if(gbFKeyPressed==true){
        glShaderSource(gFragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCodePF,NULL);
    }
    
    //compile shader
    glCompileShader(gFragmentShaderObject);
    
    //get compilation status
    glGetShaderiv(gFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
    if(iShaderCompiledStatus==GL_FALSE)
    {
        //extract log information
        glGetShaderiv(gFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0)
        {
            szInfoLog = (char *) malloc(iInfoLogLength);
            if(szInfoLog!=NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(gFragmentShaderObject, iInfoLogLength, &written, szInfoLog);
                fprintf(gpFile, "Fragment Shader Compilation Log : %s\n", szInfoLog);
                
                free(szInfoLog);
                [self release];
                [NSApp terminate:self];
            }
        }
    }
    
    //shader program object
    //create shader program object
    gShaderProgramObject = glCreateProgram();
    
    //attach vertex shader
    glAttachShader(gShaderProgramObject, gVertexShaderObject);
    
    //attach fragment shader
    glAttachShader(gShaderProgramObject, gFragmentShaderObject);
    
    //pre-link binding of shader program object with vertex shader position attribute
    glBindAttribLocation(gShaderProgramObject, VDG_ATTRIBUTE_VERTEX, "vPosition");
    glBindAttribLocation(gShaderProgramObject, VDG_ATTRIBUTE_NORMAL, "vNormal");
    
    //link shader
    glLinkProgram(gShaderProgramObject);
    
    //get link status
    GLint iShaderProgramLinkStatus = 0;
    glGetProgramiv(gShaderProgramObject, GL_LINK_STATUS, &iShaderProgramLinkStatus);
    if(iShaderProgramLinkStatus==GL_FALSE)
    {
        //extract log information
        glGetProgramiv(gShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0)
        {
            szInfoLog = (char *) malloc(iInfoLogLength);
            if(szInfoLog!=NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(gShaderProgramObject, iInfoLogLength, &written, szInfoLog);
                fprintf(gpFile,  "Shader Program Link Log : %s\n", szInfoLog);
                
                free(szInfoLog);
                [self release];
                [NSApp terminate:self];
            }
        }
    }
    
    //get uniform locations
    gModelMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_model_matrix");
    gViewMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_view_matrix");
    gProjectionMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");
    
    gLKeyPressedUniform = glGetUniformLocation(gShaderProgramObject, "u_LKeyPressed");
    
    gLaUniform = glGetUniformLocation(gShaderProgramObject, "u_La");
    gLdUniform = glGetUniformLocation(gShaderProgramObject, "u_Ld");
    gLsUniform = glGetUniformLocation(gShaderProgramObject, "u_Ls");
    gLpUniform = glGetUniformLocation(gShaderProgramObject, "u_Lp");
    
    gKaUniform = glGetUniformLocation(gShaderProgramObject, "u_Ka");
    gKdUniform = glGetUniformLocation(gShaderProgramObject, "u_Kd");
    gKsUniform = glGetUniformLocation(gShaderProgramObject, "u_Ks");
    gKShininessUniform = glGetUniformLocation(gShaderProgramObject, "u_KShininess");
}

void getSphereVertxData(float radius, unsigned int slices, unsigned int stacks)
{
    int vertexCount = (slices + 1)*(stacks + 1);
    
    gSphereVertices.resize(3 * vertexCount);
    gSphereNormals.resize(3 * vertexCount);
    gSphereTexcoords.resize(2 * vertexCount);
    gSphereElements.resize(2 * slices * stacks * 3);
    
    std::vector<GLfloat>::iterator vt = gSphereVertices.begin();
    std::vector<GLfloat>::iterator nl = gSphereNormals.begin();
    std::vector<GLfloat>::iterator tc = gSphereTexcoords.begin();
    std::vector<GLushort>::iterator ic = gSphereElements.begin();
    
    float du = 2 * M_PI / slices;
    float dv = M_PI / stacks;
    float i, j, u, v, x, y, z;
    int indexV = 0;
    int indexT = 0;
    
    for (i = 0; i <= stacks; i++) {
        v = -M_PI / 2 + i * dv;
        for (j = 0; j <= slices; j++) {
            u = j * du;
            x = cos(u)*cos(v);
            y = sin(u)*cos(v);
            z = sin(v);
            vt[indexV] = radius * x;
            nl[indexV++] = x;
            vt[indexV] = radius * y;
            nl[indexV++] = y;
            vt[indexV] = radius * z;
            nl[indexV++] = z;
            tc[indexT++] = j / slices;
            tc[indexT++] = i / stacks;
        }
    }
    int k = 0;
    for (j = 0; j < stacks; j++) {
        int row1 = j * (slices + 1);
        int row2 = (j + 1)*(slices + 1);
        for (i = 0; i < slices; i++) {
            ic[k++] = row1 + i;
            ic[k++] = row2 + i + 1;
            ic[k++] = row2 + i;
            ic[k++] = row1 + i;
            ic[k++] = row1 + i + 1;
            ic[k++] = row2 + i + 1;
        }
    }
}

-(void)reshape
{
    //code
    CGLLockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
    
    NSRect rect=[self bounds];
    
    GLfloat width=rect.size.width;
    GLfloat height=rect.size.height;
    
    if(height==0)
        height=1;
    
    glViewport(0,0,(GLsizei)width, (GLsizei)height);
    
    //glOrtho(left, right, bottom, top, near, far)
    /*if(width <= height)
    {
        gOrthographicProjectionMatrix = vmath::ortho(-100.0f, 100.0f, (-100.0f * (height/width)), (100.0f * (height/width)), -100.0f, 100.0f);
    }else{
        gOrthographicProjectionMatrix = vmath::ortho((-100.0f * (width/height)), (100.0f * (width/height)), -100.0f, 100.0f, -100.0f, 100.0f);
    }*/
    gPerspectiveProjectionMatrix = vmath::perspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f);
    
    CGLUnlockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
}

-(void)drawRect:(NSRect)dirtyRect
{
    //code
    [self drawView];
}

-(void)drawView
{
    //code
    [[self openGLContext]makeCurrentContext];
    
    CGLLockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //rotateSphere();
    
    [self initShaders];
    
    [self rotate];
    
    //start using OpenGL program object
    glUseProgram(gShaderProgramObject);
    
    if (gLightToggle == 1) {
        glUniform1i(gLKeyPressedUniform, 1);
        
        glUniform3fv(gLaUniform, 1, light_ambient);
        glUniform3fv(gLdUniform, 1, light_defused);
        glUniform3fv(gLsUniform, 1, light_specular);
        glUniform3fv(gLpUniform, 1, light_position);
        
    }
    else {
        glUniform1i(gLKeyPressedUniform, 0);
    }
    
    //OpenGL drawing
    //set modelview & modelviewprojection matrices to identity
    vmath::mat4 modelMatrix = vmath::mat4::identity();
    vmath::mat4 viewMatrix = vmath::mat4::identity();
    
    //1st row
    glUniform3fv(gKaUniform, 1, s1_material_ambient);
    glUniform3fv(gKdUniform, 1, s1_material_diffuse);
    glUniform3fv(gKsUniform, 1, s1_material_specular);
    glUniform1f(gKShininessUniform, s1_material_shininess);
    //vmath::translate
    modelMatrix = vmath::translate(-3.75f, 3.0f, zAxis);
    
    //pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
    
    // *** bind vao ***
    glBindVertexArray(gVao_sphere);
    
    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    
    // *** unbind vao ***
    glBindVertexArray(0);
    
    
    glUniform3fv(gKaUniform, 1, s2_material_ambient);
    glUniform3fv(gKdUniform, 1, s2_material_diffuse);
    glUniform3fv(gKsUniform, 1, s2_material_specular);
    glUniform1f(gKShininessUniform, s2_material_shininess);
    //vmath::translate
    modelMatrix = vmath::mat4::identity();
    modelMatrix = vmath::translate(-1.25f, 3.0f, zAxis);
    
    //pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
    
    // *** bind vao ***
    glBindVertexArray(gVao_sphere);
    
    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    
    // *** unbind vao ***
    glBindVertexArray(0);
    
    
    glUniform3fv(gKaUniform, 1, s24_material_ambient);
    glUniform3fv(gKdUniform, 1, s24_material_diffuse);
    glUniform3fv(gKsUniform, 1, s24_material_specular);
    glUniform1f(gKShininessUniform, s2_material_shininess);
    //vmath::translate
    modelMatrix = vmath::mat4::identity();
    modelMatrix = vmath::translate(1.25f, 3.0f, zAxis);
    
    //pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
    
    // *** bind vao ***
    glBindVertexArray(gVao_sphere);
    
    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    
    // *** unbind vao ***
    glBindVertexArray(0);
    
    
    
    glUniform3fv(gKaUniform, 1, s3_material_ambient);
    glUniform3fv(gKdUniform, 1, s3_material_diffuse);
    glUniform3fv(gKsUniform, 1, s3_material_specular);
    glUniform1f(gKShininessUniform, s3_material_shininess);
    //vmath::translate
    modelMatrix = vmath::mat4::identity();
    modelMatrix = vmath::translate(3.75f, 3.0f, zAxis);
    
    //pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
    
    // *** bind vao ***
    glBindVertexArray(gVao_sphere);
    
    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    
    // *** unbind vao ***
    glBindVertexArray(0);
    
    
    
    
    
    
    //2nd row
    //vmath::translate
    glUniform3fv(gKaUniform, 1, s4_material_ambient);
    glUniform3fv(gKdUniform, 1, s4_material_diffuse);
    glUniform3fv(gKsUniform, 1, s4_material_specular);
    glUniform1f(gKShininessUniform, s4_material_shininess);
    modelMatrix = vmath::translate(-3.75f, 2.0f, zAxis);
    
    //pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
    
    // *** bind vao ***
    glBindVertexArray(gVao_sphere);
    
    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    
    // *** unbind vao ***
    glBindVertexArray(0);
    
    
    
    glUniform3fv(gKaUniform, 1, s5_material_ambient);
    glUniform3fv(gKdUniform, 1, s5_material_diffuse);
    glUniform3fv(gKsUniform, 1, s5_material_specular);
    glUniform1f(gKShininessUniform, s5_material_shininess);
    //vmath::translate
    modelMatrix = vmath::mat4::identity();
    modelMatrix = vmath::translate(-1.25f, 2.0f, zAxis);
    
    //pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
    
    // *** bind vao ***
    glBindVertexArray(gVao_sphere);
    
    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    
    // *** unbind vao ***
    glBindVertexArray(0);
    
    
    glUniform3fv(gKaUniform, 1, s6_material_ambient);
    glUniform3fv(gKdUniform, 1, s6_material_diffuse);
    glUniform3fv(gKsUniform, 1, s6_material_specular);
    glUniform1f(gKShininessUniform, s6_material_shininess);
    //vmath::translate
    modelMatrix = vmath::mat4::identity();
    modelMatrix = vmath::translate(1.25f, 2.0f, zAxis);
    
    //pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
    
    // *** bind vao ***
    glBindVertexArray(gVao_sphere);
    
    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    
    // *** unbind vao ***
    glBindVertexArray(0);
    
    
    //vmath::translate
    glUniform3fv(gKaUniform, 1, s7_material_ambient);
    glUniform3fv(gKdUniform, 1, s7_material_diffuse);
    glUniform3fv(gKsUniform, 1, s7_material_specular);
    glUniform1f(gKShininessUniform, s7_material_shininess);
    modelMatrix = vmath::mat4::identity();
    modelMatrix = vmath::translate(3.75f, 2.0f, zAxis);
    
    //pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
    
    // *** bind vao ***
    glBindVertexArray(gVao_sphere);
    
    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    
    // *** unbind vao ***
    glBindVertexArray(0);
    
    
    
    
    
    
    //3rd row
    glUniform3fv(gKaUniform, 1, s8_material_ambient);
    glUniform3fv(gKdUniform, 1, s8_material_diffuse);
    glUniform3fv(gKsUniform, 1, s8_material_specular);
    glUniform1f(gKShininessUniform, s8_material_shininess);
    //vmath::translate
    modelMatrix = vmath::translate(-3.75f, 1.0f, zAxis);
    
    //pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
    
    // *** bind vao ***
    glBindVertexArray(gVao_sphere);
    
    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    
    // *** unbind vao ***
    glBindVertexArray(0);
    
    
    //vmath::translate
    glUniform3fv(gKaUniform, 1, s9_material_ambient);
    glUniform3fv(gKdUniform, 1, s9_material_diffuse);
    glUniform3fv(gKsUniform, 1, s9_material_specular);
    glUniform1f(gKShininessUniform, s9_material_shininess);
    modelMatrix = vmath::mat4::identity();
    modelMatrix = vmath::translate(-1.25f, 1.0f, zAxis);
    
    //pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
    
    // *** bind vao ***
    glBindVertexArray(gVao_sphere);
    
    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    
    // *** unbind vao ***
    glBindVertexArray(0);
    
    
    glUniform3fv(gKaUniform, 1, s10_material_ambient);
    glUniform3fv(gKdUniform, 1, s10_material_diffuse);
    glUniform3fv(gKsUniform, 1, s10_material_specular);
    glUniform1f(gKShininessUniform, s10_material_shininess);
    //vmath::translate
    modelMatrix = vmath::mat4::identity();
    modelMatrix = vmath::translate(1.25f, 1.0f, zAxis);
    
    //pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
    
    // *** bind vao ***
    glBindVertexArray(gVao_sphere);
    
    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    
    // *** unbind vao ***
    glBindVertexArray(0);
    
    
    glUniform3fv(gKaUniform, 1, s11_material_ambient);
    glUniform3fv(gKdUniform, 1, s11_material_diffuse);
    glUniform3fv(gKsUniform, 1, s11_material_specular);
    glUniform1f(gKShininessUniform, s11_material_shininess);
    //vmath::translate
    modelMatrix = vmath::mat4::identity();
    modelMatrix = vmath::translate(3.75f, 1.0f, zAxis);
    
    //pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
    
    // *** bind vao ***
    glBindVertexArray(gVao_sphere);
    
    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    
    // *** unbind vao ***
    glBindVertexArray(0);
    
    
    
    
    glUniform3fv(gKaUniform, 1, s12_material_ambient);
    glUniform3fv(gKdUniform, 1, s12_material_diffuse);
    glUniform3fv(gKsUniform, 1, s12_material_specular);
    glUniform1f(gKShininessUniform, s12_material_shininess);
    //4th row
    //vmath::translate
    modelMatrix = vmath::translate(-3.75f, 0.0f, zAxis);
    
    //pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
    
    // *** bind vao ***
    glBindVertexArray(gVao_sphere);
    
    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    
    // *** unbind vao ***
    glBindVertexArray(0);
    
    
    glUniform3fv(gKaUniform, 1, s13_material_ambient);
    glUniform3fv(gKdUniform, 1, s13_material_diffuse);
    glUniform3fv(gKsUniform, 1, s13_material_specular);
    glUniform1f(gKShininessUniform, s13_material_shininess);
    //vmath::translate
    modelMatrix = vmath::mat4::identity();
    modelMatrix = vmath::translate(-1.25f, 0.0f, zAxis);
    
    //pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
    
    // *** bind vao ***
    glBindVertexArray(gVao_sphere);
    
    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    
    // *** unbind vao ***
    glBindVertexArray(0);
    
    
    glUniform3fv(gKaUniform, 1, s14_material_ambient);
    glUniform3fv(gKdUniform, 1, s14_material_diffuse);
    glUniform3fv(gKsUniform, 1, s14_material_specular);
    glUniform1f(gKShininessUniform, s14_material_shininess);
    //vmath::translate
    modelMatrix = vmath::mat4::identity();
    modelMatrix = vmath::translate(1.25f, 0.0f, zAxis);
    
    //pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
    
    // *** bind vao ***
    glBindVertexArray(gVao_sphere);
    
    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    
    // *** unbind vao ***
    glBindVertexArray(0);
    
    
    glUniform3fv(gKaUniform, 1, s15_material_ambient);
    glUniform3fv(gKdUniform, 1, s15_material_diffuse);
    glUniform3fv(gKsUniform, 1, s15_material_specular);
    glUniform1f(gKShininessUniform, s15_material_shininess);
    //vmath::translate
    modelMatrix = vmath::mat4::identity();
    modelMatrix = vmath::translate(3.75f, 0.0f, zAxis);
    
    //pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
    
    // *** bind vao ***
    glBindVertexArray(gVao_sphere);
    
    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    
    // *** unbind vao ***
    glBindVertexArray(0);
    
    
    
    
    glUniform3fv(gKaUniform, 1, s16_material_ambient);
    glUniform3fv(gKdUniform, 1, s16_material_diffuse);
    glUniform3fv(gKsUniform, 1, s16_material_specular);
    glUniform1f(gKShininessUniform, s16_material_shininess);
    //5th row
    //vmath::translate
    modelMatrix = vmath::translate(-3.75f, -1.0f, zAxis);
    
    //pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
    
    // *** bind vao ***
    glBindVertexArray(gVao_sphere);
    
    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    
    // *** unbind vao ***
    glBindVertexArray(0);
    
    
    glUniform3fv(gKaUniform, 1, s17_material_ambient);
    glUniform3fv(gKdUniform, 1, s17_material_diffuse);
    glUniform3fv(gKsUniform, 1, s17_material_specular);
    glUniform1f(gKShininessUniform, s17_material_shininess);
    //vmath::translate
    modelMatrix = vmath::mat4::identity();
    modelMatrix = vmath::translate(-1.25f, -1.0f, zAxis);
    
    //pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
    
    // *** bind vao ***
    glBindVertexArray(gVao_sphere);
    
    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    
    // *** unbind vao ***
    glBindVertexArray(0);
    
    
    glUniform3fv(gKaUniform, 1, s18_material_ambient);
    glUniform3fv(gKdUniform, 1, s18_material_diffuse);
    glUniform3fv(gKsUniform, 1, s18_material_specular);
    glUniform1f(gKShininessUniform, s18_material_shininess);
    //vmath::translate
    modelMatrix = vmath::mat4::identity();
    modelMatrix = vmath::translate(1.25f, -1.0f, zAxis);
    
    //pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
    
    // *** bind vao ***
    glBindVertexArray(gVao_sphere);
    
    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    
    // *** unbind vao ***
    glBindVertexArray(0);
    
    glUniform3fv(gKaUniform, 1, s19_material_ambient);
    glUniform3fv(gKdUniform, 1, s19_material_diffuse);
    glUniform3fv(gKsUniform, 1, s19_material_specular);
    glUniform1f(gKShininessUniform, s19_material_shininess);
    //vmath::translate
    modelMatrix = vmath::mat4::identity();
    modelMatrix = vmath::translate(3.75f, -1.0f, zAxis);
    
    //pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
    
    // *** bind vao ***
    glBindVertexArray(gVao_sphere);
    
    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    
    // *** unbind vao ***
    glBindVertexArray(0);
    
    
    
    
    glUniform3fv(gKaUniform, 1, s20_material_ambient);
    glUniform3fv(gKdUniform, 1, s20_material_diffuse);
    glUniform3fv(gKsUniform, 1, s20_material_specular);
    glUniform1f(gKShininessUniform, s20_material_shininess);
    //6th row
    //vmath::translate
    modelMatrix = vmath::translate(-3.75f, -2.0f, zAxis);
    
    //pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
    
    // *** bind vao ***
    glBindVertexArray(gVao_sphere);
    
    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    
    // *** unbind vao ***
    glBindVertexArray(0);
    
    
    glUniform3fv(gKaUniform, 1, s21_material_ambient);
    glUniform3fv(gKdUniform, 1, s21_material_diffuse);
    glUniform3fv(gKsUniform, 1, s21_material_specular);
    glUniform1f(gKShininessUniform, s21_material_shininess);
    //vmath::translate
    modelMatrix = vmath::mat4::identity();
    modelMatrix = vmath::translate(-1.25f, -2.0f, zAxis);
    
    //pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
    
    // *** bind vao ***
    glBindVertexArray(gVao_sphere);
    
    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    
    // *** unbind vao ***
    glBindVertexArray(0);
    
    
    glUniform3fv(gKaUniform, 1, s22_material_ambient);
    glUniform3fv(gKdUniform, 1, s22_material_diffuse);
    glUniform3fv(gKsUniform, 1, s22_material_specular);
    glUniform1f(gKShininessUniform, s22_material_shininess);
    //vmath::translate
    modelMatrix = vmath::mat4::identity();
    modelMatrix = vmath::translate(1.25f, -2.0f, zAxis);
    
    //pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
    
    // *** bind vao ***
    glBindVertexArray(gVao_sphere);
    
    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    
    // *** unbind vao ***
    glBindVertexArray(0);
    
    
    glUniform3fv(gKaUniform, 1, s23_material_ambient);
    glUniform3fv(gKdUniform, 1, s23_material_diffuse);
    glUniform3fv(gKsUniform, 1, s23_material_specular);
    glUniform1f(gKShininessUniform, s23_material_shininess);
    //vmath::translate
    modelMatrix = vmath::mat4::identity();
    modelMatrix = vmath::translate(3.75f, -2.0f, zAxis);
    
    //pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
    
    // *** bind vao ***
    glBindVertexArray(gVao_sphere);
    
    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    
    // *** unbind vao ***
    glBindVertexArray(0);
    
    glUseProgram(0);
    CGLFlushDrawable((CGLContextObj)[[self openGLContext]CGLContextObj]);
    CGLUnlockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
}

-(BOOL)acceptsFirstResponder
{
    //code
    [[self window]makeFirstResponder:self];
    return(YES);
}

-(void)keyDown:(NSEvent *)theEvent
{
    //code
    int key=(int)[[theEvent characters] characterAtIndex:0];
    
    switch(key)
    {
        case 27: //Escape
            [[self window]toggleFullScreen:self];
            break;
        case 'Q':
        case 'q':
            [self release];
            [NSApp terminate:self];
            break;
        case 'A':
        case 'a':
            animationToggle();
            break;
        case 'F':
        case'f':
            
                gbFKeyPressed=true;
                gbVKeyPressed=false;
            
            break;
        case 'V':
        case'v':
           
                gbVKeyPressed=true;
                gbFKeyPressed=false;
            
            break;
        case 'L':
        case 'l':
            fprintf(gpFile, "l/L key is pressed...%d\n",gLightToggle);
            lightToggle();
            break;
        case 'W':
        case 'w':
            gbXPressed=false;
            gbYPressed=false;
            gbZPressed=false;
            break;
        case 'X':
        case 'x':
            gbXPressed=true;
            gbYPressed=false;
            gbZPressed=false;
            break;
        case 'Y':
        case 'y':
            gbXPressed=false;
            gbYPressed=true;
            gbZPressed=false;
            break;
        case 'Z':
        case 'z':
            gbXPressed=false;
            gbYPressed=false;
            gbZPressed=true;
            break;
        default:
            break;
    }
}

void zoomInOut(short szDelta)
{
    if(szDelta > 0)
    {
        fovy += 3.0f;
    }else
    {
        fovy -= 3.0f;
    }
}

void animationToggle(void)
{
    gAnimateToggle++;
    
    if(gAnimateToggle > 1)
    {
        gAnimateToggle = 0;
    }
}

void lightToggle(void)
{
    gLightToggle++;
    if(gLightToggle > 1)
    {
        gLightToggle = 0;
    }
}

void rotateSphere(void)
{
    angleRotate = angleRotate + 5.0f;
    if(angleRotate>=360.0)
    {
        angleRotate = 0.0f;
    }
}

-(void)mouseDown:(NSEvent *)theEvent
{
    //code
}

-(void)mouseDragged:(NSEvent *)theEvent
{
    //code
}

-(void)rightMouseDown:(NSEvent *)theEvent
{
    //code
}

-(void) dealloc
{
    //code
    CVDisplayLinkStop(displayLink);
    
    //destroy vao
    if(gVao_sphere)
    {
        glDeleteVertexArrays(1, &gVao_sphere);
        gVao_sphere = 0;
    }
    
    if(gVboSphereNormal)
    {
        glDeleteBuffers(1, &gVboSphereNormal);
        gVboSphereNormal = 0;
    }
    
    if(gVboSpherePosition)
    {
        glDeleteBuffers(1, &gVboSpherePosition);
        gVboSpherePosition = 0;
    }
    
    if(gVbo_sphere_element)
    {
        glDeleteBuffers(1, &gVbo_sphere_element);
        gVbo_sphere_element = 0;
    }
    
    //detach vertex shader from shader program object
    glDetachShader(gShaderProgramObject, gVertexShaderObject);
    
    glDetachShader(gShaderProgramObject, gFragmentShaderObject);
    
    glDeleteShader(gVertexShaderObject);
    gVertexShaderObject=0;
    
    glDeleteShader(gFragmentShaderObject);
    gFragmentShaderObject=0;
    
    glDeleteProgram(gShaderProgramObject);
    gShaderProgramObject=0;
    
    
    CVDisplayLinkRelease(displayLink);
    
    [super dealloc];
}
@end

CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *pNow, const CVTimeStamp *pOutputTime, CVOptionFlags flagsIn, CVOptionFlags *pFlagsOut, void *pDisplayLinkContext)
{
    CVReturn result = [(GLView *)pDisplayLinkContext getFrameForTime:pOutputTime];
    return(result);
}
