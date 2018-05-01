//global variables
var canvas=null;
var gl=null; //webgl context
var gbFullscreen=false;
var canvas_original_width;
var canvas_original_height;

const WebGLMacros= //when whole 'WebGLMacros' is 'const', all inside it are automatically 'const'
{
	VDG_ATTRIBUTE_VERTEX:0,
	VDG_ATTRIBUTE_COLOR:1,
	VDG_ATTRIBUTE_NORMAL:2,
	VDG_ATTRIBUTE_TEXTURE0:3
};

//To start animation: To have requestAnimationFrame() to be called "cross-browser" compatible
var requestAnimationFrame =
window.requestAnimationFrame ||
window.webkitRequestAnimationFrame ||
window.mozRequestAnimationFrame ||
window.oRequestAnimationFrame ||
window.msRequestAnimationFrame;

//To stop animation: To have cancelAnimationFrame() to be called "cross-browser" compatible
var cancelAnimationFrame =
window.cancelAnimationFrame ||
window.webkitCancelRequestAnimationFrame || window.webkitCancelAnimationFrame ||
window.mozCancelRequestAnimationFrame || window.mozCancelAnimationFrame ||
window.oCancelRequestAnimationFrame || window.oCancelAnimationFrame ||
window.msCancelRequestAnimationFrame || window.msCancelAnimationFrame;

var vertexShaderObject;
var fragmentShaderObject;
var shaderProgramObject;

var light0_ambient=[0.0,0.0,0.0];
    var light0_defused=[1.0,0.0,0.0];
    var light0_specular=[1.0,0.0,0.0];
    var light0_position=[0.0,0.0,0.0,1.0];

    var light1_ambient=[0.0,0.0,0.0];
    var light1_defused=[0.0,1.0,0.0];
    var light1_specular=[0.0,1.0,0.0];
    var light1_position=[0.0,0.0,0.0,1.0];

    var light2_ambient=[0.0,0.0,0.0];
    var light2_defused=[0.0,0.0,1.0];
    var light2_specular=[0.0,0.0,1.0];
    var light2_position=[0.0,0.0,0.0,1.0];

    var material_ambient=[0.0,0.0,0.0];
    var material_defused=[1.0,1.0,1.0];
    var material_specular=[1.0,1.0,1.0];
    var material_shininess = 50.0;

var sphere=null;

var modelMatrixUniform, viewMatrixUniform, projectionMatrixUniform;
var gL0dUniform;
var gL0aUniform;
var gL0sUniform;
var gL0pUniform;

var gL1dUniform;
var gL1aUniform;
var gL1sUniform;
var gL1pUniform;

var gL2dUniform;
var gL2aUniform;
var gL2sUniform;
var gL2pUniform;
 
var gKdUniform;
var gKaUniform;
var gKsUniform;
var gKShininessUniform;
var gLKeyPressedUniform;


var gPerspectiveProjectionMatrix;

var angleRedLight = 0.0;
var angleGreenLight = 0.0;
var angleBlueLight = 0.0;
var gbAnimate=false;
var gbLight=false;

//variable declarations
	var bIsAKeyPressed = false;
	var bIsLKeyPressed = false;
	var bIsFKeyPressed = false;
	var bIsVKeyPressed = false;
	
var vertexShaderSourceCodePV;
var fragmentShaderSourceCodePV;
var vertexShaderSourceCodePF;
var fragmentShaderSourceCodePF;
	
//onload function
function main()
{
	// get <canvas> element
	canvas = document.getElementById("AMC");
	if(!canvas)
	{
		console.log("Obtaining Canvas Failed\n");
	}else
	{
		console.log("Obtaining Canvas Succeeded\n");
	}
	
	canvas_original_width = canvas.width;
	canvas_original_height=canvas.height;
	
	//print canvas width and height on console
	console.log("Canvas Width: "+canvas.width+" And Canvas Height: "+canvas.height);
		
	//register keyboards keydown event handler
	window.addEventListener("keydown", keyDown, false); //third param for capturing of events e.g. multiple windows and cursor
	window.addEventListener("click", mouseDown, false);
	window.addEventListener("resize", resize, false);
	
	//initialize WebGL
	init();
	
	//start drawing here as warming-up
	resize();
	draw();
}

