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


var vao_square;
var vbo_square_position;
var vbo_square_texture;

var mvpUniform;

var gPerspectiveProjectionMatrix;


var gTexture_sampler_uniform;

var gSmiley_texture=0;
var gNumKeyPressed=0;

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
	"in vec2 vTexture0_chords;"+
	"uniform mat4 u_mvp_matrix;"+
	"out vec2 out_texture0_chords;"+
	"void main(void)"+
	"{"+
	"gl_Position=u_mvp_matrix * vPosition;"+
	"out_texture0_chords=vTexture0_chords;"+
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
	"in vec2 out_texture0_chords;"+
	"uniform sampler2D u_texture0_sampler;"+
	"out vec4 FragColor;"+
	"void main(void)"+
	"{"+
	"FragColor = texture(u_texture0_sampler, out_texture0_chords);"+
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
	gl.bindAttribLocation(shaderProgramObject, WebGLMacros.VDG_ATTRIBUTE_TEXTURE0,"vTexture0_chords");
	
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
	
	
	//load smiley texture
	gSmiley_texture = gl.createTexture();
	gSmiley_texture.image = new Image();
	gSmiley_texture.image.src = "smiley.png";
	gSmiley_texture.image.onload = function()
	{
		gl.bindTexture(gl.TEXTURE_2D, gSmiley_texture);
		gl.pixelStorei(gl.UNPACK_FLIP_X_WEBGL, true);
		gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA,gl.RGBA, gl.UNSIGNED_BYTE, gSmiley_texture.image);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
		gl.bindTexture(gl.TEXTURE_2D, null);
	}
	
	
	//get MVP uniform location
	mvpUniform=gl.getUniformLocation(shaderProgramObject,"u_mvp_matrix");
	gTexture_sampler_uniform = gl.getUniformLocation(shaderProgramObject, "u_texture0_sampler");
	// *** vertices, colors, shader attribs, vbo, vao initialization ***
	var squareVertices=new Float32Array(
									[
								 1.0,1.0,0.0,
                        -1.0,1.0,0.0,
                        -1.0,-1.0,0.0,
                        1.0,-1.0,0.0
									]
									);
		
		
	
	
	//triangle vao
	vao_square=gl.createVertexArray();
	gl.bindVertexArray(vao_square);
	
	//triangle position
	vbo_square_position=gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, vbo_square_position);
	gl.bufferData(gl.ARRAY_BUFFER, squareVertices, gl.STATIC_DRAW);
	gl.vertexAttribPointer(WebGLMacros.VDG_ATTRIBUTE_VERTEX,3, //3 is for X, Y, Z co-ordinates in out pyramidVertices array
															gl.FLOAT,
															false, 0,0);
	gl.enableVertexAttribArray(WebGLMacros.VDG_ATTRIBUTE_VERTEX);
	gl.bindBuffer(gl.ARRAY_BUFFER, null);
	
	//triangle color
	vbo_square_texture=gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, vbo_square_texture);
	gl.bufferData(gl.ARRAY_BUFFER, null, gl.DYNAMIC_DRAW);
	gl.vertexAttribPointer(WebGLMacros.VDG_ATTRIBUTE_TEXTURE0, 2, gl.FLOAT, false, 0, null);
	gl.enableVertexAttribArray(WebGLMacros.VDG_ATTRIBUTE_TEXTURE0);
	gl.bindBuffer(gl.ARRAY_BUFFER, null);
	
	gl.bindVertexArray(null);
	
	//set clear color
    //gl.shadeModel(gl.SMOOTH);
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.enable(gl.CULL_FACE);
    gl.depthFunc(gl.LEQUAL);
	
	gl.enable(gl.TEXTURE_2D);
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
	
	//pyramid draw
	var modelViewMatrix=mat4.create();
	var modelViewProjectionMatrix=mat4.create();
	
	mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -6.0]);
	mat4.multiply(modelViewProjectionMatrix, gPerspectiveProjectionMatrix, modelViewMatrix);
	
	gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);
	
	
	
	var squareTexcoords1;
									
	if(gNumKeyPressed==1){
		squareTexcoords1 = new Float32Array(
							[
							0.5,0.5,
                        0.0,0.5,
                       0.0,0.0,
                        0.5,0.0
							]
							);
	} else if(gNumKeyPressed==2){
squareTexcoords1 = new Float32Array(
							[
							1.0,1.0,
                        0.0,1.0,
                       0.0,0.0,
                        1.0,0.0
							]
							);		
	} else if(gNumKeyPressed==3){
		squareTexcoords1 = new Float32Array(
							[
							2.0,2.0,
                        0.0,2.0,
                       0.0,0.0,
                        2.0,0.0
							]
							);
	} else if(gNumKeyPressed==4){
		squareTexcoords1 = new Float32Array(
							[
							0.5,0.5,
                        0.5,0.5,
                       0.5,0.5,
                        0.5,0.5
							]
							);	
	}else{
		squareTexcoords1 = new Float32Array(
							[
							1.0,1.0,
                        0.0,1.0,
                        0.0,0.0,
                        1.0,0.0
							]
							);	
	}						
	gl.bindVertexArray(vao_square);
	
	vbo_square_texture=gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, vbo_square_texture);
	gl.bufferData(gl.ARRAY_BUFFER, squareTexcoords1, gl.DYNAMIC_DRAW);
	gl.vertexAttribPointer(WebGLMacros.VDG_ATTRIBUTE_TEXTURE0, 2, gl.FLOAT, false, 0, null);
	gl.enableVertexAttribArray(WebGLMacros.VDG_ATTRIBUTE_TEXTURE0);
	gl.bindBuffer(gl.ARRAY_BUFFER, null);
	
	gl.bindTexture(gl.TEXTURE_2D, gSmiley_texture);
	gl.uniform1i(gTexture_sampler_uniform, 0);
	
	
	
	gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
	
	gl.bindVertexArray(null);
	
	gl.useProgram(null);
	
	
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
			//drawText("Hello WebGL !!!");
			break;
		//case 31: //1  
		case 49: //1  
			gNumKeyPressed=1;
		break;
		//case 32: //2  
		case 50: //2
			gNumKeyPressed=2;
		break;
		//case 33: //3
		case 51: //3  				
			gNumKeyPressed=3;
		break;
		//case 34: //4
		case 52: //4
			gNumKeyPressed=4;
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
	if(gSmiley_texture)
    {
        gl.deleteTexture(gSmiley_texture);
        gSmiley_texture=0;
    }
    
	if(vao_square)
	{
		gl.deleteVertexArray(vao_square);
		vao_square=null;
	}
	
	if(vbo_square_position)
	{
		gl.deleteBuffer(vbo_square_position);
		vbo_square_position=null;
	}
	
	if(vbo_square_texture)
	{
		gl.deleteBuffer(vbo_square_texture);
		vbo_square_texture=null;
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