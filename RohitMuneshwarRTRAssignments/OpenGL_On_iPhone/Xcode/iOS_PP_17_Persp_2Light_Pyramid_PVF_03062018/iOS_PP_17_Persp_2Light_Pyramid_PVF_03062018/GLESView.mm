//
//  GLESView.m
//  iOS_PP_17_Persp_2Light_Pyramid_PVF_03062018
//
//  Created by rohit muneshwar on 24/06/18.
//
//

#import "OpenGLES/ES3/gl.h"
#import "OpenGLES/ES3/glext.h"
#import "GLESView.h"
#include "vmath.h"
#include <vector>

enum
{
    VDG_ATTRIBUTE_VERTEX = 0,
    VDG_ATTRIBUTE_COLOR = 1,
    VDG_ATTRIBUTE_NORMAL = 2,
    VDG_ATTRIBUTE_TEXTURE0 = 3
};

std::vector<GLfloat> gSphereVertices;
std::vector<GLfloat> gSphereNormals;
std::vector<GLfloat> gSphereTexcoords;
std::vector<GLushort> gSphereElements;

BOOL gAnimateToggle, gLightToggle;

//light0
GLfloat light0_ambient[]={0.0f,0.0f,0.0f,0.0f};
GLfloat light0_defused[]={1.0f,0.0f,0.0f,0.0f};
GLfloat light0_specular[]={1.0f,0.0f,0.0f,0.0f};
GLfloat light0_position[]={2.0f,1.0f,1.0f,0.0f};


//light1
GLfloat light1_ambient[]={0.0f,0.0f,0.0f,0.0f};
GLfloat light1_defused[]={0.0f,0.0f,1.0f,0.0f};
GLfloat light1_specular[]={0.0f,0.0f,1.0f,0.0f};
GLfloat light1_position[]={-2.0f,1.0f,1.0f,0.0f};

//material
GLfloat material_ambient[]={0.0f,0.0f,0.0f,0.0f};
GLfloat material_defused[]={1.0f,1.0f,1.0f,1.0f};
GLfloat material_specular[]={1.0f,1.0f,1.0f,1.0f};
GLfloat material_shininess=50.0f;

GLfloat gfAnglePyra = 0.0f;


@implementation GLESView
{
    EAGLContext *eaglContext;
    
    GLuint defaultFramebuffer;
    GLuint colorRenderbuffer;
    GLuint depthRenderbuffer;
    
    id displayLink;
    NSInteger animationFrameInterval;
    BOOL isAnimating;
    
    //shader objects
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
    
    GLuint singleTapCount;
    
    
    //projection matrix
    vmath::mat4 gPerspectiveProjectionMatrix;
}


-(id) initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    
    if(self)
    {
        //initialization code here
        
        CAEAGLLayer *eaglLayer =(CAEAGLLayer *) super.layer;
        eaglLayer.opaque=YES;
        eaglLayer.drawableProperties=[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
        
        eaglContext=[[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
        
        if(eaglContext==nil)
        {
            [self release];
            return(nil);
        }
        
        [EAGLContext setCurrentContext:eaglContext];
        
        glGenFramebuffers(1, &defaultFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
        
        glGenRenderbuffers(1, &colorRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
        
        [eaglContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer];
        
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer);
        
        GLint backingWidht;
        GLint backingHeight;
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidht);
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);
        
        glGenRenderbuffers(1, &depthRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, backingWidht, backingHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
        
        
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            printf("Failed To Create Complete Framebuffer Object %x\n",glCheckFramebufferStatus(GL_FRAMEBUFFER));
            glDeleteFramebuffers(1, &defaultFramebuffer);
            glDeleteRenderbuffers(1, &colorRenderbuffer);
            glDeleteRenderbuffers(1, &depthRenderbuffer);
            
            return(nil);
        }
        
        printf("Renderer : %s | GL_VERSION : %s | GLSL Version : %s\n", glGetString(GL_RENDERER), glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
        
        
        //hard coded initializations
        
        isAnimating=NO;
        animationFrameInterval=60; //default since iOS 8.2
        
        
        //vertices, colors, shader attribs, vbo, vao initialization
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
        glGenVertexArrays(1, &gVao_pyramid);
        glBindVertexArray(gVao_pyramid);
        
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
        glClearDepthf(1.0f);
        
        //enable depth testing
        glEnable(GL_DEPTH_TEST);
        
        //depth test to do
        glDepthFunc(GL_LEQUAL);
        
        //we will always cull back faces for better performance
        glEnable(GL_CULL_FACE);
        
        
        //clear color
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f); //blue
        
        //set projection matrix to identity matrix
        gPerspectiveProjectionMatrix = vmath::mat4::identity();
        
        gAnimateToggle = true;
        gLightToggle = true;
        
        //by default is per vertex
        singleTapCount = 1;
        
        
        
        //GESTURE RECOGNITION
        //tap gesture code
        UITapGestureRecognizer *singleTapGestureRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(onSingleTap:)];
        
        [singleTapGestureRecognizer setNumberOfTapsRequired:1];
        
        [singleTapGestureRecognizer setNumberOfTouchesRequired:1]; //touch of 1 finger
        
        [singleTapGestureRecognizer setDelegate:self];
        
        [self addGestureRecognizer:singleTapGestureRecognizer];
        
        //double tap gesture recognizer
        UITapGestureRecognizer *doubleTapGestureRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(onDoubleTap:)];
        
        [doubleTapGestureRecognizer setNumberOfTapsRequired:2];
        
        [doubleTapGestureRecognizer setNumberOfTouchesRequired:1];
        
        [doubleTapGestureRecognizer setDelegate:self];
        
        [self addGestureRecognizer:doubleTapGestureRecognizer];
        
        
        //this will allow us to differentiate between single tap and double tap
        [singleTapGestureRecognizer requireGestureRecognizerToFail:doubleTapGestureRecognizer];
        
        //swipe gesture
        UISwipeGestureRecognizer *swipeGestureRecognizer = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(onSwipe:)];
        
        [self addGestureRecognizer:swipeGestureRecognizer];
        
        //long-press gesture
        UILongPressGestureRecognizer *longPressGestureRecognizer = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(onLongPress:)];
        
        [self addGestureRecognizer:longPressGestureRecognizer];
        
        
    }
    return(self);
}