function updateAngle(){
	
	angleRedLight = angleRedLight+1.0;
	if(angleRedLight>=360){
		angleRedLight = 0.0;
	}
	
	angleGreenLight = angleGreenLight+1.0;
	if(angleGreenLight>=360){
		angleGreenLight = 0.0;
	}
	
	angleBlueLight = angleBlueLight+1.0;
	if(angleBlueLight>=360){
		angleBlueLight = 0.0;
	}
	
	var angleRedRadian = degToRad(angleRedLight);
        light0_position[0] =0.0;
        light0_position[1] =100.0 * (Math.cos(angleRedRadian)/2.0 - Math.sin(angleRedRadian)/2.0);
        light0_position[2] =100.0 * (Math.cos(angleRedRadian)/2.0 + Math.sin(angleRedRadian)/2.0);
        light0_position[3] =1.0;

        var angleGreenRadian = degToRad(angleGreenLight);
        light1_position[0] =100.0 * (Math.cos(angleGreenRadian)/2.0 + Math.sin(angleGreenRadian)/2.0);
        light1_position[1] =0.0;
        light1_position[2] =100.0 * (Math.cos(angleGreenRadian)/2.0 - Math.sin(angleGreenRadian)/2.0);
        light1_position[3] =1.0;

        var angleBlueRadian = degToRad(angleBlueLight);

        light2_position[0] =100.0 * (Math.cos(angleBlueRadian)/2.0 + Math.sin(angleBlueRadian)/2.0);
        light2_position[1] =100.0 * (Math.cos(angleBlueRadian)/2.0 - Math.sin(angleBlueRadian)/2.0);
        light2_position[2] =0.0;
        light2_position[3] =1.0;
}

function degToRad(degrees)
{
	return (degrees * Math.PI/180.0);
}

