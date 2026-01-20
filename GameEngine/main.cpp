#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
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

glm::vec3 lightColor = glm::vec3(1.0f);
glm::vec3 lightPos = glm::vec3(-180.0f, 100.0f, -200.0f);

glm::vec3 tumbleweedPos = glm::vec3(-300.0f, -19.0f, -250.0f);// start left side
float tumbleweedSpeed = 20.0f; // units per second
float tumbleweedRotation = 0.0f; // degrees

bool hasgun = false;
glm::vec3 gunWorldPos = glm::vec3(-80.0f, -15.0f, -80.0f);

bool hasCoin = false;

int ammo = 0;
// ===== AMMO BUILDING TASK (TASK 3) =====
glm::vec3 ammoBuildingPos = glm::vec3(-170.0f, -20.0f, -80.0f);
float ammoBuildingRadius = 60.0f;
bool ammoCollected = false;


glm::vec3 windmillPos = glm::vec3(320.0f, -20.0f, -120.0f); 
float windmillRotation = 0.0f;

glm::vec3 coinPos = glm::vec3(
	windmillPos.x - 10.0f,  
	-20.0f,                 // ground level
	windmillPos.z
);




struct Bullet
{
	glm::vec3 position;
	glm::vec3 direction;
	float speed = 300.0f;
};

std::vector<Bullet> bullets;



glm::vec3 posterPos = glm::vec3(-67.0f, -10.0f, -79.5f);

bool posterRead = false;
// ===== TASK SYSTEM =====
int currentTask = 0;

const char* taskTexts[] = {
	"Task 1: Read the bounty poster (Press R)",
	"Task 2: Pick up the revolver (Press E)",
	"Task 3: Enter the supply building and press G to load ammo",
	 "Task 4: Collect the coin (Press P)"
};




std::vector<glm::vec3> cactusPositions = {
	glm::vec3(100.0f, -20.0f, -200.0f),
	glm::vec3(-120.0f, -20.0f, -250.0f),
	glm::vec3(180.0f, -20.0f, -300.0f),
	glm::vec3(-200.0f, -20.0f, -350.0f)
};



glm::vec3 airplanePos = glm::vec3(0.0f, 200.0f, -300.0f);
float airplaneAngle = 0.0f;


//Mesh tumbleweed = loader.loadObj("Resources/Models/sphere.obj", textures2); 
// textures2 = rock texture, or use a custom tumbleweed texture
float timeOfDay = 0.0f; // increases every frame 
float daySpeed = 0.2f; // how fast the cycle runs
float getTerrainHeight(float x, float z) { 
	return -20.0f;
}

