//headers
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#import <QuartzCore/CVDisplayLink.h>

#import <OpenGL/gl3.h>
#import <OpenGL/gl3ext.h>
#import "vmath.h"

//'C' style global function declaration
CVReturn MyDisplayLinkCallback(CVDisplayLinkRef, const CVTimeStamp *,const CVTimeStamp *,CVOptionFlags,CVOptionFlags *,void *);

//global variables
FILE *gpFile=NULL;

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
    GLuint gVao_pyramid;
    GLuint gVbo_pyramid_position;
    GLuint gVbo_pyramid_normal;

    
    //uniforms
    GLuint gModelMatrixUniform, gViewMatrixUniform, gProjectionMatrixUniform;
    
    GLuint gL0aUniform;
    GLuint gL0dUniform;
    GLuint gL0sUniform;
    GLuint gL0pUniform;
    
    GLuint gL1aUniform;
    GLuint gL1dUniform;
    GLuint gL1sUniform;
    GLuint gL1pUniform;
    
    GLuint gKa0Uniform;
    GLuint gKd0Uniform;
    GLuint gKs0Uniform;
    
    GLuint gKa1Uniform;
    GLuint gKd1Uniform;
    GLuint gKs1Uniform;
    
    GLuint gKShininessUniform;
    GLuint gLKeyPressedUniform;
    
    GLfloat gAnglePyramid;
    
    bool gbAnimate;
    bool gbLight;
    
    //light0
    GLfloat light0_ambient[4];
    GLfloat light0_defused[4];
    GLfloat light0_specular[4];
    GLfloat light0_position[4];
    
    
    //light1
    GLfloat light1_ambient[4];
    GLfloat light1_defused[4];
    GLfloat light1_specular[4];
    GLfloat light1_position[4];
    
    //material
    GLfloat material_ambient[4];
    GLfloat material_defused[4];
    GLfloat material_specular[4];
    GLfloat material_shininess;
    
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
    gAngleCube=0.0f;
    gbAnimate = false;
    gbLight = false;
    
    //light0
    light0_ambient[]={0.0f,0.0f,0.0f,0.0f};
    light0_defused[]={1.0f,0.0f,0.0f,0.0f};
    light0_specular[]={1.0f,0.0f,0.0f,0.0f};
    light0_position[]={2.0f,1.0f,1.0f,0.0f};
    
    
    //light1
    light1_ambient[]={0.0f,0.0f,0.0f,0.0f};
    light1_defused[]={0.0f,0.0f,1.0f,0.0f};
    light1_specular[]={0.0f,0.0f,1.0f,0.0f};
    light1_position[]={-2.0f,1.0f,1.0f,0.0f};
    
    //material
    material_ambient[]={0.0f,0.0f,0.0f,0.0f};
    material_defused[]={1.0f,1.0f,1.0f,1.0f};
    material_specular[]={1.0f,1.0f,1.0f,1.0f};
    material_shininess=50.0f;
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
				"\n										"\
				"in vec4 vPosition;						"\
				"in vec3 vNormal;"\
				"uniform mat4 u_model_matrix;"\
				"uniform mat4 u_view_matrix;"\
				"uniform mat4 u_projection_matrix;"\
				"uniform int u_LKeyPressed;"\
				"uniform vec3 u_L0a;"\
				"uniform vec3 u_L0d;"\
				"uniform vec3 u_L0s;"\
				"uniform vec4 u_L0p;"\
				"uniform vec3 u_L1a;"\
				"uniform vec3 u_L1d;"\
				"uniform vec3 u_L1s;"\
				"uniform vec4 u_L1p;"\
				"uniform vec3 u_Ka0;"\
				"uniform vec3 u_Kd0;"\
				"uniform vec3 u_Ks0;"\
				"uniform vec3 u_Ka1;"\
				"uniform vec3 u_Kd1;"\
				"uniform vec3 u_Ks1;"\
				"uniform float u_KShininess;"\
				"out vec3 phong_ads_color;"\
				"void main(void)						"\
				"{										"\
				"if(u_LKeyPressed == 1)"\
				"{"\
				"vec4 eyeCoordinates = u_view_matrix * u_model_matrix * vPosition;"\
				"vec3 transformed_normals = normalize(mat3(u_view_matrix * u_model_matrix) * vNormal);"\
				"vec3 light0_direction = normalize(vec3(u_L0p) - eyeCoordinates.xyz);"\
				"float tn_dot_ld0=max(dot(transformed_normals, light0_direction),0.0);"\
				"vec3 ambient0 = u_L0a * u_Ka0;"\
				"vec3 diffuse0 = u_L0d * u_Kd0 * tn_dot_ld0;"\
				"vec3 reflection_vector0 = reflect(-light0_direction, transformed_normals);"\
				"vec3 viewer_vector = normalize(-eyeCoordinates.xyz);"\
				"vec3 specular0 = u_L0s * u_Ks0 * pow(max(dot(reflection_vector0, viewer_vector),0.0), u_KShininess);"\
				"vec3 light_direction1 = normalize(vec3(u_L1p) - eyeCoordinates.xyz);"\
				"float tn_dot_ld1=max(dot(transformed_normals, light_direction1),0.0);"\
				"vec3 ambient1 = u_L1a * u_Ka1;"\
				"vec3 diffuse1 = u_L1d * u_Kd1 * tn_dot_ld1;"\
				"vec3 reflection_vector1 = reflect(-light_direction1, transformed_normals);"\
				"vec3 specular1 = u_L1s * u_Ks1 * pow(max(dot(reflection_vector1, viewer_vector),0.0), u_KShininess);"\
				"phong_ads_color=(ambient0)+(diffuse0)+(specular0)+ambient1+diffuse1+specular1;"\
				"}"\
				"else"\
				"{"\
				"phong_ads_color=vec3(1.0,1.0,1.0);"\
				"}"\
				"gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;	"\
				"}										";
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
    "\n" \
				"in vec3 phong_ads_color;"\
				"out vec4 FragColor;" \
				"void main(void)" \
				"{" \
				"FragColor=vec4(phong_ads_color, 1.0);" \
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
    
    //get MVP uniform location
    gModelMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_model_matrix");
    gViewMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_view_matrix");
    gProjectionMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");
    
    gLKeyPressedUniform = glGetUniformLocation(gShaderProgramObject, "u_LKeyPressed");
    
    gL0aUniform = glGetUniformLocation(gShaderProgramObject, "u_L0a");
    gL0dUniform = glGetUniformLocation(gShaderProgramObject, "u_L0d");
    gL0sUniform = glGetUniformLocation(gShaderProgramObject, "u_L0s");
    gL0pUniform = glGetUniformLocation(gShaderProgramObject, "u_L0p");
    
    gKa0Uniform = glGetUniformLocation(gShaderProgramObject, "u_Ka0");
    gKd0Uniform = glGetUniformLocation(gShaderProgramObject, "u_Kd0");
    gKs0Uniform = glGetUniformLocation(gShaderProgramObject, "u_Ks0");
    
    gL1aUniform = glGetUniformLocation(gShaderProgramObject, "u_L1a");
    gL1dUniform = glGetUniformLocation(gShaderProgramObject, "u_L1d");
    gL1sUniform = glGetUniformLocation(gShaderProgramObject, "u_L1s");
    gL1pUniform = glGetUniformLocation(gShaderProgramObject, "u_L1p");
    
    gKa1Uniform = glGetUniformLocation(gShaderProgramObject, "u_Ka1");
    gKd1Uniform = glGetUniformLocation(gShaderProgramObject, "u_Kd1");
    gKs1Uniform = glGetUniformLocation(gShaderProgramObject, "u_Ks1");
    
    gKShininessUniform = glGetUniformLocation(gShaderProgramObject, "u_KShininess");
    
    
    //vertices, colors, shader attribs, vbo, vao initialization
    const GLfloat pyramidPosition[] =
    {
        0, 1, 0,
        -1, -1, 1,
        1, -1, 1,
        
        0, 1, 0,
        1, -1, 1,
        1, -1, -1,
        
        0, 1, 0,
        1, -1, -1,
        -1, -1, -1,
        
        0, 1, 0,
        -1, -1, -1,
        -1, -1, 1
    };
    
    
    const GLfloat pyramidNoramals[] =
    {
        0.0f,0.447214f,0.894427f,
        0.0f,0.447214f,0.894427f,
        0.0f,0.447214f,0.894427f,
        
        0.894427f,0.447214f,0.0f,
        0.894427f,0.447214f,0.0f,
        0.894427f,0.447214f,0.0f,
        
        0.0f,0.447214f,-0.0894427f,
        0.0f,0.447214f,-0.0894427f,
        0.0f,0.447214f,-0.0894427f,
        
        -0.894427f,0.447214f,0.0f,
        -0.894427f,0.447214f,0.0f,
        -0.894427f,0.447214f,0.0f
    };
    //configure vao
    glGenVertexArrays(1, &gVao_cube);
    glBindVertexArray(gVao_cube);
    
    glGenBuffers(1, &gVbo_pyramid_position);
    glBindBuffer(GL_ARRAY_BUFFER, gVbo_pyramid_position);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidPosition), pyramidPosition, GL_STATIC_DRAW);
    glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glGenBuffers(1, &gVbo_pyramid_normal);
    glBindBuffer(GL_ARRAY_BUFFER, gVbo_pyramid_normal);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidNoramals), pyramidNoramals, GL_STATIC_DRAW);
    glVertexAttribPointer(VDG_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(VDG_ATTRIBUTE_NORMAL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glBindVertexArray(0);
    
    //
    glClearDepth(1.0f);
    
    //enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    //depth test to do
    glDepthFunc(GL_LEQUAL);
    
    //we will always cull back faces for better performance
    glEnable(GL_CULL_FACE);
    
    //set background color
    glClearColor(0.1f, 0.2f, 0.3f, 0.0f); //blue
    
    gPerspectiveProjectionMatrix = vmath::mat4::identity();
    
    //video refresh rate linking
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, self);
    CGLContextObj cglContext=(CGLContextObj) [[self openGLContext]CGLContextObj];
    CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj) [[self pixelFormat]CGLPixelFormatObj];
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
    CVDisplayLinkStart(displayLink);
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
    
    [self updateAngle];
    
    CGLLockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //start using OpenGL program object
    glUseProgram(gShaderProgramObject);
    
    if(gbLight==true)
    {
        glUniform1i(gLKeyPressedUniform, 1);
        glUniform3fv(gL0aUniform, 1, light0_ambient);
        glUniform3fv(gL0dUniform, 1, light0_defused);
        glUniform3fv(gL0sUniform, 1, light0_specular);
        glUniform3fv(gL0pUniform, 1, light0_position);
        
        glUniform3fv(gKa0Uniform, 1, material_ambient);
        glUniform3fv(gKd0Uniform, 1, material_defused);
        glUniform3fv(gKs0Uniform, 1, material_specular);
        
        glUniform3fv(gL1aUniform, 1, light1_ambient);
        glUniform3fv(gL1dUniform, 1, light1_defused);
        glUniform3fv(gL1sUniform, 1, light1_specular);
        glUniform3fv(gL1pUniform, 1, light1_position);
        
        glUniform3fv(gKa1Uniform, 1, material_ambient);
        glUniform3fv(gKd1Uniform, 1, material_defused);
        glUniform3fv(gKs1Uniform, 1, material_specular);
        
        glUniform1f(gKShininessUniform, material_shininess);
    }else
    {
        glUniform1i(gLKeyPressedUniform,0);
    }
    
    //OpenGL Drawing
    //set modelview & modelviewprojection matrices to identity
    vmath::mat4 modelMatrix = vmath::mat4::identity();
    vmath::mat4 viewMatrix = vmath::mat4::identity();
    vmath::mat4 rotationMatrix=vmath::mat4::identity();
    //translate
    modelMatrix = translate(0.0f,0.0f,-5.0f);
    rotationMatrix = vmath::rotate(gfAnglePyra,0.0f,1.0f,0.0f);
    modelMatrix = modelMatrix * rotationMatrix;
    //pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
    
    ///cube vao binding
    glBindVertexArray(gVao_pyramid);
    
    glDrawArrays(GL_TRIANGLES, 0, 12);
    glBindVertexArray(0);

    
    glUseProgram(0);
    CGLFlushDrawable((CGLContextObj)[[self openGLContext]CGLContextObj]);
    CGLUnlockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
}

-(void)updateAngle
{
    gAngleCube = gAngleCube+0.4f;
    
    if(gAngleCube>=360.0f)
    {
        gAngleCube = gAngleCube - 360.0f;
    }
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
            
        case 'A':
        case 'a':
            if(gbAnimate==true)
            {
                gbAnimate = false;
            }else
            {
                gbAnimate=true;
            }
            break;
            
        case 'L':
        case 'l':
            if(gbLight==true)
            {
                gbLight=false;
            }else
            {
                gbLight=true;
            }
            
            break;
        default:
            break;
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
    if(gVao_pyramid)
    {
        glDeleteVertexArrays(1, &gVao_pyramid);
        gVao_pyramid = 0;
    }
    
    if(gVbo_pyramid_position)
    {
        glDeleteBuffers(1, &gVbo_pyramid_position);
        gVbo_pyramid_position = 0;
    }
    
    if(gVbo_pyramid_normal)
    {
        glDeleteBuffers(1, &gVbo_pyramid_normal);
        gVbo_pyramid_normal = 0;
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