function init()
{
	//code
	//get WebGL 2.0 context
	gl=canvas.getContext("webgl2");
	if(!gl)
	{
		console.log("ailed to get rendering context for WebGL\n");
		return;
	}else
	{
		console.log("Obtaining webgl2 rendering Context Succeeded\n");
	}
	gl.viewportWidth=canvas.width;
	gl.viewportHeight=canvas.height;
	
	
	//vertex shader
	 vertexShaderSourceCodePF=
	"#version 300 es"+
    "\n"+
   "in vec4 vPosition;						"+
					"in vec3 vNormal;"+
					"uniform mat4 u_model_matrix;"+
					"uniform mat4 u_view_matrix;"+
					"uniform mat4 u_projection_matrix;"+
					"uniform mediump int u_LKeyPressed;"+
					"uniform vec4 u_L0p;"+
					"uniform vec4 u_L1p;"+
					"uniform vec4 u_L2p;"+
					"out vec3 transformed_normals;"+
					"out vec3 light_direction0;"+
					"out vec3 light_direction1;"+
					"out vec3 light_direction2;"+
					"out vec3 viewer_vector;"+
					"void main(void)						"+
					"{										"+
					"if(u_LKeyPressed == 1)"+
					"{"+
					"vec4 eyeCoordinates = u_view_matrix * u_model_matrix * vPosition;"+
					"transformed_normals = mat3(u_view_matrix * u_model_matrix) * vNormal;"+
					"light_direction0 = vec3(u_L0p) - eyeCoordinates.xyz;"+
					"light_direction1 = vec3(u_L1p) - eyeCoordinates.xyz;"+
					"light_direction2 = vec3(u_L2p) - eyeCoordinates.xyz;"+
					"viewer_vector = -eyeCoordinates.xyz;"+
					"}"+
					"gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;	"+
					"}										";
	
	 fragmentShaderSourceCodePF=
	"#version 300 es"+
    "\n"+
    "precision highp float;"+
                        "in vec3 transformed_normals;"+
                        "in vec3 light_direction0;"+
						"in vec3 light_direction1;"+
						"in vec3 light_direction2;"+
                        "in vec3 viewer_vector;"+
                        "out vec4 FragColor;"+
                        "uniform vec3 u_L0a;"+
                        "uniform vec3 u_L0d;"+
                        "uniform vec3 u_L0s;"+
						"uniform vec3 u_L1a;"+
                        "uniform vec3 u_L1d;"+
                        "uniform vec3 u_L1s;"+
						"uniform vec3 u_L2a;"+
                        "uniform vec3 u_L2d;"+
                        "uniform vec3 u_L2s;"+
                        "uniform vec3 u_Ka;"+
                        "uniform vec3 u_Kd;"+
                        "uniform vec3 u_Ks;"+
                        "uniform float u_KShininess;"+
                        "uniform int u_LKeyPressed;"+
                        "void main(void)"+
                        "{"+
                        "vec3 phong_ads_color;"+
                        "if(u_LKeyPressed==1)"+
                        "{"+
                        "vec3 normalized_transformed_normals=normalize(transformed_normals);"+
                        "vec3 normalized_light_direction0=normalize(light_direction0);"+
                        "vec3 normalized_viewer_vector=normalize(viewer_vector);"+
                        "vec3 ambient0 = u_L0a * u_Ka;"+
                        "float tn_dot_ld0 = max(dot(normalized_transformed_normals, normalized_light_direction0),0.0);"+
                        "vec3 diffuse0 = u_L0d * u_Kd * tn_dot_ld0;"+
                        "vec3 reflection_vector0 = reflect(-normalized_light_direction0, normalized_transformed_normals);"+
                        "vec3 specular0 = u_L0s * u_Ks * pow(max(dot(reflection_vector0, normalized_viewer_vector), 0.0), u_KShininess);"+
                        

						"vec3 normalized_light_direction1=normalize(light_direction1);"+
                        
                        "vec3 ambient1 = u_L1a * u_Ka;"+
                        "float tn_dot_ld1 = max(dot(normalized_transformed_normals, normalized_light_direction1),0.0);"+
                        "vec3 diffuse1 = u_L1d * u_Kd * tn_dot_ld1;"+
                        "vec3 reflection_vector1 = reflect(-normalized_light_direction1, normalized_transformed_normals);"+
                        "vec3 specular1 = u_L1s * u_Ks * pow(max(dot(reflection_vector1, normalized_viewer_vector), 0.0), u_KShininess);"+
                        
						"vec3 normalized_light_direction2=normalize(light_direction2);"+
                        
                        "vec3 ambient2 = u_L2a * u_Ka;"+
                        "float tn_dot_ld2 = max(dot(normalized_transformed_normals, normalized_light_direction2),0.0);"+
                        "vec3 diffuse2 = u_L2d * u_Kd * tn_dot_ld2;"+
                        "vec3 reflection_vector2 = reflect(-normalized_light_direction2, normalized_transformed_normals);"+
                        "vec3 specular2 = u_L2s * u_Ks * pow(max(dot(reflection_vector2, normalized_viewer_vector), 0.0), u_KShininess);"+
                        
						"phong_ads_color=ambient0 + diffuse0 + specular0 + ambient1 + diffuse1 + specular1 + ambient2 + diffuse2 + specular2;"+
                        "}"+
                        "else"+
                        "{"+
                        "phong_ads_color = vec3(1.0, 1.0, 1.0);"+
                        "}"+
                        "FragColor = vec4(phong_ads_color, 1.0);"+
                        "}";
	
	
	//set clear color
    //gl.shadeModel(gl.SMOOTH);
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.enable(gl.CULL_FACE);
    gl.depthFunc(gl.LEQUAL);
	gl.clearColor(0.0 , 0.0 , 0.0 , 1.0);
	
	gbAnimate=false;
	gbLight=false;
	
	//initialize projection matrix
	gPerspectiveProjectionMatrix=mat4.create();
}

function resize()
{
	//code
	if(gbFullscreen==true)
	{
		canvas.width=window.innerWidth;
		canvas.height=window.innerHeight;
	}else
	{
		canvas.width=canvas_original_width;
		canvas.height=canvas_original_height;
	}
	
	//set the viewport to match
	gl.viewport(0,0,canvas.width, canvas.height);
	
	mat4.perspective(gPerspectiveProjectionMatrix, 45.0, parseFloat(canvas.width)/parseFloat(canvas.height), 0.1, 100.0);
}