Mesh createCylinder(float radius, float height, int segments)
{
	std::vector<Vertex> vertices;
	std::vector<int> indices;

	for (int i = 0; i <= segments; i++)
	{
		float angle = (float)i / segments * 2.0f * 3.14159f;
		float x = cos(angle) * radius;
		float z = sin(angle) * radius;

		// bottom vertex
		Vertex v1;
		v1.pos = glm::vec3(x, 0.0f, z);
		v1.normals = glm::normalize(glm::vec3(x, 0.0f, z));
		v1.textureCoords = glm::vec2((float)i / segments, 0.0f);
		vertices.push_back(v1);

		// top vertex
		Vertex v2;
		v2.pos = glm::vec3(x, height, z);
		v2.normals = glm::normalize(glm::vec3(x, 0.0f, z));
		v2.textureCoords = glm::vec2((float)i / segments, 1.0f);
		vertices.push_back(v2);
	}

	for (int i = 0; i < segments * 2; i += 2)
	{
		indices.push_back(i);
		indices.push_back(i + 1);
		indices.push_back(i + 2);

		indices.push_back(i + 1);
		indices.push_back(i + 3);
		indices.push_back(i + 2);
	}

	std::vector<Texture> emptyTextures;
	return Mesh(vertices, indices, emptyTextures);
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
	GLuint texairplane = loadBMP("Resources/Textures/moonnou.bmp");
	GLuint texPoster = loadBMP("Resources/Textures/wanted.bmp");



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

	// WINDMILL TEXTURE
	//GLuint texWindmill = loadBMP("assets/models/windmill/.bmp");


	glEnable(GL_DEPTH_TEST);

// INIT IMGUI

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window.getWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 400");

	

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

	std::vector<Texture> airplaneTextures;
	airplaneTextures.push_back(Texture());
	airplaneTextures[0].id = texairplane;
	airplaneTextures[0].type = "texture_diffuse";

	// ADDED — saloon textures
	std::vector<Texture> saloonTextures;
	saloonTextures.push_back(Texture());
	saloonTextures[0].id = texSaloon;
	saloonTextures[0].type = "texture_diffuse";

	// WINDMILL TEXTURES
	std::vector<Texture> windmillTextures;
	windmillTextures.push_back(Texture());
	//windmillTextures[0].id = texWindmill;
	windmillTextures[0].type = "texture_diffuse";


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

	//poster
	std::vector<Texture> posterTextures;
	posterTextures.push_back(Texture());
	posterTextures[0].id = texPoster;
	posterTextures[0].type = "texture_diffuse";


	Mesh mesh(vert, ind, textures3);

	// Create Obj files - easier :)
	// we can add here our textures :)
	MeshLoaderObj loader;
	Mesh windmill = loader.loadObj("assets/models/windmill/woodenwindmill.obj", windmillTextures);
	Mesh coin = loader.loadObj("assets/models/coin/coin.obj");

	//GLuint texCactus = loadBMP("assets/models/cactus/cactus_diffuse.bmp");

	Mesh cactus = loader.loadObj("assets/models/cactus/Cactus_lowpoly.obj");
	Mesh gun = loader.loadObj("assets/models/gun/GUN.obj");



	Mesh sun = loader.loadObj("Resources/Models/sphere.obj");
	Mesh airplane = loader.loadObj("Resources/Models/airplane.obj", airplaneTextures);
	Mesh tumbleweed = loader.loadObj("Resources/Models/sphere.obj", textures2);
	Mesh box = loader.loadObj("Resources/Models/cube.obj", textures);
	// CHANGED — plane now uses ground texture instead of orange
	Mesh plane = loader.loadObj("Resources/Models/plane.obj", groundTextures);
	Mesh poster = loader.loadObj("Resources/Models/plane.obj", posterTextures);


	// Building 1
	Mesh building1 = loader.loadObj(
		"assets/models/building1/casa.obj",
		saloonTextures   // reuse saloon texture for now
	);

	// Building 2
	Mesh building2 = loader.loadObj(
		"assets/models/building2/casa2.obj",
		saloonTextures
	);

	// ADDED — sky plane
	Mesh skyPlane = loader.loadObj("Resources/Models/plane.obj", skyTextures);

	// ADDED — saloon model
	Mesh saloon = loader.loadObj("Resources/Models/saloon.obj", saloonTextures);

	// add cactus mesh
	Mesh cactusBody = createCylinder(5.0f, 30.0f, 20);
	Mesh cactusArm = createCylinder(3.0f, 20.0f, 20);


	//check if we close the window or press the escape button
	while (!window.isPressed(GLFW_KEY_ESCAPE) &&
		glfwWindowShouldClose(window.getWindow()) == 0)
	{
		window.clear();
	
// START IMGUI FRAME

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processKeyboardInput();
		float distToPoster = glm::distance(camera.getCameraPosition(), posterPos);
		bool nearPoster = distToPoster < 15.0f;
		// mark poster as read when player presses R while near it
		if (nearPoster && window.isPressed(GLFW_KEY_R) && currentTask == 0)
		{
			posterRead = true;
			currentTask = 1; // move to next task
		}



		windmillRotation += 50.0f * deltaTime;

		//  COIN PICKUP 
		if (!hasCoin && currentTask == 3) // only after ammo task
		{
			float distToCoin = glm::distance(
				camera.getCameraPosition(),
				glm::vec3(windmillPos.x, -19.0f, windmillPos.z - 20.0f)
			);

			if (distToCoin < 8.0f && window.isPressed(GLFW_KEY_P))
			{
				hasCoin = true;
				currentTask = 4; // task 4 complete
			}
		}

		// ===== GUN PICKUP LOGIC =====
		float distToGun = glm::distance(
			camera.getCameraPosition(),
			gunWorldPos
		);

		if (!hasgun && distToGun < 10.0f && currentTask == 1)
		{
			if (window.isPressed(GLFW_KEY_E))
			{
				hasgun = true;
				currentTask = 2; // task 2 complete

				glm::vec3 camPos = camera.getCameraPosition();
				camPos.y = -15.0f;
				camera.setCameraPosition(camPos);
			}
		}
		// ===== TASK 3: ENTER BUILDING AND LOAD AMMO =====
		float distToAmmoBuilding = glm::distance(
			camera.getCameraPosition(),
			ammoBuildingPos
		);

		bool insideAmmoBuilding = distToAmmoBuilding < ammoBuildingRadius;

		if (currentTask == 2 && insideAmmoBuilding && !ammoCollected)
		{
			if (window.isPressed(GLFW_KEY_G))
			{
				ammo = 7;                 // refill ammo
				ammoCollected = true;     // prevent reloading forever
				currentTask = 3;          // move to next task
			}
		}



		// ===== SHOOT BULLET =====
		if (hasgun && ammo > 0 && window.isMousePressed(GLFW_MOUSE_BUTTON_LEFT))
		{
			Bullet b;
			b.position = camera.getCameraPosition();
			b.direction = glm::normalize(camera.getCameraViewDirection());
			b.speed = 300.0f;

			bullets.push_back(b);
			ammo--;
		}


		
		// ===== UPDATE BULLETS =====
		for (int i = 0; i < bullets.size(); i++)
		{
			bullets[i].position += bullets[i].direction * bullets[i].speed * deltaTime;

			// remove bullet if too far
			if (glm::length(bullets[i].position - camera.getCameraPosition()) > 500.0f)
			{
				bullets.erase(bullets.begin() + i);
				i--;
			}
		}

	
// UPDATE DAY–NIGHT CYCLE

		timeOfDay += daySpeed * deltaTime;

		// keep value between 0 and 2π
		if (timeOfDay > 6.2831f)
			timeOfDay = 0.0f;

		// brightness goes from 0 (night) to 1 (day)
		float brightness = (sin(timeOfDay) + 1.0f) / 2.0f;

		// airplane ANIMATION
		airplaneAngle += 0.3f * deltaTime;

		float airplaneRadius = 500.0f; 
		airplanePos.x = cos(airplaneAngle) * airplaneRadius;
		airplanePos.z = sin(airplaneAngle) * airplaneRadius;
		airplanePos.y = 200.0f;

		// update light color (warm day → blue night)
		lightColor = glm::vec3(
			0.8f * brightness + 0.2f,   // red
			0.7f * brightness + 0.2f,   // green
			1.0f * (1.0f - brightness)  // blue tint at night
		);

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


				glm::mat4 ProjectionMatrix = glm::perspective(90.0f, window.getWidth() * 1.0f / window.getHeight(), 0.1f, 10000.0f);
				glm::mat4 ViewMatrix = glm::lookAt(camera.getCameraPosition(), camera.getCameraPosition() + camera.getCameraViewDirection(), camera.getCameraUp());

				glm::mat4 ModelMatrix;
				glm::mat4 MVP;

				GLuint MatrixID = glGetUniformLocation(sunShader.getId(), "MVP");

				

				//Test for one Obj loading = light source

			    ModelMatrix = glm::mat4(1.0);
				ModelMatrix = glm::translate(ModelMatrix, lightPos);
			    MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
				glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

				sun.draw(sunShader);

				//// End code for the light ////
				shader.use();

				GLuint MatrixID2 = glGetUniformLocation(shader.getId(), "MVP");
				GLuint ModelMatrixID = glGetUniformLocation(shader.getId(), "model");

				ModelMatrix = glm::mat4(1.0f);
				ModelMatrix = glm::translate(ModelMatrix, airplanePos);
				ModelMatrix = glm::scale(ModelMatrix, glm::vec3(6.0f));
				
				MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

				glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
				glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
				
				airplane.draw(shader);
				

				///// Test Obj files for box ////
				

				ModelMatrix = glm::mat4(1.0);
				ModelMatrix = glm::translate(ModelMatrix, glm::vec3(50.0f, -20.0f, -50.0f));
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
				// OPTIONAL — sky tint based on time of day
				glm::vec3 skyTint = glm::vec3(
					0.4f * brightness + 0.1f,
					0.5f * brightness + 0.1f,
					1.0f
				);

				glUniform3f(glGetUniformLocation(shader.getId(), "skyTint"),
					skyTint.x, skyTint.y, skyTint.z);

				skyPlane.draw(shader);

				// ADDED — draw saloon
				ModelMatrix = glm::mat4(1.0);
				ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, -20.0f, -80.0f));
				MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
				glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
				glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

				saloon.draw(shader);

				// ===== DRAW WINDMILL =====
			    ModelMatrix = glm::mat4(1.0f);
				ModelMatrix = glm::translate(ModelMatrix, windmillPos);
				ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.07f)); 
				ModelMatrix = glm::rotate(
					ModelMatrix,
					glm::radians(windmillRotation),
					glm::vec3(0, 1, 0) // rotate around Y axis
				);
			
				glUniform3f(
					glGetUniformLocation(shader.getId(), "objectColor"),
					0.78f, 0.52f, 0.30f
				);

                MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
				glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
				glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

			

				windmill.draw(shader);

				// ===== DRAW COIN =====
				if (!hasCoin)
				{
					ModelMatrix = glm::mat4(1.0f);

					
					ModelMatrix = glm::translate(
						ModelMatrix,
						glm::vec3(
							windmillPos.x,      
							-19.0f,            
							windmillPos.z - 20.0f  
						)
					);

					
					ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.3f));

					// stand upright
					ModelMatrix = glm::rotate(
						ModelMatrix,
						glm::radians(90.0f),
						glm::vec3(1, 0, 0)
					);

					MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
					glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
					glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

					// gold color
					glUniform3f(glGetUniformLocation(shader.getId(), "objectColor"), 1.0f, 0.84f, 0.0f);

					coin.draw(shader);
				}
				

				// ===== DRAW GUN ON GROUND =====
				if (!hasgun)
				{
				   
					ModelMatrix = glm::mat4(1.0f);
					ModelMatrix = glm::translate(ModelMatrix, gunWorldPos);
					ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.2f));
					ModelMatrix = glm::rotate(ModelMatrix, glm::radians(90.0f), glm::vec3(1, 0, 0));

					glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

					glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
					glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

					gun.draw(shader);
				}


				// ===== draw wanted poster =====
				ModelMatrix = glm::mat4(1.0f);

				// Position on wall
				ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-67.0f, -10.0f, -79.5f));

				// Make it small
				ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.05f, 0.05f, 0.05f));

				// FIX: rotate plane upright
				ModelMatrix = glm::rotate(ModelMatrix, glm::radians(5000.0f), glm::vec3(0, 0, 1));

				// Face camera
				ModelMatrix = glm::rotate(ModelMatrix, glm::radians(180.0f), glm::vec3(0, 1, 0));

				MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

				glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
				glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

				poster.draw(shader);



				// ===== draw building 1 =====
				ModelMatrix = glm::mat4(1.0f);
				ModelMatrix = glm::translate(
					ModelMatrix,
					glm::vec3(170.0f, -20.0f, -80.0f) // right side
				);
				ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.8f));

				MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
				glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
				glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
				building1.draw(shader);


				// ===== draw building 2 =====
				ModelMatrix = glm::mat4(1.0f);
				ModelMatrix = glm::translate(
					ModelMatrix,
					glm::vec3(-170.0f, -20.0f, -80.0f) // left side
				);
				ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.7f));

				MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
				glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
				glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
				building2.draw(shader);

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

				// ===== DRAW BULLETS =====
				for (auto& b : bullets)
				{
					ModelMatrix = glm::mat4(1.0f);
					ModelMatrix = glm::translate(ModelMatrix, b.position);
					ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.5f));

					glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
					glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
					glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

					box.draw(shader); // reuse cube as bullet
				}


				// added - draw cacti
				for (auto& pos : cactusPositions)
				{
					float cactusBaseOffset = -15.0f; // half the cactus height

					glm::vec3 adjustedPos = pos + glm::vec3(0.0f, cactusBaseOffset, 0.0f);

					// MAIN BODY
					glm::mat4 ModelMatrix = glm::mat4(1.0f);
					ModelMatrix = glm::translate(ModelMatrix, adjustedPos);
					ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.0f));

					glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
					glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
					glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

					cactusBody.draw(shader);

					// LEFT ARM
					glm::mat4 armL = glm::mat4(1.0f);
					armL = glm::translate(armL, adjustedPos + glm::vec3(-5.0f, 15.0f, 0.0f));
					armL = glm::rotate(armL, glm::radians(90.0f), glm::vec3(0, 0, 1));

					MVP = ProjectionMatrix * ViewMatrix * armL;
					glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
					glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &armL[0][0]);

					cactusArm.draw(shader);

					// RIGHT ARM
					glm::mat4 armR = glm::mat4(1.0f);
					armR = glm::translate(armR, adjustedPos + glm::vec3(5.0f, 15.0f, 0.0f));
					armR = glm::rotate(armR, glm::radians(-90.0f), glm::vec3(0, 0, 1));

					MVP = ProjectionMatrix * ViewMatrix * armR;
					glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
					glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &armR[0][0]);

					cactusArm.draw(shader);
				}



			
