//
//  GLESView.m
//  iOS_PP_05_Persp_Col_Tri_03062018
//
//  Created by rohit muneshwar on 12/06/18.
//
//

#import "OpenGLES/ES3/gl.h"
#import "OpenGLES/ES3/glext.h"
#import "GLESView.h"
#include "vmath.h"

enum
{
    VDG_ATTRIBUTE_VERTEX = 0,
    VDG_ATTRIBUTE_COLOR = 1,
    VDG_ATTRIBUTE_NORMAL = 2,
    VDG_ATTRIBUTE_TEXTURE0 = 3
};
@implementation GLESView
{
    EAGLContext *eaglContext;
    
    GLuint defaultFramebuffer;
    GLuint colorRenderbuffer;
    GLuint depthRenderbuffer;
    
    id displayLink;
    NSInteger animationFrameInterval;
    BOOL isAnimating;
    
    //for shaders
    GLuint gVertexShaderObject;
    GLuint gFragmentShaderObject;
    GLuint gShaderProgramObject;
    
    //vertex array and buffer objects
    GLuint gVao_triangle;
    GLuint gVbo_triangle_position;
    GLuint gVbo_triangle_color;
    
    //uniforms
    GLuint mvpUniform;
    
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
        
        //create shader objects
        //vertex shader
        gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
        
        //provide source code to shader
        const GLchar *vertexShaderSourceCode=
        "#version 300 es"\
        "\n"\
        "in vec4 vPosition;"\
        "in vec4 vColor;"\
        "uniform mat4 u_mvp_matrix;"\
        "out vec4 out_color;"\
        "void main(void)"\
        "{"\
        "gl_Position=u_mvp_matrix * vPosition;"\
        "out_color=vColor;"\
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
                    printf("Vertex Shader Compilation Log : %s\n", szInfoLog);
                    
                    free(szInfoLog);
                    [self release];
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
        "#version 300 es"\
        "\n"\
        "precision highp float;"\
        "in vec4 out_color;"\
        "out vec4 FragColor;"\
        "void main(void)"\
        "{"\
        "FragColor = out_color;"\
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
                    printf("Fragment Shader Compilation Log : %s\n", szInfoLog);
                    
                    free(szInfoLog);
                    [self release];
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
        glBindAttribLocation(gShaderProgramObject, VDG_ATTRIBUTE_COLOR, "vColor");
        
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
                    printf("Shader Program Link Log : %s\n", szInfoLog);
                    
                    free(szInfoLog);
                    [self release];                }
            }
        }
        
        //get uniform locations
        mvpUniform = glGetUniformLocation(gShaderProgramObject, "u_mvp_matrix");
        
        //vertices, colors, shader attribs, vbo, vao initialization
        const GLfloat triangleVertices[] =
        {
            0.0f, 1.0f, 0.0f, //appex
            -1.0f, -1.0f, 0.0f, //left-bottom
            1.0f, -1.0f, 0.0f //right-bottom
        };
        
        const GLfloat triangleColor[] =
        {
            1.0f, 0.0f, 0.0f, //red
            0.0f, 1.0f, 0.0f, //green
            0.0f, 0.0f, 1.0f
        };
        
        //configure vao
        glGenVertexArrays(1, &gVao_triangle);
        glBindVertexArray(gVao_triangle);
        
        glGenBuffers(1, &gVbo_triangle_position);
        glBindBuffer(GL_ARRAY_BUFFER, gVbo_triangle_position);
        glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        glGenBuffers(1, &gVbo_triangle_color);
        glBindBuffer(GL_ARRAY_BUFFER, gVbo_triangle_color);
        glBufferData(GL_ARRAY_BUFFER, sizeof(triangleColor), triangleColor, GL_STATIC_DRAW);
        glVertexAttribPointer(VDG_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(VDG_ATTRIBUTE_COLOR);
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
        glClearColor(0.1f, 0.2f, 0.3f, 1.0f); //blue
        
        //set projection matrix to identity matrix
        gPerspectiveProjectionMatrix = vmath::mat4::identity();
        
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
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    //start using OpenGL program object
    glUseProgram(gShaderProgramObject);
    
    //OpenGL Drawing
    //set modelview and projection matrices to identity
    vmath::mat4 modelViewMatrix = vmath::mat4::identity();
    vmath::mat4 modelViewProjectionMatrix = vmath::mat4::identity();
    
    //translate
    modelViewMatrix = vmath::translate(0.0f,0.0f,-6.0f);
    
    //multiply the modelview and orthographic matrix to get modelViewProjectionMatrix
    //order is important
    modelViewProjectionMatrix = gPerspectiveProjectionMatrix * modelViewMatrix;
    
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
    
    //bind vao
    glBindVertexArray(gVao_triangle);
    
    glDrawArrays(GL_TRIANGLES, 0, 3);
    
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
    
}

-(void)onDoubleTap: (UITapGestureRecognizer *)gr
{
    //code
    
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
    if(gVao_triangle)
    {
        glDeleteVertexArrays(1, &gVao_triangle);
        gVao_triangle = 0;
    }
    
    if(gVbo_triangle_position)
    {
        glDeleteBuffers(1, &gVbo_triangle_position);
        gVbo_triangle_position = 0;
    }
    
    if(gVbo_triangle_color)
    {
        glDeleteBuffers(1, &gVbo_triangle_color);
        gVbo_triangle_color = 0;
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