function draw()
{
	//code
	gl.clear(gl.COLOR_BUFFER_BIT);
	
	//vertex shader
	vertexShaderObject=gl.createShader(gl.VERTEX_SHADER);
	gl.shaderSource(vertexShaderObject, vertexShaderSourceCodePF);
	gl.compileShader(vertexShaderObject);
	if(gl.getShaderParameter(vertexShaderObject,gl.COMPILE_STATUS)==false)
	{
		var error = gl.getShaderInfoLog(vertexShaderObject);
		if(error.length>0)
		{
			alert(error);
			uninitialize();
		}
	}
	
	//fragment shader
	
	fragmentShaderObject=gl.createShader(gl.FRAGMENT_SHADER);
	gl.shaderSource(fragmentShaderObject, fragmentShaderSourceCodePF);
	gl.compileShader(fragmentShaderObject);
	if(gl.getShaderParameter(fragmentShaderObject,gl.COMPILE_STATUS)==false)
	{
		var error = gl.getShaderInfoLog(fragmentShaderObject);
		if(error.length>0)
		{
			alert(error);
			uninitialize();
		}
	}
	
	//shader program
	shaderProgramObject=gl.createProgram();
	gl.attachShader(shaderProgramObject, vertexShaderObject);
	gl.attachShader(shaderProgramObject, fragmentShaderObject);
	
	//pre-link binding of shader program object with vertex shader attributes
	gl.bindAttribLocation(shaderProgramObject, WebGLMacros.VDG_ATTRIBUTE_VERTEX,"vPosition");
	gl.bindAttribLocation(shaderProgramObject, WebGLMacros.VDG_ATTRIBUTE_NORMAL,"vNormal");
	
	//linking
	gl.linkProgram(shaderProgramObject);
	if(!gl.getProgramParameter(shaderProgramObject, gl.LINK_STATUS))
	{
		var error=gl.getProgramInfoLog(shaderProgramObject);
		if(error.length>0)
		{
			alert(error);
			uninitialize();
		}
	}
	
	//get MVP uniform location
	modelMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_model_matrix");
	viewMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_view_matrix");
	projectionMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_projection_matrix");
	
	gLKeyPressedUniform = gl.getUniformLocation(shaderProgramObject, "u_LKeyPressed");
	
	gL0aUniform = gl.getUniformLocation(shaderProgramObject, "u_L0a");
	gL0dUniform = gl.getUniformLocation(shaderProgramObject, "u_L0d");
	gL0sUniform = gl.getUniformLocation(shaderProgramObject, "u_L0s");
	gL0pUniform = gl.getUniformLocation(shaderProgramObject, "u_L0p");
	
	gL1aUniform = gl.getUniformLocation(shaderProgramObject, "u_L1a");
	gL1dUniform = gl.getUniformLocation(shaderProgramObject, "u_L1d");
	gL1sUniform = gl.getUniformLocation(shaderProgramObject, "u_L1s");
	gL1pUniform = gl.getUniformLocation(shaderProgramObject, "u_L1p");
	
	gL2aUniform = gl.getUniformLocation(shaderProgramObject, "u_L2a");
	gL2dUniform = gl.getUniformLocation(shaderProgramObject, "u_L2d");
	gL2sUniform = gl.getUniformLocation(shaderProgramObject, "u_L2s");
	gL2pUniform = gl.getUniformLocation(shaderProgramObject, "u_L2p");
	
	gKaUniform = gl.getUniformLocation(shaderProgramObject, "u_Ka");
	gKdUniform = gl.getUniformLocation(shaderProgramObject, "u_Kd");
	gKsUniform = gl.getUniformLocation(shaderProgramObject, "u_Ks");
	gKShininessUniform = gl.getUniformLocation(shaderProgramObject, "u_KShininess");
	
	// *** vertices, colors, shader attribs, vbo, vao initializations ***
    sphere=new Mesh();
    makeSphere(sphere,2.0,30,30);
	
	
	
	gl.useProgram(shaderProgramObject);
	
	if(gbLight==true)
	{
		gl.uniform1i(gLKeyPressedUniform, 1);
		
		//light0_position[1] = angleRedLight;
		gl.uniform3fv(gL0aUniform,light0_ambient);
		gl.uniform3fv(gL0dUniform,light0_defused);
		gl.uniform3fv(gL0sUniform,light0_specular);
		gl.uniform4fv(gL0pUniform,light0_position);
		
		//light1_position[0]=angleGreenLight;
		gl.uniform3fv(gL1aUniform,light1_ambient);
		gl.uniform3fv(gL1dUniform,light1_defused);
		gl.uniform3fv(gL1sUniform,light1_specular);
		gl.uniform4fv(gL1pUniform,light1_position);
		
		//light2_position[0]=angleBlueLight;
		gl.uniform3fv(gL2aUniform,light2_ambient);
		gl.uniform3fv(gL2dUniform,light2_defused);
		gl.uniform3fv(gL2sUniform,light2_specular);
		gl.uniform4fv(gL2pUniform,light2_position);
		
		gl.uniform3fv(gKaUniform,material_ambient);
		gl.uniform3fv(gKdUniform,material_defused);
		gl.uniform3fv(gKsUniform,material_specular);
		gl.uniform1f(gKShininessUniform, material_shininess);
	}else
	{
		gl.uniform1i(gLKeyPressedUniform,0);
	}
	
	var modelMatrix=mat4.create();
    var viewMatrix=mat4.create();

    mat4.translate(modelMatrix, modelMatrix, [0.0,0.0,-6.0]);
    
    gl.uniformMatrix4fv(modelMatrixUniform,false,modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform,false,viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform,false,gPerspectiveProjectionMatrix);
    
    sphere.draw();
	
	gl.useProgram(null);
	
	//animation loop
	//if(gbAnimate==true)
	updateAngle();
	requestAnimationFrame(draw, canvas);
}

