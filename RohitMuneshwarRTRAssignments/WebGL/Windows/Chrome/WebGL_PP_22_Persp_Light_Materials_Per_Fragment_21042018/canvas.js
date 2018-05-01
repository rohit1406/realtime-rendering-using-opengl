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


	var gModelMatrixUniform, gViewMatrixUniform, gProjectionMatrixUniform;

var gLdUniform;
var gLaUniform;
var gLsUniform;
var gLpUniform;
 
var gKdUniform;
var gKaUniform;
var gKsUniform;
var gKShininessUniform;

var gLKeyPressedUniform;

var gPerspectiveProjectionMatrix;

var angleTriangle=0.0;
var gbLight=false;

var sphere =null;
var light_ambient=[0.5,0.5,0.5];
var light_defused=[1.0,1.0,1.0];
var light_specular=[1.0,1.0,1.0];
var light_position=[0.0,0.0,1.0,0.0];

var s1_material_ambient=[0.0215,0.1745,0.0215];
var s1_material_diffuse=[0.07568,0.61424,0.07568];
var s1_material_specular=[0.633,0.727811,0.633];
var s1_material_shininess=0.6 * 128;

var s2_material_ambient=[0.135,0.2225,0.1575];
var s2_material_diffuse=[0.54,0.89,0.63];
var s2_material_specular=[0.316228,0.316228,0.316228];
var s2_material_shininess=0.1 * 128;

var s3_material_ambient=[0.05375,0.05,0.06625];
var s3_material_diffuse=[0.18275,0.17,0.22525];
var s3_material_specular=[0.332741,0.328634,0.346435];
var s3_material_shininess=0.3 * 128;

var s4_material_ambient=[0.25,0.20725,0.20725];
var s4_material_diffuse=[1.0,0.829,0.829];
var s4_material_specular=[0.296648,0.296648,0.296648];
var s4_material_shininess=0.088 * 128;

var s5_material_ambient=[0.1745,0.01175,0.01175];
var s5_material_diffuse=[0.61424,0.04136,0.04136];
var s5_material_specular=[0.727811,0.626959,0.626959];
var s5_material_shininess=0.6 * 128;

var s6_material_ambient=[0.1,0.18725,0.1745];
var s6_material_diffuse=[0.396,0.74151,0.69102];
var s6_material_specular=[0.297254,0.30829,0.306678];
var s6_material_shininess=0.1 * 128;

var s7_material_ambient=[0.329412,0.223529,0.027451];
var s7_material_diffuse=[0.780392,0.568627,0.113725];
var s7_material_specular=[0.992157,0.941176,0.807843];
var s7_material_shininess=0.21794872 * 128;

var s8_material_ambient=[0.2125,0.1275,0.054];
var s8_material_diffuse=[0.714,0.4284,0.18144];
var s8_material_specular=[0.393548,0.271906,0.166721];
var s8_material_shininess=0.2 * 128;

var s9_material_ambient=[0.25,0.25,0.25];
var s9_material_diffuse=[0.4,0.4,0.4];
var s9_material_specular=[0.774597,0.774597,0.774597];
var s9_material_shininess=0.6 * 128;

var s10_material_ambient=[0.19125,0.0735,0.0225];
var s10_material_diffuse=[0.7038,0.27048,0.0828];
var s10_material_specular=[0.256777,0.137622,0.086014];
var s10_material_shininess=0.1 * 128;

var s11_material_ambient=[0.24725,0.1995,0.0745];
var s11_material_diffuse=[0.75164,0.60648,0.22648];
var s11_material_specular=[0.628281,0.555802,0.366065];
var s11_material_shininess=0.4 * 128;

var s12_material_ambient=[0.19225,0.19225,0.19225];
var s12_material_diffuse=[0.50754,0.50754,0.50754];
var s12_material_specular=[0.508273,0.508273,0.508273];
var s12_material_shininess=0.4 * 128;

var s13_material_ambient=[0.0,0.0,0.0];
var s13_material_diffuse=[0.01,0.01,0.01];
var s13_material_specular=[0.50,0.50,0.50];
var s13_material_shininess=0.25 * 128;

