//
//  GLESView.m
//  iOS_PP_18_Persp_3Light_Sphere_PVF_03062018
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

GLfloat light0_ambient[] = {0.0f,0.0f,0.0f,1.0f};
GLfloat light0_defused[] = {1.0f,0.0f,0.0f,1.0f};
GLfloat light0_specular[] = {1.0f,0.0f,0.0f,1.0f};
GLfloat light0_position[] = {0.0f,0.0f,0.0f,1.0f};

GLfloat light1_ambient[] = {0.0f,0.0f,0.0f,1.0f};
GLfloat light1_defused[] = {0.0f,1.0f,0.0f,1.0f};
GLfloat light1_specular[] = {0.0f,1.0f,0.0f,1.0f};
GLfloat light1_position[] = {0.0f,0.0f,0.0f,1.0f};

GLfloat light2_ambient[] = {0.0f,0.0f,0.0f,1.0f};
GLfloat light2_defused[] = {0.0f,0.0f,1.0f,1.0f};
GLfloat light2_specular[] = {0.0f,0.0f,1.0f,1.0f};
GLfloat light2_position[] = {0.0f,0.0f,0.0f,1.0f};

GLfloat materialAmbient[] = {0.0f,0.0f,0.0f,1.0f};
GLfloat materialDiffuse[] = {1.0,1.0f,1.0f,1.0f};
GLfloat materialSpecular[] = {1.0f,1.0f,1.0f,1.0f};
GLfloat materialShininess = {50.0f};

GLfloat angleRedLight = 0.0f;
GLfloat angleGreenLight = 0.0f;
GLfloat angleBlueLight = 0.0f;

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
    
    //VAOs and VBOs
    //vertex array and buffer objects
    //vertex array and buffer objects
    GLuint gVaoSphere;
    GLuint gVboSpherePosition;
    GLuint gVboSphereNormal;
    GLuint gVboSphereElement;
    
    //uniforms
    GLuint gModelMatrixUniform, gViewMatrixUniform, gProjectionMatrixUniform;
    GLuint gLKeyPressedUniform;
    
    GLuint gL0dUniform;
    GLuint gL0aUniform;
    GLuint gL0sUniform;
    GLuint gL0pUniform;
    
    GLuint gL1dUniform;
    GLuint gL1aUniform;
    GLuint gL1sUniform;
    GLuint gL1pUniform;
    
    GLuint gL2dUniform;
    GLuint gL2aUniform;
    GLuint gL2sUniform;
    GLuint gL2pUniform;
    
    GLuint gKdUniform;
    GLuint gKaUniform;
    GLuint gKsUniform;
    GLuint gKShininessUniform;
    
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
        getSphereVertxData(0.5, 64, 32);
        
        
        printf("Sphere Vertices Size: %lu\n", gSphereVertices.size());
        int count = 0;
        for(std::vector<GLfloat>::const_iterator i = gSphereVertices.begin(); i!=gSphereVertices.end(); i++)
        {
            printf("%f,", *i);
            if(++count>=3)
            {
                printf("\n");
                count=0;
            }
        }
        
        printf("Sphere Normals size: %lu\n", gSphereNormals.size());
        printf("Sphere Texcoords size: %lu\n", gSphereTexcoords.size());
        printf("Sphere Indices size: %lu\n", gSphereElements.size());
        
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