// GUI WINDOW

				ImGui::Begin("Wild West Controls");
				ImGui::Separator();
				ImGui::Text("Current Objective:");
				if (currentTask < 4)
					ImGui::Text("%s", taskTexts[currentTask]);
				else
					ImGui::Text("All tasks complete!");

				ImGui::Separator();
				// ===== TASK 3 GUI PROMPT =====
				if (currentTask == 2 && insideAmmoBuilding && !ammoCollected)
				{
					ImGui::Text("Press G to load ammo");
				}
				//  TASK 4 GUI PROMPT 
				if (currentTask == 3 && !hasCoin)
				{
					float distToCoin = glm::distance(camera.getCameraPosition(), coinPos);
					if (distToCoin < 8.0f)
					{
						ImGui::Text("Press P to collect the coin");
					}
				}

				// ===== DRAW GUN IN HAND =====
				if (hasgun)
				{
					glm::vec3 camPos = camera.getCameraPosition();
					glm::vec3 camDir = camera.getCameraViewDirection();
					glm::vec3 camRight = glm::normalize(glm::cross(camDir, camera.getCameraUp()));
					glm::vec3 camUp = camera.getCameraUp();

					glm::vec3 gunPos =
						camPos +
						camDir * 5.0f +
						camRight * 2.0f -
						camUp * 1.5f;

					glm::mat4 ModelMatrix = glm::mat4(1.0f);
					ModelMatrix = glm::translate(ModelMatrix, gunPos);
					ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.6f));
					ModelMatrix = glm::rotate(ModelMatrix, glm::radians(-90.0f), glm::vec3(0, 1, 0));

					glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

					glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
					glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

					gun.draw(shader);
				}


				static bool skyBlue = true;

				if (ImGui::Button("Toggle Sky Color"))
				{
					skyBlue = !skyBlue;

					if (skyBlue)
						glClearColor(0.2f, 0.8f, 1.0f, 1.0f);   // day sky
					else
						glClearColor(0.0f, 0.0f, 0.1f, 1.0f);   // night sky
				}

				ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
				ImGui::Text("Ammo: %d / 7", ammo);

				if (currentTask == 0 && nearPoster)
				{
					ImGui::Text("Press R to read the poster");
				}





				ImGui::End();

				
				// RENDER IMGUI
				
				ImGui::Render();
				ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());




				window.update();
	}

	
	// SHUTDOWN IMGUI

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	return 0;
}