var s14_material_ambient=[0.0,0.1,0.06];
var s14_material_diffuse=[0.0,0.50980392,0.50980392];
var s14_material_specular=[0.50196078,0.50196078,0.50196078];
var s14_material_shininess=0.25 * 128;

var s15_material_ambient=[0.0,0.0,0.0];
var s15_material_diffuse=[0.1,0.35,0.1];
var s15_material_specular=[0.45,0.55,0.45];
var s15_material_shininess=0.25 * 128;

var s16_material_ambient=[0.0,0.0,0.0];
var s16_material_diffuse=[0.5,0.0,0.0];
var s16_material_specular=[0.7,0.6,0.6];
var s16_material_shininess=0.25 * 128;

var s17_material_ambient=[0.0,0.0,0.0];
var s17_material_diffuse=[0.55,0.55,0.55];
var s17_material_specular=[0.70,0.70,0.70];
var s17_material_shininess=0.25 * 128;

var s18_material_ambient=[0.0,0.0,0.0];
var s18_material_diffuse=[0.5,0.5,0.0];
var s18_material_specular=[0.60,0.60,0.50];
var s18_material_shininess=0.25 * 128;

var s19_material_ambient=[0.02,0.02,0.02];
var s19_material_diffuse=[0.01,0.01,0.01];
var s19_material_specular=[0.4,0.4,0.4];
var s19_material_shininess=0.078125 * 128;

var s20_material_ambient=[0.0,0.05,0.05];
var s20_material_diffuse=[0.4,0.5,0.5];
var s20_material_specular=[0.04,0.7,0.7];
var s20_material_shininess=0.078125 * 128;

var s21_material_ambient=[0.0,0.05,0.0];
var s21_material_diffuse=[0.4,0.5,0.4];
var s21_material_specular=[0.04,0.7,0.04];
var s21_material_shininess=0.078125 * 128;

var s22_material_ambient=[0.05,0.0,0.0];
var s22_material_diffuse=[0.5,0.4,0.4];
var s22_material_specular=[0.7,0.04,0.04];
var s22_material_shininess=0.078125 * 128;

var s23_material_ambient=[0.05,0.05,0.05];
var s23_material_diffuse=[0.5,0.5,0.5];
var s23_material_specular=[0.7,0.7,0.7];
var s23_material_shininess=0.078125 * 128;

var s24_material_ambient=[0.05,0.05,0.0];
var s24_material_diffuse=[0.5,0.5,0.4];
var s24_material_specular=[0.7,0.7,0.04];
var s24_material_shininess=0.078125 * 128;