-(void) initShaders
{
    //create shader objects
    //vertex shader
    gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    
    //provide source code to shader
    const GLchar *vertexShaderSourceCodePV=
    "#version 300 es"\
    "\n"\
    "in vec4 vPosition;						"\
				"in vec3 vNormal;"\
				"uniform mat4 u_model_matrix;"\
				"uniform mat4 u_view_matrix;"\
				"uniform mat4 u_projection_matrix;"\
				"uniform mediump int u_LKeyPressed;"\
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
    
    const GLchar *vertexShaderSourceCodePF=
    "#version 300 es"\
    "\n"\
    "in vec4 vPosition;						"\
				"in vec3 vNormal;"\
				"uniform mat4 u_model_matrix;"\
				"uniform mat4 u_view_matrix;"\
				"uniform mat4 u_projection_matrix;"\
				"uniform mediump int u_LKeyPressed;"\
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
    
    if(singleTapCount==1){
        glShaderSource(gVertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCodePV,NULL);
    }else if(singleTapCount==2){
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
                printf("Vertex Shader Compilation Log : %s\n", szInfoLog);
                
                free(szInfoLog);
                [self release];
                //[NSApp terminate:self];
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
    "#version 300 es"\
    "\n"\
    "precision highp float;"\
    "in vec3 phong_ads_color;"\
				"out vec4 FragColor;" \
				"void main(void)" \
				"{" \
				"FragColor=vec4(phong_ads_color, 1.0);" \
				"}";
    
    const GLchar* fragmentShaderSourceCodePF =
    "#version 300 es"\
    "\n"\
    "precision highp float;"\
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
				"uniform mediump int u_LKeyPressed;"
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
    if(singleTapCount==1){
        glShaderSource(gFragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCodePV,NULL);
    }else if(singleTapCount==2){
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
                printf("Fragment Shader Compilation Log : %s\n", szInfoLog);
                
                free(szInfoLog);
                [self release];
                //[NSApp terminate:self];
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
                printf( "Shader Program Link Log : %s\n", szInfoLog);
                
                free(szInfoLog);
                [self release];
                //[NSApp terminate:self];
            }
        }
    }
    
    //get uniform locations
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

}



// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
/*
 - (void)drawRect:(CGRect)rect {
 // Drawing code
 
 //black background
 UIColor *fillColor = [UIColor blackColor];
 [fillColor set];
 UIRectFill(rect);
 
 //dictionary with kvc
 NSDictionary *dictionaryForTextAttributes = [NSDictionary dictionaryWithObjectsAndKeys:[UIFont fontWithName:@"Helvetica" size:24], NSFontAttributeName, [UIColor greenColor], NSForegroundColorAttributeName, nil];
 
 CGSize textSize=[centralText sizeWithAttributes:dictionaryForTextAttributes];
 
 CGPoint point;
 point.x = (rect.size.width/2) - (textSize.width/2);
 point.y = (rect.size.height/2) - (textSize.height/2) + 12; //12 for doc size
 
 [centralText drawAtPoint:point withAttributes:dictionaryForTextAttributes];
 }*/

