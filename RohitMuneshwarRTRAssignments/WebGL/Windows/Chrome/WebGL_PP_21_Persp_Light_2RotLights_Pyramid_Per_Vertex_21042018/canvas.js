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

var vao_pyramid;
var vbo_pyramid_position;
var vbo_pyramid_normal;


	var gModelMatrixUniform, gViewMatrixUniform, gProjectionMatrixUniform;
	var gL0aUniform, gL0dUniform, gL0pUniform, gL0sUniform, gKaUniform, gKsUniform, gKShininessUniform,gKdUniform;
	var gL1aUniform, gL1dUniform, gL1pUniform, gL1sUniform;
	var gLKeyPressedUniform;

var gPerspectiveProjectionMatrix;

var angleTriangle=0.0;
var gbLight=false;


	//light0
var light0_ambient=[0.0,0.0,0.0];
var light0_defused=[1.0,0.0,0.0];
var light0_specular=[1.0,0.0,0.0];
var light0_position=[2.0,1.0,1.0,0.0];


//light1
var light1_ambient=[0.0,0.0,0.0];
var light1_defused=[0.0,0.0,1.0];
var light1_specular=[0.0,0.0,1.0];
var light1_position=[-2.0,1.0,1.0,0.0];

var materialAmbient=[0.0,0.0,0.0];
var materialDifuse=[1.0,1.0,1.0];
var materialSpecular=[1.0,1.0,1.0];
var materialShininess = 50.0;

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
	if(angleTriangle>=360.0)
	{
		angleTriangle=0.0;
	}

	angleTriangle=angleTriangle+0.7;

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
	var vertexShaderSourceCode=
	"#version 300 es"+
	"\n"+
	"in vec4 vPosition;						"+
				"in vec3 vNormal;"+
				"uniform mat4 u_model_matrix;"+
				"uniform mat4 u_view_matrix;"+
				"uniform mat4 u_projection_matrix;"+
				"uniform int u_LKeyPressed;"+
				"uniform vec3 u_L0a;"+
				"uniform vec3 u_L0d;"+
				"uniform vec3 u_L0s;"+
				"uniform vec4 u_L0p;"+
				"uniform vec3 u_L1a;"+
				"uniform vec3 u_L1d;"+
				"uniform vec3 u_L1s;"+
				"uniform vec4 u_L1p;"+
				"uniform vec3 u_L2a;"+
				"uniform vec3 u_L2d;"+
				"uniform vec3 u_L2s;"+
				"uniform vec4 u_L2p;"+
				"uniform vec3 u_Ka;"+
				"uniform vec3 u_Kd;"+
				"uniform vec3 u_Ks;"+
				"uniform float u_KShininess;"+
				"out vec3 phong_ads_color;"+
				"void main(void)						"+
				"{										"+
				"if(u_LKeyPressed == 1)"+
				"{"+
				"vec4 eyeCoordinates = u_view_matrix * u_model_matrix * vPosition;"+
				"vec3 transformed_normals = normalize(mat3(u_view_matrix * u_model_matrix) * vNormal);"+
				"vec3 light_direction0 = normalize(vec3(u_L0p) - eyeCoordinates.xyz);"+
				"float tn_dot_ld0=max(dot(transformed_normals, light_direction0),0.0);"+
				"vec3 ambient0 = u_L0a * u_Ka;"+
				"vec3 diffuse0 = u_L0d * u_Kd * tn_dot_ld0;"+
				"vec3 reflection_vector0 = reflect(-light_direction0, transformed_normals);"+
				"vec3 viewer_vector = normalize(-eyeCoordinates.xyz);"+
				"vec3 specular0 = u_L0s * u_Ks * pow(max(dot(reflection_vector0, viewer_vector),0.0), u_KShininess);"+
				
				"vec3 light_direction1 = normalize(vec3(u_L1p) - eyeCoordinates.xyz);"+
				"float tn_dot_ld1=max(dot(transformed_normals, light_direction1),0.0);"+
				"vec3 ambient1 = u_L1a * u_Ka;"+
				"vec3 diffuse1 = u_L1d * u_Kd * tn_dot_ld1;"+
				"vec3 reflection_vector1 = reflect(-light_direction1, transformed_normals);"+
				
				"vec3 specular1 = u_L1s * u_Ks * pow(max(dot(reflection_vector1, viewer_vector),0.0), u_KShininess);"+
				
				"vec3 light_direction2 = normalize(vec3(u_L2p) - eyeCoordinates.xyz);"+
				"float tn_dot_ld2=max(dot(transformed_normals, light_direction2),0.0);"+
				"vec3 ambient2 = u_L2a * u_Ka;"+
				"vec3 diffuse2 = u_L2d * u_Kd * tn_dot_ld2;"+
				"vec3 reflection_vector2 = reflect(-light_direction2, transformed_normals);"+
				
				"vec3 specular2 = u_L2s * u_Ks * pow(max(dot(reflection_vector2, viewer_vector),0.0), u_KShininess);"+
				"phong_ads_color=ambient0+diffuse0+specular0+ambient1+diffuse1+specular1+ambient2+diffuse2+specular2;"+
				"}"+
				"else"+
				"{"+
				"phong_ads_color=vec3(1.0,1.0,1.0);"+
				"}"+
				"gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;	"+
				"}										";
	
	vertexShaderObject=gl.createShader(gl.VERTEX_SHADER);
	gl.shaderSource(vertexShaderObject, vertexShaderSourceCode);
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
	var fragmentShaderSourceCode=
	"#version 300 es"+
	"\n"+
	"precision highp float;"+
                        "in vec3 phong_ads_color;"+
                        "out vec4 FragColor;"+
                        "void main(void)"+
                        "{"+
                        "FragColor = vec4(phong_ads_color, 1.0);"+
                        "}";
	fragmentShaderObject=gl.createShader(gl.FRAGMENT_SHADER);
	gl.shaderSource(fragmentShaderObject, fragmentShaderSourceCode);
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
	
	
	
	//set clear color
    //gl.shadeModel(gl.SMOOTH);
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.enable(gl.CULL_FACE);
    gl.depthFunc(gl.LEQUAL);
	gl.clearColor(0.0 , 0.0 , 0.0 , 1.0);
	
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
	
	//get MVP uniform location
	gModelMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_model_matrix");
	gViewMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_view_matrix");
	gProjectionMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_projection_matrix");
	
	gLKeyPressedUniform = gl.getUniformLocation(shaderProgramObject, "u_LKeyPressed");
	
	gL0aUniform = gl.getUniformLocation(shaderProgramObject, "u_L0a");
	gL0dUniform = gl.getUniformLocation(shaderProgramObject, "u_L0d");
	gL0sUniform = gl.getUniformLocation(shaderProgramObject, "u_L0s");
	gL0pUniform = gl.getUniformLocation(shaderProgramObject, "u_L0p");
	gL1aUniform = gl.getUniformLocation(shaderProgramObject, "u_L1a");
	gL1dUniform = gl.getUniformLocation(shaderProgramObject, "u_L1d");
	gL1sUniform = gl.getUniformLocation(shaderProgramObject, "u_L1s");
	gL1pUniform = gl.getUniformLocation(shaderProgramObject, "u_L1p");
	
	gKaUniform = gl.getUniformLocation(shaderProgramObject, "u_Ka");
	gKdUniform = gl.getUniformLocation(shaderProgramObject, "u_Kd");
	gKsUniform = gl.getUniformLocation(shaderProgramObject, "u_Ks");
	gKShininessUniform = gl.getUniformLocation(shaderProgramObject, "u_KShininess");
		
	
	// *** vertices, colors, shader attribs, vbo, vao initialization ***
	var pyramidVertices=new Float32Array(
									[
								  0.0, 1.0, 0.0,    // front-top
								  -1.0, -1.0, 1.0,  // front-left
								  1.0, -1.0, 1.0,   // front-right
								  
								  0.0, 1.0, 0.0,    // right-top
								  1.0, -1.0, 1.0,   // right-left
								  1.0, -1.0, -1.0,  // right-right
								  
								  0.0, 1.0, 0.0,    // back-top
								  1.0, -1.0, -1.0,  // back-left
								  -1.0, -1.0, -1.0, // back-right
								  
								  0.0, 1.0, 0.0,    // left-top
								  -1.0, -1.0, -1.0, // left-left
								  -1.0, -1.0, 1.0   // left-right
									]
									);
		
	var pyramidNormals=new Float32Array(
							[
							0.0,0.447214,0.894427,
		0.0,0.447214,0.894427,
		0.0,0.447214,0.894427,
		
		0.894427,0.447214,0.0,
		0.894427,0.447214,0.0,
		0.894427,0.447214,0.0,
		
		0.0,0.447214,-0.0894427,
		0.0,0.447214,-0.0894427,
		0.0,0.447214,-0.0894427,
		
		-0.894427,0.447214,0.0,
		-0.894427,0.447214,0.0,
		-0.894427,0.447214,0.0
							]
							);
									
		
	
	//triangle vao
	vao_pyramid=gl.createVertexArray();
	gl.bindVertexArray(vao_pyramid);
	
	//triangle position
	vbo_pyramid_position=gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, vbo_pyramid_position);
	gl.bufferData(gl.ARRAY_BUFFER, pyramidVertices, gl.STATIC_DRAW);
	gl.vertexAttribPointer(WebGLMacros.VDG_ATTRIBUTE_VERTEX,3, //3 is for X, Y, Z co-ordinates in out pyramidVertices array
															gl.FLOAT,
															false, 0,0);
	gl.enableVertexAttribArray(WebGLMacros.VDG_ATTRIBUTE_VERTEX);
	gl.bindBuffer(gl.ARRAY_BUFFER, null);
	
	//triangle color
	vbo_pyramid_normal=gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, vbo_pyramid_normal);
	gl.bufferData(gl.ARRAY_BUFFER, pyramidNormals, gl.STATIC_DRAW);
	gl.vertexAttribPointer(WebGLMacros.VDG_ATTRIBUTE_NORMAL, 3, gl.FLOAT, false, 0, 0);
	gl.enableVertexAttribArray(WebGLMacros.VDG_ATTRIBUTE_NORMAL);
	gl.bindBuffer(gl.ARRAY_BUFFER, null);
	
	
	gl.bindVertexArray(null);
	
	gl.useProgram(shaderProgramObject);
	
	if(gbLight==true)
		{
			gl.uniform1i(gLKeyPressedUniform, 1);
			gl.uniform3fv(gL0aUniform, light0_ambient);
			gl.uniform3fv(gL0dUniform, light0_defused);
			gl.uniform3fv(gL0sUniform, light0_specular);
			gl.uniform4fv(gL0pUniform, light0_position);
			gl.uniform3fv(gL1aUniform, light1_ambient);
			gl.uniform3fv(gL1dUniform, light1_defused);
			gl.uniform3fv(gL1sUniform, light1_specular);
			gl.uniform4fv(gL1pUniform, light1_position);
			gl.uniform3fv(gKaUniform, materialAmbient);
			gl.uniform3fv(gKdUniform, materialDifuse);
			gl.uniform3fv(gKsUniform, materialSpecular);
			gl.uniform1f(gKShininessUniform, materialShininess);
			
		}else
		{
			gl.uniform1i(gLKeyPressedUniform, 0);
		}
		
	//triangle draw
	var modelMatrix=mat4.create();
    var viewMatrix=mat4.create();
	var rotateMatrix=mat4.create();
	
	mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -3.0]);
	mat4.rotateY(rotateMatrix, modelMatrix, degToRad(angleTriangle));
	//mat4.multiply(modelViewProjectionMatrix, gPerspectiveProjectionMatrix, rotateMatrix);
	
	 gl.uniformMatrix4fv(gModelMatrixUniform,false,rotateMatrix);
       gl.uniformMatrix4fv(gViewMatrixUniform,false,viewMatrix);
        gl.uniformMatrix4fv(gProjectionMatrixUniform,false,gPerspectiveProjectionMatrix);
	gl.bindVertexArray(vao_pyramid);
	
	gl.drawArrays(gl.TRIANGLES, 0, 12);
	
	gl.bindVertexArray(null);
	
	gl.useProgram(null);
	
	//animation loop
	updateAngle();
	requestAnimationFrame(draw, canvas);
}

function keyDown(event)
{
	//code
	switch(event.keyCode)
	{
		case 27: //escape
			//uninitialize
			uninitialize();
			//close our applications tab
			window.close();
			break;
		case 76:
		if (gbLight == false)
					{
						gbLight = true;
						
					}
					else
					{
						gbLight = false;
					}
		break;
		case 70: //'F' or 'f'
			toggleFullScreen();
			
			//repait
			drawText("Hello WebGL !!!");
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
	if(vao_pyramid)
	{
		gl.deleteVertexArray(vao_pyramid);
		vao_pyramid=null;
	}
	
	if(vbo_pyramid_position)
	{
		gl.deleteBuffer(vbo_pyramid_position);
		vbo_pyramid_position=null;
	}
	
	if(vbo_pyramid_normal)
	{
		gl.deleteBuffer(vbo_pyramid_normal);
		vbo_pyramid_normal=null;
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