#include "Graphics\window.h"
#include "Camera\camera.h"
#include "Shaders\shader.h"
#include "Model Loading\mesh.h"
#include "Model Loading\texture.h"
#include "Model Loading\meshLoaderObj.h"

void processKeyboardInput();

float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

Window window("Game Engine", 800, 800);
Camera camera;

glm::vec3 playerPosition = glm::vec3(0.0f, 0.0f, 5.0f); // player start position

bool firstPerson = true;              // current camera mode
float thirdPersonDistance = 10.0f;    // distance behind player in third-person view


glm::vec3 lightColor = glm::vec3(1.0f);
glm::vec3 lightPos = glm::vec3(-180.0f, 100.0f, -200.0f);

glm::vec3 tumbleweedPos = glm::vec3(-300.0f, -19.0f, -250.0f);// start left side
float tumbleweedSpeed = 20.0f; // units per second
float tumbleweedRotation = 0.0f; // degrees
//Mesh tumbleweed = loader.loadObj("Resources/Models/sphere.obj", textures2); 
// textures2 = rock texture, or use a custom tumbleweed texture

float getTerrainHeight(float x, float z) { 
	return -20.0f;
}

int main()
{
	glClearColor(0.2f, 0.8f, 1.0f, 1.0f);

	//building and compiling shader program
	Shader shader("Shaders/vertex_shader.glsl", "Shaders/fragment_shader.glsl");
	Shader sunShader("Shaders/sun_vertex_shader.glsl", "Shaders/sun_fragment_shader.glsl");

	//Textures
	GLuint tex = loadBMP("Resources/Textures/wood.bmp");
	GLuint tex2 = loadBMP("Resources/Textures/rock.bmp");
	GLuint tex3 = loadBMP("Resources/Textures/orange.bmp");
	// ADDED — ground texture
	GLuint texGround = loadBMP("Resources/Textures/ground.bmp");
	glBindTexture(GL_TEXTURE_2D, texGround);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// ADDED — sky texture
	GLuint texSky = loadBMP("Resources/Textures/sky.bmp");
	glBindTexture(GL_TEXTURE_2D, texSky);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// ADDED — saloon texture
	GLuint texSaloon = loadBMP("Resources/Textures/saloon_defuse.bmp");

	glEnable(GL_DEPTH_TEST);

	glm::mat4 ProjectionMatrix;
	glm::mat4 ViewMatrix;

	GLuint MatrixID;
	GLuint MatrixID2;
	GLuint ModelMatrixID;

	//Test custom mesh loading
	std::vector<Vertex> vert;
	vert.push_back(Vertex());
	vert[0].pos = glm::vec3(10.5f, 10.5f, 0.0f);
	vert[0].textureCoords = glm::vec2(1.0f, 1.0f);

	vert.push_back(Vertex());
	vert[1].pos = glm::vec3(10.5f, -10.5f, 0.0f);
	vert[1].textureCoords = glm::vec2(1.0f, 0.0f);

	vert.push_back(Vertex());
	vert[2].pos = glm::vec3(-10.5f, -10.5f, 0.0f);
	vert[2].textureCoords = glm::vec2(0.0f, 0.0f);

	vert.push_back(Vertex());
	vert[3].pos = glm::vec3(-10.5f, 10.5f, 0.0f);
	vert[3].textureCoords = glm::vec2(0.0f, 1.0f);

	vert[0].normals = glm::normalize(glm::cross(vert[1].pos - vert[0].pos, vert[3].pos - vert[0].pos));
	vert[1].normals = glm::normalize(glm::cross(vert[2].pos - vert[1].pos, vert[0].pos - vert[1].pos));
	vert[2].normals = glm::normalize(glm::cross(vert[3].pos - vert[2].pos, vert[1].pos - vert[2].pos));
	vert[3].normals = glm::normalize(glm::cross(vert[0].pos - vert[3].pos, vert[2].pos - vert[3].pos));

	std::vector<int> ind = { 0, 1, 3,
		1, 2, 3 };

	std::vector<Texture> textures;
	textures.push_back(Texture());
	textures[0].id = tex;
	textures[0].type = "texture_diffuse";

	std::vector<Texture> textures2;
	textures2.push_back(Texture());
	textures2[0].id = tex2;
	textures2[0].type = "texture_diffuse";

	std::vector<Texture> textures3;
	textures3.push_back(Texture());
	textures3[0].id = tex3;
	textures3[0].type = "texture_diffuse";

	// ADDED — saloon textures
	std::vector<Texture> saloonTextures;
	saloonTextures.push_back(Texture());
	saloonTextures[0].id = texSaloon;
	saloonTextures[0].type = "texture_diffuse";

	// ADDED — ground textures
	std::vector<Texture> groundTextures;
	groundTextures.push_back(Texture());
	groundTextures[0].id = texGround;
	groundTextures[0].type = "texture_diffuse";

	// ADDED — sky textures
	std::vector<Texture> skyTextures;
	skyTextures.push_back(Texture());
	skyTextures[0].id = texSky;
	skyTextures[0].type = "texture_diffuse";

	Mesh mesh(vert, ind, textures3);

	// Create Obj files - easier :)
	// we can add here our textures :)
	MeshLoaderObj loader;
	Mesh sun = loader.loadObj("Resources/Models/sphere.obj");
	Mesh tumbleweed = loader.loadObj("Resources/Models/sphere.obj", textures2);
	Mesh box = loader.loadObj("Resources/Models/cube.obj", textures);
	// CHANGED — plane now uses ground texture instead of orange
	Mesh plane = loader.loadObj("Resources/Models/plane.obj", groundTextures);

	// ADDED — sky plane
	Mesh skyPlane = loader.loadObj("Resources/Models/plane.obj", skyTextures);

	// ADDED — saloon model
	Mesh saloon = loader.loadObj("Resources/Models/saloon.obj", saloonTextures);

	//check if we close the window or press the escape button
	while (!window.isPressed(GLFW_KEY_ESCAPE) &&
		glfwWindowShouldClose(window.getWindow()) == 0)
	{
		window.clear();
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processKeyboardInput();
		//first pers camera
		while (!window.isPressed(GLFW_KEY_ESCAPE) &&
			glfwWindowShouldClose(window.getWindow()) == 0)
		{
			window.clear();
			float currentFrame = glfwGetTime();
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;

			processKeyboardInput();
			if (firstPerson)
			{
				camera.setCameraPosition(playerPosition);
			}
			else
			{
				camera.setCameraPosition(playerPosition - camera.getCameraViewDirection() * thirdPersonDistance);
			}


		glm::mat4 ViewMatrix;

		if (firstPerson)
		{
			// First person: camera looks where player is facing
			ViewMatrix = glm::lookAt(camera.getCameraPosition(),
				camera.getCameraPosition() + camera.getCameraViewDirection(),
				camera.getCameraUp());
		}
		else
		{
			// Third person: camera is behind the player, look at player
			ViewMatrix = glm::lookAt(camera.getCameraPosition(), camera.getCameraPosition() + camera.getCameraViewDirection(), camera.getCameraUp());


		//test mouse input
		if (window.isMousePressed(GLFW_MOUSE_BUTTON_LEFT))
		{
			std::cout << "Pressing mouse button" << std::endl;
		}
		//// Code for the light ////

		sunShader.use();

		// Move tumbleweed left ? right
		tumbleweedPos.x += tumbleweedSpeed * deltaTime;

		// Rotate it as it rolls
		tumbleweedRotation += 200.0f * deltaTime; // adjust for faster/slower spin

		// Reset when off-screen
		if (tumbleweedPos.x > 300.0f)
		{
			tumbleweedPos.x = -300.0f;
			tumbleweedRotation = 0.0f;
		}

		float groundY = getTerrainHeight(tumbleweedPos.x, tumbleweedPos.z);

		float tumbleweedScale = 1.0f;
		float tumbleweedRadius = 1.0f * tumbleweedScale;

		// Lift the tumbleweed so it doesn’t sink visually
		float extraLift = 17.0f;

		tumbleweedPos.y = groundY + tumbleweedRadius + extraLift;



		ProjectionMatrix = glm::perspective(glm::radians(90.0f),
			window.getWidth() * 1.0f / window.getHeight(),
			0.1f, 10000.0f);

		ViewMatrix = glm::lookAt(camera.getCameraPosition(),
			camera.getCameraPosition() + camera.getCameraViewDirection(),
			camera.getCameraUp());

		MatrixID = glGetUniformLocation(sunShader.getId(), "MVP");
		MatrixID2 = glGetUniformLocation(shader.getId(), "MVP");
		ModelMatrixID = glGetUniformLocation(shader.getId(), "model");


		//Test for one Obj loading = light source

		glm::mat4 ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, lightPos);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		sun.draw(sunShader);

		//// End code for the light ////

		shader.use();

		///// Test Obj files for box ////

		GLuint MatrixID2 = glGetUniformLocation(shader.getId(), "MVP");
		GLuint ModelMatrixID = glGetUniformLocation(shader.getId(), "model");

		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightColor"), lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "viewPos"), camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		box.draw(shader);

		///// Test plane Obj file //////

		// CHANGED — ground is now scaled desert
		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, -20.0f, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(500.0f, 1.0f, 500.0f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		plane.draw(shader);

		// ADDED — draw sky
		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 100.0f, -600.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(800.0f, 1.0f, 400.0f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		skyPlane.draw(shader);

		// ADDED — draw saloon
		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, -20.0f, -80.0f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		saloon.draw(shader);

		// ADDED - draw tumbleweed
		// Draw tumbleweed
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, tumbleweedPos);
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.0f)); // size of sphere
		ModelMatrix = glm::rotate(ModelMatrix, glm::radians(tumbleweedRotation), glm::vec3(0, 0, 1)); // roll around Z axis

		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		tumbleweed.draw(shader);




		window.update();
	}
}

void processKeyboardInput();
 {
	float cameraSpeed = 30 * deltaTime;

	// translation — MOVE PLAYER
	if (window.isPressed(GLFW_KEY_W))
		playerPosition += camera.getCameraViewDirection() * cameraSpeed;
	if (window.isPressed(GLFW_KEY_S))
		playerPosition -= camera.getCameraViewDirection() * cameraSpeed;
	if (window.isPressed(GLFW_KEY_A))
		playerPosition -= glm::normalize(glm::cross(
			camera.getCameraViewDirection(), camera.getCameraUp())) * cameraSpeed;
	if (window.isPressed(GLFW_KEY_D))
		playerPosition += glm::normalize(glm::cross(
			camera.getCameraViewDirection(), camera.getCameraUp())) * cameraSpeed;


	//rotation
	if (window.isPressed(GLFW_KEY_LEFT))
		camera.rotateOy(cameraSpeed);
	if (window.isPressed(GLFW_KEY_RIGHT))
		camera.rotateOy(-cameraSpeed);
	if (window.isPressed(GLFW_KEY_UP))
		camera.rotateOx(cameraSpeed);
	if (window.isPressed(GLFW_KEY_DOWN))
		camera.rotateOx(-cameraSpeed);
}