function keyDown(event)
{
	//code
	switch(event.keyCode)
	{
		case 81:
		case 48: //q
			//uninitialize
			uninitialize();
			//close our applications tab
			window.close();
			break;
		case 70: //'F' or 'f'
		if(bIsFKeyPressed==false && bIsVKeyPressed==true)
		{
			bIsFKeyPressed=true;
			bIsVKeyPressed=false;
		}else{
			bIsFKeyPressed=false;
			bIsVKeyPressed=true;
		}
		break;
		case 86: //'V' or 'v'
		if(bIsFKeyPressed==true && bIsVKeyPressed==false)
		{
			bIsFKeyPressed=false;
			bIsVKeyPressed=true;
		}else
		{
			bIsFKeyPressed=true;
			bIsVKeyPressed=false;
		}
		break;
		case 83:
			toggleFullScreen();
			
			//repait
			//drawText("Hello WebGL !!!");
			break;
		case 76:
		if (bIsLKeyPressed == false)
					{
						gbLight = true;
						bIsLKeyPressed = true;
					}
					else
					{
						gbLight = false;
						bIsLKeyPressed = false;
					}
		break;
		case 65:
		if (bIsAKeyPressed == false)
					{
						gbAnimate = true;
						bIsAKeyPressed = true;
					}
					else
					{
						gbAnimate = false;
						bIsAKeyPressed = false;
					}
					break;
		break;
	}
}

function mouseDown()
{
	//code
	//alert("Mouse Is Clicked");
}

function toggleFullScreen()
{
	//code
	var fullscreen_element =
	document.fullscreenElement ||
	document.webkitFullscreenElement ||
	document.mozFullScreenElement ||
	document.msFullscreenElement ||
	null;
	
	//if not fullscreen
	if(fullscreen_element==null)
	{
		if(canvas.requestFullscreen)
		{
			canvas.requestFullscreen();
		}else if(canvas.mozRequestFullScreen)
		{
			canvas.mozRequestFullScreen();
		}else if(canvas.webkitRequestFullscreen)
		{
			canvas.webkitRequestFullscreen();
		}else if(canvas.msRequestFullscreen)
		{
			canvas.msRequestFullscreen();
		}
	}else //if already fullscreen
	{
		if(document.exitFullscreen)
		{
			document.exitFullscreen();
		}else if(document.mozCancelFullScreen)
		{
			document.mozCancelFullScreen();
		}else if(document.webkitExitFullscreen)
		{
			document.webkitExitFullscreen();
		}else if(document.msExitFullscreen)
		{
			document.msExitFullscreen();
		}
	}
}

function uninitialize()
{
	//code
	if(sphere)
    {
        sphere.deallocate();
        sphere=null;
    }
	
	if(shaderProgramObject)
	{
		if(fragmentShaderObject)
		{
			gl.detachShader(shaderProgramObject, fragmentShaderObject);
			gl.deleteShader(fragmentShaderObject);
			fragmentShaderObject=null;
		}
		
		if(vertexShaderObject)
		{
			gl.detachShader(shaderProgramObject, vertexShaderObject);
			gl.deleteShader(vertexShaderObject);
			vertexShaderObject=null;
		}
		gl.deleteProgram(shaderProgramObject);
		shaderProgramObject=null;
	}
}