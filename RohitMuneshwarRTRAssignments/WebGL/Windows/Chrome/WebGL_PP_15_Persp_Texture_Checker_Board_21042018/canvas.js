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


var gVao_StraightBoard;
var gVbo_StraightBoard_Position;
var gVbo_StraightBoard_Texture;

var gVao_TiltedBoard;
var gVbo_TiltedBoard_Position

var mvpUniform;

var gPerspectiveProjectionMatrix;


var gTexture_sampler_uniform;

var gChecker_board_texture=0;
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
	
	
	//load checker board texture
	gChecker_board_texture = gl.createTexture();
	
		gl.bindTexture(gl.TEXTURE_2D, gChecker_board_texture);
    gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL,true);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.REPEAT);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.REPEAT);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
		
		var c = 0;
		var checkImage = [];
		for(var i =0;i<64;i++){
			for(var j =0; j<64;j++){
				for (var k =0 ;k< 4;k++){
					c = ((i & 0x8) ^ (j & 0x8))*255;
					checkImage[(i*64+j)*4]= c;
					checkImage[(i*64+j)*4+1]= c;
					checkImage[(i*64+j)*4+2]= c;
					checkImage[(i*64+j)*4+3]= 0xff;
				}
			}
		} 
    gl.texImage2D(gl.TEXTURE_2D,0,gl.RGBA,64,64,0,gl.RGBA,gl.UNSIGNED_BYTE,new Uint8Array(checkImage));
		//gl.texEnvf(gl.TEXTURE_ENV, gl.TEXTURE_ENV_MODE, gl.REPLACE);
		
		gl.bindTexture(gl.TEXTURE_2D, null);
	
	
	
	//get MVP uniform location
	mvpUniform=gl.getUniformLocation(shaderProgramObject,"u_mvp_matrix");
	gTexture_sampler_uniform = gl.getUniformLocation(shaderProgramObject, "u_texture0_sampler");
	// *** vertices, colors, shader attribs, vbo, vao initialization ***
	var straightFacingQuadVertices=new Float32Array(
									[
								 -2.0,-1.0,0.0,
		-2.0,1.0,0.0,
		0.0,1.0,0.0,
		0.0,-1.0,0.0
									]
									);
		
	var checkerBoardTexcoords =new Float32Array(
	[
		0.0, 0.0,
		0.0,1.0,
		1.0,1.0,
		1.0,0.0
	]);

	var tiltedFacingQuadVertices = new Float32Array([
        1.0,-1.0,0.0,
        1.0,1.0,0.0,
        2.41421,1.0,-1.41421,
        2.41421,-1.0,-1.41421
   ] );
	
	//triangle vao
	gVao_StraightBoard=gl.createVertexArray();
	gl.bindVertexArray(gVao_StraightBoard);
	
	//triangle position
	gVbo_StraightBoard_Position=gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, gVbo_StraightBoard_Position);
	gl.bufferData(gl.ARRAY_BUFFER, straightFacingQuadVertices, gl.STATIC_DRAW);
	gl.vertexAttribPointer(WebGLMacros.VDG_ATTRIBUTE_VERTEX,3, //3 is for X, Y, Z co-ordinates in out pyramidVertices array
															gl.FLOAT,
															false, 0,0);
	gl.enableVertexAttribArray(WebGLMacros.VDG_ATTRIBUTE_VERTEX);
	gl.bindBuffer(gl.ARRAY_BUFFER, null);
	
	//triangle color
	gVbo_StraightBoard_Texture=gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, gVbo_StraightBoard_Texture);
	gl.bufferData(gl.ARRAY_BUFFER, checkerBoardTexcoords, gl.STATIC_DRAW);
	gl.vertexAttribPointer(WebGLMacros.VDG_ATTRIBUTE_TEXTURE0, 2, gl.FLOAT, false, 0, null);
	gl.enableVertexAttribArray(WebGLMacros.VDG_ATTRIBUTE_TEXTURE0);
	gl.bindBuffer(gl.ARRAY_BUFFER, null);
	
	gl.bindVertexArray(null);
	
	
	 gVao_TiltedBoard = gl.createVertexArray();
    gl.bindVertexArray(gVao_TiltedBoard);
    //position
    gVbo_TiltedBoard_Position = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER,gVbo_TiltedBoard_Position);
    gl.bufferData(gl.ARRAY_BUFFER,tiltedFacingQuadVertices,gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.VDG_ATTRIBUTE_VERTEX,3,gl.FLOAT,false,0,0);
    gl.enableVertexAttribArray(WebGLMacros.VDG_ATTRIBUTE_VERTEX);
    gl.bindBuffer(gl.ARRAY_BUFFER,null);
   //vbo texture
    gVbo_Texture = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER,gVbo_Texture)
    gl.bufferData(gl.ARRAY_BUFFER,checkerBoardTexcoords,gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.VDG_ATTRIBUTE_TEXTURE0,2,gl.FLOAT,false,0,0); // 2 for S and T in tex cord
    gl.enableVertexAttribArray(WebGLMacros.VDG_ATTRIBUTE_TEXTURE0);
    gl.bindBuffer(gl.ARRAY_BUFFER,null)
    
    //gl.vertexAttrib3f(WebGLMacros.SAM_ATTRIBUTE_COLOR,0.5,0.5,1.0);
    gl.bindVertexArray(null);
	
	//set clear color
    //gl.shadeModel(gl.SMOOTH);
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
   // gl.enable(gl.CULL_FACE);
    gl.depthFunc(gl.LEQUAL);
	
	gl.enable(gl.TEXTURE_2D);
	gl.clearColor(0.2 , 0.3 , 0.3 , 1.0);
	
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
	
	//straight draw
	var modelViewMatrix=mat4.create();
	var modelViewProjectionMatrix=mat4.create();
	
	mat4.translate(modelViewMatrix, modelViewMatrix, [-0.5, 0.0, -5.0]);
	mat4.multiply(modelViewProjectionMatrix, gPerspectiveProjectionMatrix, modelViewMatrix);
	
	gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);
	
	
						
	gl.bindVertexArray(gVao_StraightBoard);
	
	gl.bindTexture(gl.TEXTURE_2D, gChecker_board_texture);
	gl.uniform1i(gTexture_sampler_uniform, 0);
	
	
	
	gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
	
	gl.bindVertexArray(null);
	
	
	
	modelViewMatrix=mat4.create();
	modelViewProjectionMatrix=mat4.create();
	
	mat4.translate(modelViewMatrix, modelViewMatrix, [0.5, 0.0, -5.0]);
	mat4.multiply(modelViewProjectionMatrix, gPerspectiveProjectionMatrix, modelViewMatrix);
	
	gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);
	
	
						
	gl.bindVertexArray(gVao_TiltedBoard);
	
	gl.bindTexture(gl.TEXTURE_2D, gChecker_board_texture);
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
	if(gChecker_board_texture)
    {
        gl.deleteTexture(gChecker_board_texture);
        gChecker_board_texture=0;
    }
    
	if(gVao_StraightBoard)
	{
		gl.deleteVertexArray(gVao_StraightBoard);
		gVao_StraightBoard=null;
	}
	
	if(gVbo_StraightBoard_Position)
	{
		gl.deleteBuffer(gVbo_StraightBoard_Position);
		gVbo_StraightBoard_Position=null;
	}
	
	if(gVbo_StraightBoard_Texture)
	{
		gl.deleteBuffer(gVbo_StraightBoard_Texture);
		gVbo_StraightBoard_Texture=null;
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