void processKeyboardInput()
{
	float cameraSpeed = 30.0f * deltaTime;

	
if (window.isPressed(GLFW_KEY_W))
		camera.keyboardMoveFront(cameraSpeed);
if (window.isPressed(GLFW_KEY_S))
		camera.keyboardMoveBack(cameraSpeed);
	if (window.isPressed(GLFW_KEY_A))
	camera.keyboardMoveLeft(cameraSpeed);
	if (window.isPressed(GLFW_KEY_D))
		camera.keyboardMoveRight(cameraSpeed);
	//if (window.isPressed(GLFW_KEY_R))
	//	camera.keyboardMoveUp(cameraSpeed);
	//if (window.isPressed(GLFW_KEY_F))
	//	camera.keyboardMoveDown(cameraSpeed);


	if (window.isPressed(GLFW_KEY_LEFT))
		camera.rotateOy(cameraSpeed);

	if (window.isPressed(GLFW_KEY_RIGHT))
		camera.rotateOy(-cameraSpeed);

	if (window.isPressed(GLFW_KEY_UP))
		camera.rotateOx(cameraSpeed);

	if (window.isPressed(GLFW_KEY_DOWN))
		camera.rotateOx(-cameraSpeed);
	// ===== LOCK CAMERA TO GROUND://///oof =====

// get current camera position
	glm::vec3 camPos = camera.getCameraPosition();

	// ground and eye height
	float groundY = -20.0f;   // same Y as your ground
	float eyeHeight = 5.0f;   // camera height above ground

	// prevent falling below ground
	if (camPos.y < groundY + eyeHeight)
	{
		camPos.y = groundY + eyeHeight;
		camera.setCameraPosition(camPos);
	}
	camera.setCameraPosition(glm::vec3(camera.getCameraPosition().x, -15.0f, camera.getCameraPosition().z));



}
		

		

	