var zAxis = -30.0;
var gRotateRedAngle = 0.0;
var gbXPressed=false;
var gbYPressed=false;
var gbZPressed=false;

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
	gRotateRedAngle = gRotateRedAngle + 0.5;
        if (gRotateRedAngle >= 360) {
            gRotateRedAngle = 0.0;
        }

        var angleRedRadian =  (3.14 * gRotateRedAngle / 180.0);

        if (gbXPressed==true) {
            light_position[0] = 0.0;
            light_position[1] = 100.0 * ( Math.cos(angleRedRadian) / 2.0 -  Math.sin(angleRedRadian) / 2.0);
            light_position[2] = 100.0 * ( Math.cos(angleRedRadian) / 2.0 +  Math.sin(angleRedRadian) / 2.0);
            light_position[3] = 1.0;
        } else if (gbYPressed==true) {
            light_position[0] =100.0 * ( Math.cos(angleRedRadian)/2.0 +  Math.sin(angleRedRadian)/2.0);
            light_position[1] =0.0;
            light_position[2] =100.0 * ( Math.cos(angleRedRadian)/2.0 -  Math.sin(angleRedRadian)/2.0);
            light_position[3] =1.0;
        } else if (gbZPressed==true) {
            light_position[0] =100.0 * ( Math.cos(angleRedRadian)/2.0 +  Math.sin(angleRedRadian)/2.0);
            light_position[1] =100.0 * ( Math.cos(angleRedRadian)/2.0 -  Math.sin(angleRedRadian)/2.0);
            light_position[2] =0.0;
            light_position[3] =1.0;
        }



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
					"uniform mediump int u_LKeyPressed;"+
					"uniform vec4 u_Lp;"+
					"out vec3 transformed_normals;"+
					"out vec3 light_direction;"+
					"out vec3 viewer_vector;"+
					"void main(void)						"+
					"{										"+
					"if(u_LKeyPressed == 1)"+
					"{"+
					"vec4 eyeCoordinates = u_view_matrix * u_model_matrix * vPosition;"+
					"transformed_normals = mat3(u_view_matrix * u_model_matrix) * vNormal;"+
					"light_direction = vec3(u_Lp) - eyeCoordinates.xyz;"+
					"viewer_vector = -eyeCoordinates.xyz;"+
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
                        "in vec3 transformed_normals;"+
                        "in vec3 light_direction;"+
                        "in vec3 viewer_vector;"+
                        "out vec4 FragColor;"+
                        "uniform vec3 u_La;"+
                        "uniform vec3 u_Ld;"+
                        "uniform vec3 u_Ls;"+
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
                        "vec3 normalized_light_direction=normalize(light_direction);"+
                        "vec3 normalized_viewer_vector=normalize(viewer_vector);"+
                        "vec3 ambient = u_La * u_Ka;"+
                        "float tn_dot_ld = max(dot(normalized_transformed_normals, normalized_light_direction),0.0);"+
                        "vec3 diffuse = u_Ld * u_Kd * tn_dot_ld;"+
                        "vec3 reflection_vector = reflect(-normalized_light_direction, normalized_transformed_normals);"+
                        "vec3 specular = u_Ls * u_Ks * pow(max(dot(reflection_vector, normalized_viewer_vector), 0.0), u_KShininess);"+
                        "phong_ads_color=ambient + diffuse + specular;"+
                        "}"+
                        "else"+
                        "{"+
                        "phong_ads_color = vec3(1.0, 1.0, 1.0);"+
                        "}"+
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
	
	//get MVP uniform location
	gModelMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_model_matrix");
	gViewMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_view_matrix");
	gProjectionMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_projection_matrix");
	
	gLKeyPressedUniform = gl.getUniformLocation(shaderProgramObject, "u_LKeyPressed");
	
	gLaUniform = gl.getUniformLocation(shaderProgramObject, "u_La");
	gLdUniform = gl.getUniformLocation(shaderProgramObject, "u_Ld");
	gLsUniform = gl.getUniformLocation(shaderProgramObject, "u_Ls");
	gLpUniform = gl.getUniformLocation(shaderProgramObject, "u_Lp");
	
	gKaUniform = gl.getUniformLocation(shaderProgramObject, "u_Ka");
	gKdUniform = gl.getUniformLocation(shaderProgramObject, "u_Kd");
	gKsUniform = gl.getUniformLocation(shaderProgramObject, "u_Ks");
	gKShininessUniform = gl.getUniformLocation(shaderProgramObject, "u_KShininess");
	
	// *** vertices, colors, shader attribs, vbo, vao initializations ***
    sphere=new Mesh();
    makeSphere(sphere,2.0,30,30);
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
	
	
	gl.useProgram(shaderProgramObject);
	
	if(gbLight==true)
	{
		gl.uniform1i(gLKeyPressedUniform, 1);
		gl.uniform3fv(gLaUniform, light_ambient);
		gl.uniform3fv(gLdUniform, light_defused);
		gl.uniform3fv(gLsUniform, light_specular);
		gl.uniform4fv(gLpUniform, light_position);
	}else
	{
		gl.uniform1i(gLKeyPressedUniform,0);
	}
		
	
	//OpenGL drawing
	//set modelview & modelviewprojection matrices to identity
	var modelMatrix = mat4.create();
	var viewMatrix = mat4.create();
	
	//1st row
	//1st
	gl.uniform3fv(gKaUniform, s1_material_ambient);
	gl.uniform3fv(gKdUniform, s1_material_diffuse);
	gl.uniform3fv(gKsUniform, s1_material_specular);
	gl.uniform1f(gKShininessUniform, s1_material_shininess);
	//translate
	modelMatrix = mat4.create();
	mat4.translate(modelMatrix, modelMatrix, [-8.0, 6.0, zAxis]);
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	gl.uniformMatrix4fv(gModelMatrixUniform, false, modelMatrix);
	gl.uniformMatrix4fv(gViewMatrixUniform, false, viewMatrix);
	gl.uniformMatrix4fv(gProjectionMatrixUniform, false, gPerspectiveProjectionMatrix);
	
	sphere.draw();
	
	//2nd
	gl.uniform3fv(gKaUniform, s2_material_ambient);
	gl.uniform3fv(gKdUniform, s2_material_diffuse);
	gl.uniform3fv(gKsUniform, s2_material_specular);
	gl.uniform1f(gKShininessUniform, s2_material_shininess);
	//translate
	modelMatrix = mat4.create();
	//modelMatrix = translate(-1.25, 3.0, zAxis);
	mat4.translate(modelMatrix, modelMatrix, [-4.0, 6.0, zAxis]);
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	gl.uniformMatrix4fv(gModelMatrixUniform, false, modelMatrix);
	gl.uniformMatrix4fv(gViewMatrixUniform, false, viewMatrix);
	gl.uniformMatrix4fv(gProjectionMatrixUniform, false, gPerspectiveProjectionMatrix);
	
	sphere.draw();
	
	//3rd
	gl.uniform3fv(gKaUniform, s3_material_ambient);
	gl.uniform3fv(gKdUniform, s3_material_diffuse);
	gl.uniform3fv(gKsUniform, s3_material_specular);
	gl.uniform1f(gKShininessUniform, s3_material_shininess);
	//translate
	modelMatrix = mat4.create();
	//modelMatrix = translate(1.25, 3.0, zAxis);
	mat4.translate(modelMatrix, modelMatrix, [0.0, 6.0, zAxis]);
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	gl.uniformMatrix4fv(gModelMatrixUniform, false, modelMatrix);
	gl.uniformMatrix4fv(gViewMatrixUniform, false, viewMatrix);
	gl.uniformMatrix4fv(gProjectionMatrixUniform, false, gPerspectiveProjectionMatrix);
	
	sphere.draw();
	
	
	//4th
	gl.uniform3fv(gKaUniform, s4_material_ambient);
	gl.uniform3fv(gKdUniform, s4_material_diffuse);
	gl.uniform3fv(gKsUniform, s4_material_specular);
	gl.uniform1f(gKShininessUniform, s4_material_shininess);
	//translate
	modelMatrix = mat4.create();
	//modelMatrix = translate(3.75, 3.0, zAxis);
	mat4.translate(modelMatrix, modelMatrix, [4.0, 6.0, zAxis]);
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	gl.uniformMatrix4fv(gModelMatrixUniform, false, modelMatrix);
	gl.uniformMatrix4fv(gViewMatrixUniform, false, viewMatrix);
	gl.uniformMatrix4fv(gProjectionMatrixUniform, false, gPerspectiveProjectionMatrix);
	
	sphere.draw();
	
	
	
	
	
	
	//2nd row
	//1st
	//translate
	gl.uniform3fv(gKaUniform, s5_material_ambient);
	gl.uniform3fv(gKdUniform, s5_material_diffuse);
	gl.uniform3fv(gKsUniform, s5_material_specular);
	gl.uniform1f(gKShininessUniform, s5_material_shininess);
	modelMatrix = mat4.create();
	//modelMatrix = translate(-8.0, 2.0, zAxis);
	mat4.translate(modelMatrix, modelMatrix, [-8.0, 2.0, zAxis]);
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	gl.uniformMatrix4fv(gModelMatrixUniform, false, modelMatrix);
	gl.uniformMatrix4fv(gViewMatrixUniform, false, viewMatrix);
	gl.uniformMatrix4fv(gProjectionMatrixUniform, false, gPerspectiveProjectionMatrix);
	
	sphere.draw();
	
	
	//2nd
	gl.uniform3fv(gKaUniform, s6_material_ambient);
	gl.uniform3fv(gKdUniform, s6_material_diffuse);
	gl.uniform3fv(gKsUniform, s6_material_specular);
	gl.uniform1f(gKShininessUniform, s6_material_shininess);
	//translate
	modelMatrix = mat4.create();
	//modelMatrix = translate(-1.25, 2.0, zAxis);
	mat4.translate(modelMatrix, modelMatrix, [-4.0, 2.0, zAxis]);
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	gl.uniformMatrix4fv(gModelMatrixUniform, false, modelMatrix);
	gl.uniformMatrix4fv(gViewMatrixUniform, false, viewMatrix);
	gl.uniformMatrix4fv(gProjectionMatrixUniform, false, gPerspectiveProjectionMatrix);
	
	sphere.draw();
	
	
	//3rd
	gl.uniform3fv(gKaUniform, s6_material_ambient);
	gl.uniform3fv(gKdUniform, s6_material_diffuse);
	gl.uniform3fv(gKsUniform, s6_material_specular);
	gl.uniform1f(gKShininessUniform, s6_material_shininess);
	//translate
	modelMatrix = mat4.create();
	//modelMatrix = translate(1.25, 2.0, zAxis);
	mat4.translate(modelMatrix, modelMatrix, [0.0, 2.0, zAxis]);
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	gl.uniformMatrix4fv(gModelMatrixUniform, false, modelMatrix);
	gl.uniformMatrix4fv(gViewMatrixUniform, false, viewMatrix);
	gl.uniformMatrix4fv(gProjectionMatrixUniform, false, gPerspectiveProjectionMatrix);
	
	sphere.draw();
	
	//4th
	//translate
	gl.uniform3fv(gKaUniform, s7_material_ambient);
	gl.uniform3fv(gKdUniform, s7_material_diffuse);
	gl.uniform3fv(gKsUniform, s7_material_specular);
	gl.uniform1f(gKShininessUniform, s7_material_shininess);
	modelMatrix = mat4.create();
	//modelMatrix = translate(3.75, 2.0, zAxis);
	mat4.translate(modelMatrix, modelMatrix, [4.0, 2.0, zAxis]);
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	gl.uniformMatrix4fv(gModelMatrixUniform, false, modelMatrix);
	gl.uniformMatrix4fv(gViewMatrixUniform, false, viewMatrix);
	gl.uniformMatrix4fv(gProjectionMatrixUniform, false, gPerspectiveProjectionMatrix);
	
	sphere.draw();
	
	
	
	
	//3rd row
	//1st
	gl.uniform3fv(gKaUniform, s8_material_ambient);
	gl.uniform3fv(gKdUniform, s8_material_diffuse);
	gl.uniform3fv(gKsUniform, s8_material_specular);
	gl.uniform1f(gKShininessUniform, s8_material_shininess);
	//translate
	modelMatrix = mat4.create();
	//modelMatrix = translate(-8.0, 1.0, zAxis);
	mat4.translate(modelMatrix, modelMatrix, [-8.0, -2.0, zAxis]);
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	gl.uniformMatrix4fv(gModelMatrixUniform, false, modelMatrix);
	gl.uniformMatrix4fv(gViewMatrixUniform, false, viewMatrix);
	gl.uniformMatrix4fv(gProjectionMatrixUniform, false, gPerspectiveProjectionMatrix);
	
	sphere.draw();
	
	//2nd
	//translate
	gl.uniform3fv(gKaUniform, s9_material_ambient);
	gl.uniform3fv(gKdUniform, s9_material_diffuse);
	gl.uniform3fv(gKsUniform, s9_material_specular);
	gl.uniform1f(gKShininessUniform, s9_material_shininess);
	modelMatrix = mat4.create();
	//modelMatrix = translate(-1.25, 1.0, zAxis);
	mat4.translate(modelMatrix, modelMatrix, [-4.0, -2.0, zAxis]);
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	gl.uniformMatrix4fv(gModelMatrixUniform, false, modelMatrix);
	gl.uniformMatrix4fv(gViewMatrixUniform, false, viewMatrix);
	gl.uniformMatrix4fv(gProjectionMatrixUniform, false, gPerspectiveProjectionMatrix);
	
	sphere.draw();
	
	//3rd
	gl.uniform3fv(gKaUniform, s10_material_ambient);
	gl.uniform3fv(gKdUniform, s10_material_diffuse);
	gl.uniform3fv(gKsUniform, s10_material_specular);
	gl.uniform1f(gKShininessUniform, s10_material_shininess);
	//translate
	modelMatrix = mat4.create();
	//modelMatrix = translate(1.25, 1.0, zAxis);
	mat4.translate(modelMatrix, modelMatrix, [0.0, -2.0, zAxis]);
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	gl.uniformMatrix4fv(gModelMatrixUniform, false, modelMatrix);
	gl.uniformMatrix4fv(gViewMatrixUniform, false, viewMatrix);
	gl.uniformMatrix4fv(gProjectionMatrixUniform, false, gPerspectiveProjectionMatrix);
	
	sphere.draw();
	
	//4th
	gl.uniform3fv(gKaUniform, s11_material_ambient);
	gl.uniform3fv(gKdUniform, s11_material_diffuse);
	gl.uniform3fv(gKsUniform, s11_material_specular);
	gl.uniform1f(gKShininessUniform, s11_material_shininess);
	//translate
	modelMatrix = mat4.create();
	//modelMatrix = translate(3.75, 1.0, zAxis);
	mat4.translate(modelMatrix, modelMatrix, [4.0, -2.0, zAxis]);
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	gl.uniformMatrix4fv(gModelMatrixUniform, false, modelMatrix);
	gl.uniformMatrix4fv(gViewMatrixUniform, false, viewMatrix);
	gl.uniformMatrix4fv(gProjectionMatrixUniform, false, gPerspectiveProjectionMatrix);
	
	sphere.draw();
	
	
	//4rd row
	gl.uniform3fv(gKaUniform, s12_material_ambient);
	gl.uniform3fv(gKdUniform, s12_material_diffuse);
	gl.uniform3fv(gKsUniform, s12_material_specular);
	gl.uniform1f(gKShininessUniform, s12_material_shininess);
	
	
	
	//4th row
	//1st
	//translate
	//modelMatrix = translate(-8.0, 0.0, zAxis);
	modelMatrix = mat4.create();
	mat4.translate(modelMatrix, modelMatrix, [-8.0, -6.0, zAxis]);
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	gl.uniformMatrix4fv(gModelMatrixUniform, false, modelMatrix);
	gl.uniformMatrix4fv(gViewMatrixUniform, false, viewMatrix);
	gl.uniformMatrix4fv(gProjectionMatrixUniform, false, gPerspectiveProjectionMatrix);
	
	sphere.draw();
	
	//2nd
	gl.uniform3fv(gKaUniform, s13_material_ambient);
	gl.uniform3fv(gKdUniform, s13_material_diffuse);
	gl.uniform3fv(gKsUniform, s13_material_specular);
	gl.uniform1f(gKShininessUniform, s13_material_shininess);
	//translate
	modelMatrix = mat4.create();
	//modelMatrix = translate(-1.25, 0.0, zAxis);
	mat4.translate(modelMatrix, modelMatrix, [-4.0, -6.0, zAxis]);
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	gl.uniformMatrix4fv(gModelMatrixUniform, false, modelMatrix);
	gl.uniformMatrix4fv(gViewMatrixUniform, false, viewMatrix);
	gl.uniformMatrix4fv(gProjectionMatrixUniform, false, gPerspectiveProjectionMatrix);
	
	sphere.draw();
	
	
	//3rd
	gl.uniform3fv(gKaUniform, s14_material_ambient);
	gl.uniform3fv(gKdUniform, s14_material_diffuse);
	gl.uniform3fv(gKsUniform, s14_material_specular);
	gl.uniform1f(gKShininessUniform, s14_material_shininess);
	//translate
	modelMatrix = mat4.create();
	//modelMatrix = translate(1.25, 0.0, zAxis);
	mat4.translate(modelMatrix, modelMatrix, [0.0, -6.0, zAxis]);
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	gl.uniformMatrix4fv(gModelMatrixUniform, false, modelMatrix);
	gl.uniformMatrix4fv(gViewMatrixUniform, false, viewMatrix);
	gl.uniformMatrix4fv(gProjectionMatrixUniform, false, gPerspectiveProjectionMatrix);
	
	sphere.draw();
	
	//4th
	gl.uniform3fv(gKaUniform, s15_material_ambient);
	gl.uniform3fv(gKdUniform, s15_material_diffuse);
	gl.uniform3fv(gKsUniform, s15_material_specular);
	gl.uniform1f(gKShininessUniform, s15_material_shininess);
	//translate
	modelMatrix = mat4.create();
	//modelMatrix = translate(3.75, 0.0, zAxis);
	mat4.translate(modelMatrix, modelMatrix, [4.0, -6.0, zAxis]);
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	gl.uniformMatrix4fv(gModelMatrixUniform, false, modelMatrix);
	gl.uniformMatrix4fv(gViewMatrixUniform, false, viewMatrix);
	gl.uniformMatrix4fv(gProjectionMatrixUniform, false, gPerspectiveProjectionMatrix);
	
	sphere.draw();
	
	
	
	//5th row
	//1st
	gl.uniform3fv(gKaUniform, s16_material_ambient);
	gl.uniform3fv(gKdUniform, s16_material_diffuse);
	gl.uniform3fv(gKsUniform, s16_material_specular);
	gl.uniform1f(gKShininessUniform, s16_material_shininess);
	//translate
	modelMatrix = mat4.create();
	//modelMatrix = translate(-8.0, -1.0, zAxis);
	mat4.translate(modelMatrix, modelMatrix, [-8.0, -10.0, zAxis]);
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	gl.uniformMatrix4fv(gModelMatrixUniform, false, modelMatrix);
	gl.uniformMatrix4fv(gViewMatrixUniform, false, viewMatrix);
	gl.uniformMatrix4fv(gProjectionMatrixUniform, false, gPerspectiveProjectionMatrix);
	
	sphere.draw();
	
	//2nd
	gl.uniform3fv(gKaUniform, s17_material_ambient);
	gl.uniform3fv(gKdUniform, s17_material_diffuse);
	gl.uniform3fv(gKsUniform, s17_material_specular);
	gl.uniform1f(gKShininessUniform, s17_material_shininess);
	//translate
	modelMatrix = mat4.create();
	//modelMatrix = translate(-1.25, -1.0, zAxis);
	mat4.translate(modelMatrix, modelMatrix, [-4.0, -10.0, zAxis]);
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	gl.uniformMatrix4fv(gModelMatrixUniform, false, modelMatrix);
	gl.uniformMatrix4fv(gViewMatrixUniform, false, viewMatrix);
	gl.uniformMatrix4fv(gProjectionMatrixUniform, false, gPerspectiveProjectionMatrix);
	
	sphere.draw();
	
	//3rd
	gl.uniform3fv(gKaUniform, s18_material_ambient);
	gl.uniform3fv(gKdUniform, s18_material_diffuse);
	gl.uniform3fv(gKsUniform, s18_material_specular);
	gl.uniform1f(gKShininessUniform, s18_material_shininess);
	//translate
	modelMatrix = mat4.create();
	//modelMatrix = translate(1.25, -1.0, zAxis);
	mat4.translate(modelMatrix, modelMatrix, [0.0, -10.0, zAxis]);
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	gl.uniformMatrix4fv(gModelMatrixUniform, false, modelMatrix);
	gl.uniformMatrix4fv(gViewMatrixUniform, false, viewMatrix);
	gl.uniformMatrix4fv(gProjectionMatrixUniform, false, gPerspectiveProjectionMatrix);
	
	sphere.draw();
	
	//4th
	gl.uniform3fv(gKaUniform, s19_material_ambient);
	gl.uniform3fv(gKdUniform, s19_material_diffuse);
	gl.uniform3fv(gKsUniform, s19_material_specular);
	gl.uniform1f(gKShininessUniform, s19_material_shininess);
	//translate
	modelMatrix = mat4.create();
	//modelMatrix = translate(3.75, -1.0, zAxis);
	mat4.translate(modelMatrix, modelMatrix, [4.0, -10.0, zAxis]);
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	gl.uniformMatrix4fv(gModelMatrixUniform, false, modelMatrix);
	gl.uniformMatrix4fv(gViewMatrixUniform, false, viewMatrix);
	gl.uniformMatrix4fv(gProjectionMatrixUniform, false, gPerspectiveProjectionMatrix);
	