+(Class)layerClass
{
    //code
    return([CAEAGLLayer class]);
}

-(void) drawView:(id)sender
{
    //code
    [EAGLContext setCurrentContext:eaglContext];
    
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
    
    [self initShaders];
    
    [self updateAngle];
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    //start using OpenGL program object
    glUseProgram(gShaderProgramObject);
    
    if (gLightToggle == 1) {
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
        
    }
    else {
        glUniform1i(gLKeyPressedUniform, 0);
    }
    
    //OpenGL Drawing
    //set modelview & modelviewprojection matrices to identity
    vmath::mat4 modelMatrix = vmath::mat4::identity();
    vmath::mat4 viewMatrix = vmath::mat4::identity();
    vmath::mat4 rotationMatrix=vmath::mat4::identity();
    //translate
    modelMatrix = vmath::translate(0.0f,0.0f,-8.0f);
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
    
    
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    [eaglContext presentRenderbuffer:GL_RENDERBUFFER];
}


-(void) layoutSubviews
{
    //code
    GLint width;
    GLint height;
    
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    [eaglContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer *)self.layer];
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
    
    glGenRenderbuffers(1, &depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
    
    glViewport(0, 0, width, height);
    
    /*
     //glOrtho(left, right, bottom, top, near, far)
     if(width <= height)
     {
     gOrthographicProjectionMatrix = vmath::ortho(-100.0f, 100.0f, (-100.0f * (height/width)), (100.0f * (height/width)), -100.0f, 100.0f);
     }else{
     gOrthographicProjectionMatrix = vmath::ortho((-100.0f * (width/height)), (100.0f * (width/height)), -100.0f, 100.0f, -100.0f, 100.0f);
     }*/
    gPerspectiveProjectionMatrix = vmath::perspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f);
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)
    {
        printf("Failed To Create Complete Framebuffer Object %x\n", glCheckFramebufferStatus(GL_FRAMEBUFFER));
        [self drawView:nil];
    }
}

-(void) startAnimation
{
    if(!isAnimating)
    {
        displayLink = [NSClassFromString(@"CADisplayLink")displayLinkWithTarget:self selector:@selector(drawView:)];
        [displayLink setPreferredFramesPerSecond:animationFrameInterval];
        [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        
        isAnimating=YES;
    }
}

-(void)updateAngle
{
    gfAnglePyra = gfAnglePyra+0.4f;
    
    if(gfAnglePyra>=360.0f)
    {
        gfAnglePyra = gfAnglePyra - 360.0f;
    }
}


-(void)stopAnimation
{
    if(isAnimating)
    {
        [displayLink invalidate];
        displayLink=nil;
        isAnimating=NO;
    }
}

//to become first responder
-(BOOL) acceptsFirstResponder
{
    //code
    return(YES);
}

//like keydown
-(void) touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    //code
    /*
     centralText = @"'touchesBegan' Event Occured";
     [self setNeedsDisplay]; //repainting
     */
}

-(void)onSingleTap: (UITapGestureRecognizer *)gr
{
    //code
    singleTapCount++;
    
    if(singleTapCount>2)
    {
        singleTapCount=1;
    }
}

-(void)onDoubleTap: (UITapGestureRecognizer *)gr
{
    //code
    if(gLightToggle==true)
    {
        gLightToggle=false;
    }else
    {
        gLightToggle=true;
    }
}

-(void) onSwipe: (UISwipeGestureRecognizer *) gr
{
    //code
    [self release];
    exit(0);
}

-(void) onLongPress: (UILongPressGestureRecognizer *) gr
{
    //code
    
}

- (void) dealloc
{
    //code
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
    
    
    if(depthRenderbuffer)
    {
        glDeleteRenderbuffers(1, &depthRenderbuffer);
        depthRenderbuffer=0;
    }
    
    if(colorRenderbuffer)
    {
        glDeleteRenderbuffers(1, &colorRenderbuffer);
        colorRenderbuffer=0;
    }
    
    if(defaultFramebuffer)
    {
        glDeleteFramebuffers(1, &defaultFramebuffer);
        defaultFramebuffer=0;
    }
    
    if([EAGLContext currentContext]==eaglContext)
    {
        [EAGLContext setCurrentContext:nil];
    }
    
    [eaglContext release];
    eaglContext=nil;
    [super dealloc];
}
@end
