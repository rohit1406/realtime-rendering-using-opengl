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
var vbo_pyramid_color;

var vao_cube;
var vbo_cube_position;
var vbo_cube_color;

var mvpUniform;

var gPerspectiveProjectionMatrix;

var angleTriangle=0.0;
var angleSquare=0.0;

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

	if(angleSquare>=360.0)
	{
		angleSquare=0.0;
	}	
	
	angleTriangle=angleTriangle+0.7;
	angleSquare=angleSquare+0.7;
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
	"in vec4 vPosition;"+
	"in vec4 vColor;"+
	"uniform mat4 u_mvp_matrix;"+
	"out vec4 out_color;"+
	"void main(void)"+
	"{"+
	"gl_Position=u_mvp_matrix * vPosition;"+
	"out_color=vColor;"+
	"}";
	
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
	"in vec4 out_color;"+
	"out vec4 FragColor;"+
	"void main(void)"+
	"{"+
	"FragColor = out_color;"+
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
	gl.bindAttribLocation(shaderProgramObject, WebGLMacros.VDG_ATTRIBUTE_COLOR,"vColor");
	
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
	mvpUniform=gl.getUniformLocation(shaderProgramObject,"u_mvp_matrix");
	
	// *** vertices, colors, shader attribs, vbo, vao initialization ***
	var pyramidVertices=new Float32Array(
									[
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
									]
									);
		
	var pyramidColor=new Float32Array(
							[
							1, 0, 0,
							0, 1, 0,
							0, 0, 1,

							1, 0, 0,
							0, 0, 1,
							0, 1, 0,

							1, 0, 0,
							0, 1, 0,
							0, 0, 1,

							1, 0, 0,
							0, 0, 1,
							0, 1, 0
							]
							);
									
									
	var cubeVertices=new Float32Array(
									[
									1.0, 1.0, -1.0,
									-1.0, 1.0, -1.0,
									-1.0, 1.0, 1.0,
									1.0, 1.0, 1.0,

									1.0, -1.0, 1.0,
									-1.0, -1.0, 1.0,
									-1.0, -1.0, -1.0,
									1.0, -1.0, -1.0,

									1.0, 1.0, 1.0,
									-1.0, 1.0, 1.0,
									-1.0, -1.0, 1.0,
									1.0, -1.0, 1.0,

									1.0, -1.0, -1.0,
									-1.0, -1.0, -1.0,
									-1.0, 1.0, -1.0,
									1.0, 1.0, -1.0,

									-1.0, 1.0, 1.0,
									-1.0, 1.0, -1.0,
									-1.0, -1.0, -1.0,
									-1.0, -1.0, 1.0,

									1.0, 1.0, -1.0,
									1.0, 1.0, 1.0,
									1.0, -1.0, 1.0,
									1.0, -1.0, -1.0
									]
									);
									
	var cubeColor=new Float32Array(
									[
									0.0, 1.0, 0.0,
									0.0, 1.0, 0.0,
									0.0, 1.0, 0.0,
									0.0, 1.0, 0.0,

									1.0, 0.5, 0.0,
									1.0, 0.5, 0.0,
									1.0, 0.5, 0.0,
									1.0, 0.5, 0.0,

									1.0, 0.0, 0.0,
									1.0, 0.0, 0.0,
									1.0, 0.0, 0.0,
									1.0, 0.0, 0.0,

									1.0, 1.0, 0.0,
									1.0, 1.0, 0.0,
									1.0, 1.0, 0.0,
									1.0, 1.0, 0.0,

									0.0, 0.0, 1.0,
									0.0, 0.0, 1.0,
									0.0, 0.0, 1.0,
									0.0, 0.0, 1.0,

									1.0, 0.0, 1.0,
									1.0, 0.0, 1.0,
									1.0, 0.0, 1.0,
									1.0, 0.0, 1.0
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
	vbo_pyramid_color=gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, vbo_pyramid_color);
	gl.bufferData(gl.ARRAY_BUFFER, pyramidColor, gl.STATIC_DRAW);
	gl.vertexAttribPointer(WebGLMacros.VDG_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
	gl.enableVertexAttribArray(WebGLMacros.VDG_ATTRIBUTE_COLOR);
	gl.bindBuffer(gl.ARRAY_BUFFER, null);
	
	
	gl.bindVertexArray(null);
	
	//square vao
	vao_cube=gl.createVertexArray();
	gl.bindVertexArray(vao_cube);
	
	//vbo square
	vbo_cube_position=gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER,vbo_cube_position);
	gl.bufferData(gl.ARRAY_BUFFER, cubeVertices, gl.STATIC_DRAW);
	gl.vertexAttribPointer(WebGLMacros.VDG_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
	gl.enableVertexAttribArray(WebGLMacros.VDG_ATTRIBUTE_VERTEX);
	gl.bindBuffer(gl.ARRAY_BUFFER, null);
	
	
	//square color
	vbo_cube_color=gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, vbo_cube_color);
	gl.bufferData(gl.ARRAY_BUFFER, cubeColor, gl.STATIC_DRAW);
	gl.vertexAttribPointer(WebGLMacros.VDG_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
	gl.enableVertexAttribArray(WebGLMacros.VDG_ATTRIBUTE_COLOR);
	gl.bindBuffer(gl.ARRAY_BUFFER, null);
	
	
	gl.bindVertexArray(null);
	
	//set clear color
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
	
	gl.useProgram(shaderProgramObject);
	
	//triangle draw
	var modelViewMatrix=mat4.create();
	var modelViewProjectionMatrix=mat4.create();
	var rotateMatrix=mat4.create();
	
	mat4.translate(modelViewMatrix, modelViewMatrix, [-1.5, 0.0, -6.0]);
	mat4.rotateY(rotateMatrix, modelViewMatrix, degToRad(angleTriangle));
	mat4.multiply(modelViewProjectionMatrix, gPerspectiveProjectionMatrix, rotateMatrix);
	
	gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);
	
	gl.bindVertexArray(vao_pyramid);
	
	gl.drawArrays(gl.TRIANGLES, 0, 12);
	
	gl.bindVertexArray(null);
	
	
	//square draw
	modelViewMatrix=mat4.create();
	modelViewProjectionMatrix=mat4.create();
	rotateMatrix=mat4.create();
	mat4.translate(modelViewMatrix, modelViewMatrix, [1.5, 0.0, -7.0]);
	mat4.rotateX(rotateMatrix, modelViewMatrix, degToRad(angleSquare));
	mat4.rotateY(rotateMatrix, rotateMatrix, degToRad(angleSquare));
	mat4.rotateZ(rotateMatrix, rotateMatrix, degToRad(angleSquare));
	mat4.multiply(modelViewProjectionMatrix, gPerspectiveProjectionMatrix, rotateMatrix);
	gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);
	gl.bindVertexArray(vao_cube);
	gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
	gl.drawArrays(gl.TRIANGLE_FAN, 4, 4);
	gl.drawArrays(gl.TRIANGLE_FAN, 8, 4);
	gl.drawArrays(gl.TRIANGLE_FAN, 12, 4);
	gl.drawArrays(gl.TRIANGLE_FAN, 16, 4);
	gl.drawArrays(gl.TRIANGLE_FAN, 20, 4);
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
	
	if(vbo_pyramid_color)
	{
		gl.deleteBuffer(vbo_pyramid_color);
		vbo_pyramid_color=null;
	}
	
	if(vao_cube)
	{
		gl.deleteVertexArray(vao_cube);
		vao_cube=null;
	}
	
	if(vbo_cube_position)
	{
		gl.deleteBuffer(vbo_cube_position);
		vbo_cube_position=null;
	}
	
	if(vbo_cube_color)
	{
		gl.deleteBuffer(vbo_cube_color);
		vbo_cube_color=null;
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