sphere.draw();
	
	
	//6th row
	//1st
	gl.uniform3fv(gKaUniform, s20_material_ambient);
	gl.uniform3fv(gKdUniform, s20_material_diffuse);
	gl.uniform3fv(gKsUniform, s20_material_specular);
	gl.uniform1f(gKShininessUniform, s20_material_shininess);
	//translate
	modelMatrix = mat4.create();
	//modelMatrix = translate(-8.0, -2.0, zAxis);
	mat4.translate(modelMatrix, modelMatrix, [-8.0, -14.0, zAxis]);
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	gl.uniformMatrix4fv(gModelMatrixUniform, false, modelMatrix);
	gl.uniformMatrix4fv(gViewMatrixUniform, false, viewMatrix);
	gl.uniformMatrix4fv(gProjectionMatrixUniform, false, gPerspectiveProjectionMatrix);
	
	sphere.draw();
	
	//2nd
	gl.uniform3fv(gKaUniform, s21_material_ambient);
	gl.uniform3fv(gKdUniform, s21_material_diffuse);
	gl.uniform3fv(gKsUniform, s21_material_specular);
	gl.uniform1f(gKShininessUniform, s21_material_shininess);
	//translate
	modelMatrix = mat4.create();
	//modelMatrix = translate(-1.25, -2.0, zAxis);
	mat4.translate(modelMatrix, modelMatrix, [-4.0, -14.0, zAxis]);
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	gl.uniformMatrix4fv(gModelMatrixUniform, false, modelMatrix);
	gl.uniformMatrix4fv(gViewMatrixUniform, false, viewMatrix);
	gl.uniformMatrix4fv(gProjectionMatrixUniform, false, gPerspectiveProjectionMatrix);
	
	sphere.draw();
	
	
	//3rd
	gl.uniform3fv(gKaUniform, s22_material_ambient);
	gl.uniform3fv(gKdUniform, s22_material_diffuse);
	gl.uniform3fv(gKsUniform, s22_material_specular);
	gl.uniform1f(gKShininessUniform, s22_material_shininess);
	//translate
	modelMatrix = mat4.create();
	//modelMatrix = translate(1.25, -2.0, zAxis);
	mat4.translate(modelMatrix, modelMatrix, [0.0, -14.0, zAxis]);
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	gl.uniformMatrix4fv(gModelMatrixUniform, false, modelMatrix);
	gl.uniformMatrix4fv(gViewMatrixUniform, false, viewMatrix);
	gl.uniformMatrix4fv(gProjectionMatrixUniform, false, gPerspectiveProjectionMatrix);
	