-(void)updateAngle{
    
    angleRedLight = angleRedLight+5.0f;
    if(angleRedLight>=360){
        angleRedLight = 0.0f;
    }
    
    angleGreenLight = angleGreenLight+5.0f;
    if(angleGreenLight>=360){
        angleGreenLight = 0.0f;
    }
    
    angleBlueLight = angleBlueLight+5.0f;
    if(angleBlueLight>=360){
        angleBlueLight = 0.0f;
    }
    
    float angleRedRadian = (float) (3.14f *angleRedLight/180.0f);
    light0_position[0] =0.0f;
    light0_position[1] =100.0f * ((float) cos(angleRedRadian)/2.0f - (float) sin(angleRedRadian)/2.0f);
    light0_position[2] =100.0f * ((float) cos(angleRedRadian)/2.0f + (float) sin(angleRedRadian)/2.0f);
    light0_position[3] =1.0f;
    
    float angleGreenRadian = (float) (3.14f *angleGreenLight/180.0f);
    light1_position[0] =100.0f * ((float) cos(angleGreenRadian)/2.0f + (float) sin(angleGreenRadian)/2.0f);
    light1_position[1] =0.0f;
    light1_position[2] =100.0f * ((float) cos(angleGreenRadian)/2.0f - (float) sin(angleGreenRadian)/2.0f);
    light1_position[3] =1.0f;
    
    float angleBlueRadian = (float) (3.14f *angleBlueLight/180.0f);
    
    light2_position[0] =100.0f * ((float) cos(angleBlueRadian)/2.0f + (float) sin(angleBlueRadian)/2.0f);
    light2_position[1] =100.0f * ((float) cos(angleBlueRadian)/2.0f - (float) sin(angleBlueRadian)/2.0f);
    light2_position[2] =0.0f;
    light2_position[3] =1.0f;
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
				"uniform vec3 u_L2a;"\
				"uniform vec3 u_L2d;"\
				"uniform vec3 u_L2s;"\
				"uniform vec4 u_L2p;"\
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
				
				"vec3 light_direction0= normalize(vec3(u_L0p) - eyeCoordinates.xyz);"\
				"float tn_dot_ld0=max(dot(transformed_normals, light_direction0),0.0);"\
				"vec3 ambient0 = u_L0a * u_Ka;"\
				"vec3 diffuse0 = u_L0d * u_Kd * tn_dot_ld0;"\
				"vec3 reflection_vector0 = reflect(-light_direction0, transformed_normals);"\
				"vec3 viewer_vector0 = normalize(-eyeCoordinates.xyz);"\
				"vec3 specular0 = u_L0s * u_Ks * pow(max(dot(reflection_vector0, viewer_vector0),0.0), u_KShininess);"\
				
				"vec3 light_direction1 = normalize(vec3(u_L1p) - eyeCoordinates.xyz);"\
				"float tn_dot_ld1=max(dot(transformed_normals, light_direction1),0.0);"\
				"vec3 ambient1 = u_L1a * u_Ka;"\
				"vec3 diffuse1 = u_L1d * u_Kd * tn_dot_ld1;"\
				"vec3 reflection_vector1 = reflect(-light_direction1, transformed_normals);"\
				"vec3 viewer_vector1 = normalize(-eyeCoordinates.xyz);"\
				"vec3 specular1 = u_L1s * u_Ks * pow(max(dot(reflection_vector1, viewer_vector1),0.0), u_KShininess);"\
				
				"vec3 light_direction2 = normalize(vec3(u_L2p) - eyeCoordinates.xyz);"\
				"float tn_dot_ld2=max(dot(transformed_normals, light_direction2),0.0);"\
				"vec3 ambient2 = u_L2a * u_Ka;"\
				"vec3 diffuse2 = u_L2d * u_Kd * tn_dot_ld2;"\
				"vec3 reflection_vector2 = reflect(-light_direction2, transformed_normals);"\
				"vec3 viewer_vector2 = normalize(-eyeCoordinates.xyz);"\
				"vec3 specular2 = u_L2s * u_Ks * pow(max(dot(reflection_vector2, viewer_vector2),0.0), u_KShininess);"\
				
				"phong_ads_color=ambient0+diffuse0+specular0 + ambient1+diffuse1+specular1 + ambient2+diffuse2+specular2;"\
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
				"uniform vec3 u_L0p;"\
				"uniform vec3 u_L1p;"\
				"uniform vec3 u_L2p;"\
				"out vec3 transformed_normals;"\
				"out vec3 light0_direction;"\
				"out vec3 light1_direction;"\
				"out vec3 light2_direction;"\
				"out vec3 viewer_vector;"\
				"void main(void)						"\
				"{										"\
				"if(u_LKeyPressed == 1)"\
				"{"\
				"vec4 eyeCoordinates = u_view_matrix * u_model_matrix * vPosition;"\
				"transformed_normals = mat3(u_view_matrix * u_model_matrix) * vNormal;"\
				"light0_direction = vec3(u_L0p) - eyeCoordinates.xyz;"\
				"light1_direction = vec3(u_L1p) - eyeCoordinates.xyz;"\
				"light2_direction = vec3(u_L2p) - eyeCoordinates.xyz;"\
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
				"in vec3 light0_direction;"\
				"in vec3 light1_direction;"\
				"in vec3 light2_direction;"\
				"in vec3 viewer_vector;"\
				"out vec4 FragColor;" \
				"uniform vec3 u_L0a;"\
				"uniform vec3 u_L0d;"\
				"uniform vec3 u_L0s;"\
				"uniform vec3 u_L1a;"\
				"uniform vec3 u_L1d;"\
				"uniform vec3 u_L1s;"\
				"uniform vec3 u_L2a;"\
				"uniform vec3 u_L2d;"\
				"uniform vec3 u_L2s;"\
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
				"vec3 normalized_light0_direction=normalize(light0_direction);"\
				"vec3 normalized_light1_direction=normalize(light1_direction);"\
				"vec3 normalized_light2_direction=normalize(light2_direction);"\
				"vec3 normalized_viewer_vector=normalize(viewer_vector);"\
				
				"vec3 ambient0=u_L0a * u_Ka;"\
				"float tn_dot_id0 = max(dot(normalized_transformed_normals,normalized_light0_direction),0.0);"\
				"vec3 diffuse0 = u_L0d * u_Kd * tn_dot_id0;"\
				"vec3 reflection_vector0=reflect(-normalized_light0_direction, normalized_transformed_normals);"\
				"vec3 specular0 = u_L0s * u_Ks * pow(max(dot(reflection_vector0, normalized_viewer_vector),0.0), u_KShininess);"\
				
				"vec3 ambient1=u_L1a * u_Ka;"\
				"float tn_dot_id1 = max(dot(normalized_transformed_normals,normalized_light1_direction),0.0);"\
				"vec3 diffuse1 = u_L1d * u_Kd * tn_dot_id1;"\
				"vec3 reflection_vector1=reflect(-normalized_light1_direction, normalized_transformed_normals);"\
				"vec3 specular1 = u_L1s * u_Ks * pow(max(dot(reflection_vector1, normalized_viewer_vector),0.0), u_KShininess);"\
				
				"vec3 ambient2=u_L2a * u_Ka;"\
				"float tn_dot_id2 = max(dot(normalized_transformed_normals,normalized_light2_direction),0.0);"\
				"vec3 diffuse2 = u_L2d * u_Kd * tn_dot_id2;"\
				"vec3 reflection_vector2=reflect(-normalized_light2_direction, normalized_transformed_normals);"\
				"vec3 specular2 = u_L2s * u_Ks * pow(max(dot(reflection_vector2, normalized_viewer_vector),0.0), u_KShininess);"\
				
				"phong_ads_color=ambient0+diffuse0+specular0 + ambient1+diffuse1+specular1 + ambient2+diffuse2+specular2;"\
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
    
    gL1aUniform = glGetUniformLocation(gShaderProgramObject, "u_L1a");
    gL1dUniform = glGetUniformLocation(gShaderProgramObject, "u_L1d");
    gL1sUniform = glGetUniformLocation(gShaderProgramObject, "u_L1s");
    gL1pUniform = glGetUniformLocation(gShaderProgramObject, "u_L1p");
    
    gL2aUniform = glGetUniformLocation(gShaderProgramObject, "u_L2a");
    gL2dUniform = glGetUniformLocation(gShaderProgramObject, "u_L2d");
    gL2sUniform = glGetUniformLocation(gShaderProgramObject, "u_L2s");
    gL2pUniform = glGetUniformLocation(gShaderProgramObject, "u_L2p");
    
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
    
    [self initShaders];
    
    [self updateAngle];

    
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
    
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    //start using OpenGL program object
    glUseProgram(gShaderProgramObject);
    
    if (gLightToggle == 1) {
        glUniform1i(gLKeyPressedUniform, 1);
        //light0_position[1] = angleRedLight;
        glUniform3fv(gL0aUniform, 1, light0_ambient);
        glUniform3fv(gL0dUniform, 1, light0_defused);
        glUniform3fv(gL0sUniform, 1, light0_specular);
        glUniform3fv(gL0pUniform, 1, light0_position);
        
        //light1_position[0]=angleGreenLight;
        glUniform3fv(gL1aUniform, 1, light1_ambient);
        glUniform3fv(gL1dUniform, 1, light1_defused);
        glUniform3fv(gL1sUniform, 1, light1_specular);
        glUniform3fv(gL1pUniform, 1, light1_position);
        
        //light2_position[0]=angleBlueLight;
        glUniform3fv(gL2aUniform, 1, light2_ambient);
        glUniform3fv(gL2dUniform, 1, light2_defused);
        glUniform3fv(gL2sUniform, 1, light2_specular);
        glUniform3fv(gL2pUniform, 1, light2_position);
        
        glUniform3fv(gKaUniform, 1, materialAmbient);
        glUniform3fv(gKdUniform, 1, materialDiffuse);
        glUniform3fv(gKsUniform, 1, materialSpecular);
        glUniform1f(gKShininessUniform, materialShininess);
        
    }
    else {
        glUniform1i(gLKeyPressedUniform, 0);
    }
    
    //OpenGL Drawing
    //set modelview and projection matrices to identity
    //set modelview & modelviewprojection matrices to identity
    vmath::mat4 modelMatrix = vmath::mat4::identity();
    vmath::mat4 viewMatrix = vmath::mat4::identity();
    
    //translate
    modelMatrix = vmath::translate(0.0f,0.0f,-5.0f);
    
    
    //pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
    
    
    //bind vao
    glBindVertexArray(gVaoSphere);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVboSphereElement);
    glDrawElements(GL_TRIANGLES, gSphereElements.size(), GL_UNSIGNED_SHORT, 0);
    
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
    
    if(gVboSphereElement)
    {
        glDeleteBuffers(1, &gVboSphereElement);
        gVboSphereElement = 0;
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
