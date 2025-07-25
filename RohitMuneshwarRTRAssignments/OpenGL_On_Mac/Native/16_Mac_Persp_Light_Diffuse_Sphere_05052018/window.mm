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
    GLuint gVaoSphere;
    GLuint gVboSpherePosition;
    GLuint gVboSphereNormal;
    GLuint gVboSphereElement;
    
    //uniforms
    GLuint gModelViewUniform, gProjectionMatrixUniform;
    GLuint ldUniform, kdUniform, lightPositionUniform;
    GLuint lightToggleUniform;
    
    
    //projection matricex
    vmath::mat4 gPerspectiveProjectionMatrix;
    
    
    
    
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
    
    //create shader objects
    //vertex shader
    gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    
    //provide source code to shader
    const GLchar *vertexShaderSourceCode=
    "#version 410"\
    "\n"\
    "in vec4 vPosition;"\
    "in vec3 vNormal;"\
    "uniform mat4 u_model_view_matrix;"\
    "uniform mat4 u_projection_matrix;"\
    "uniform mediump int u_light_toggle_uniform;"\
    "uniform vec3 u_Ld;"\
    "uniform vec3 u_Kd;"\
    "uniform vec4 u_light_position;"\
    "out vec3 out_diffuse_light;"\
    "void main(void)"\
    "{"\
    "if(u_light_toggle_uniform==1)"\
    "{"\
    "vec4 eyeCoordinates = u_model_view_matrix * vPosition;"\
    "vec3 tnorm = normalize(mat3(u_model_view_matrix) * vNormal);"\
    "vec3 s = normalize(vec3(u_light_position - eyeCoordinates));"\
    "out_diffuse_light = u_Ld * u_Kd * max(dot(s, tnorm), 0.0);"\
    "}"\
    "gl_Position=u_projection_matrix * u_model_view_matrix * vPosition;"\
    "}";
    
    //attach shader source to above vertex shader object
    glShaderSource(gVertexShaderObject, //handle of the shader object whose source code has to be replaced
                   1, //number of elements in next two arrays
                   (const GLchar **) &vertexShaderSourceCode, //array of pointers to string containing the source code to be loaded into the shader
                   NULL //array of string lengths
                   );
    
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
    const GLchar* fragmentShaderSourceCode =
    "#version 410"\
    "\n"\
    "in vec3 out_diffuse_light;"\
    "out vec4 FragColor;"\
    "uniform int u_light_toggle_uniform;"\
    "void main(void)"\
    "{"\
    "if(u_light_toggle_uniform==1)"\
    "{"\
    "FragColor = vec4(out_diffuse_light,1.0);"\
    "}"\
    "else"\
    "{"\
    "FragColor = vec4(1.0,1.0,1.0,1.0);"\
    "}"\
    "}";
    
    //attach source code
    glShaderSource(gFragmentShaderObject, 1, (const GLchar **) &fragmentShaderSourceCode, NULL);
    
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
    gModelViewUniform = glGetUniformLocation(gShaderProgramObject, "u_model_view_matrix");
    gProjectionMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");
    lightToggleUniform = glGetUniformLocation(gShaderProgramObject, "u_light_toggle_uniform");
    ldUniform = glGetUniformLocation(gShaderProgramObject, "u_Ld");
    kdUniform = glGetUniformLocation(gShaderProgramObject, "u_Kd");
    lightPositionUniform = glGetUniformLocation(gShaderProgramObject, "u_light_position");
    
    //vertices, colors, shader attribs, vbo, vao initialization
    getSphereVertxData(0.5, 64, 32);
    
    std::vector<GLfloat>::iterator v = gSphereVertices.begin();
    std::vector<GLfloat>::iterator n = gSphereNormals.begin();
    std::vector<GLfloat>::iterator t = gSphereTexcoords.begin();
    std::vector<GLushort>::iterator i = gSphereElements.begin();
    
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
    glGenVertexArrays(1, &gVaoSphere);
    glBindVertexArray(gVaoSphere);
    
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
    
    glGenBuffers(1, &gVboSphereElement);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVboSphereElement);
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
    
    //video refresh rate linking
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, self);
    CGLContextObj cglContext=(CGLContextObj) [[self openGLContext]CGLContextObj];
    CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj) [[self pixelFormat]CGLPixelFormatObj];
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
    CVDisplayLinkStart(displayLink);
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
    
    //start using OpenGL program object
    glUseProgram(gShaderProgramObject);
    
    if (gLightToggle == 1) {
        glUniform1i(lightToggleUniform, 1);
        glUniform3f(ldUniform, 1.0f, 1.0f, 1.0f); //diffuse intensity of light
        glUniform3f(kdUniform, 0.5f, 0.5f, 0.5f); //diffuse reflectivity of Material
        GLfloat lightPosition[] = { 0.0f,0.0f,2.0f,1.0f };
        glUniform4fv(lightPositionUniform, 1, lightPosition); //light position
        
    }
    else {
        glUniform1i(lightToggleUniform, 0);
    }
    
    //OpenGL Drawing
    //set modelview and projection matrices to identity
    vmath::mat4 modelViewMatrix = vmath::mat4::identity();
    vmath::mat4 modelViewProjectionMatrix = vmath::mat4::identity();
    
    //translate
    modelViewMatrix = vmath::translate(0.0f,0.0f,-3.0f);
    
    //multiply the modelview and orthographic matrix to get modelViewProjectionMatrix
    //order is important
    modelViewProjectionMatrix = gPerspectiveProjectionMatrix;
    
    glUniformMatrix4fv(gModelViewUniform, 1, GL_FALSE, modelViewMatrix);
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, modelViewProjectionMatrix);
    
    //bind vao
    glBindVertexArray(gVaoSphere);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVboSphereElement);
    glDrawElements(GL_TRIANGLES, gSphereElements.size(), GL_UNSIGNED_SHORT, 0);
    
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
            [self release];
            [NSApp terminate:self];
            break;
        case 'F':
        case'f':
            [[self window]toggleFullScreen:self];
            break;
        case 'L':
        case 'l':
            fprintf(gpFile, "l/L key is pressed...%d\n",gLightToggle);
            lightToggle();
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
    if(gVaoSphere)
    {
        glDeleteVertexArrays(1, &gVaoSphere);
        gVaoSphere = 0;
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