sphere.draw();
	
	//4th
	gl.uniform3fv(gKaUniform, s23_material_ambient);
	gl.uniform3fv(gKdUniform, s23_material_diffuse);
	gl.uniform3fv(gKsUniform, s23_material_specular);
	gl.uniform1f(gKShininessUniform, s23_material_shininess);
	//translate
	modelMatrix = mat4.create();
	//modelMatrix = translate(3.75, -2.0, zAxis);
	mat4.translate(modelMatrix, modelMatrix, [4.0, -14.0, zAxis]);
	//pass the above modelViewProjectionMatrix to the vertex shader in 'u_mvp_matrix' shader variable
	//whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
	gl.uniformMatrix4fv(gModelMatrixUniform, false, modelMatrix);
	gl.uniformMatrix4fv(gViewMatrixUniform, false, viewMatrix);
	gl.uniformMatrix4fv(gProjectionMatrixUniform, false, gPerspectiveProjectionMatrix);
	
	sphere.draw();
	
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
		case 87:
		case 199:
			gbXPressed=false;
			gbYPressed=false;
			gbZPressed=false;
		break;
		case 88:
		case 120:
			gbXPressed=true;
			gbYPressed=false;
			gbZPressed=false;
		break;
		case 89:
		case 121:
			gbXPressed=false;
			gbYPressed=true;
			gbZPressed=false;
		break;
		case 90:
		case 122:
			gbXPressed=false;
			gbYPressed=false;
			gbZPressed=true;
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