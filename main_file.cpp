/*
Niniejszy program jest wolnym oprogramowaniem; możesz go
rozprowadzać dalej i / lub modyfikować na warunkach Powszechnej
Licencji Publicznej GNU, wydanej przez Fundację Wolnego
Oprogramowania - według wersji 2 tej Licencji lub(według twojego
wyboru) którejś z późniejszych wersji.

Niniejszy program rozpowszechniany jest z nadzieją, iż będzie on
użyteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domyślnej
gwarancji PRZYDATNOŚCI HANDLOWEJ albo PRZYDATNOŚCI DO OKREŚLONYCH
ZASTOSOWAŃ.W celu uzyskania bliższych informacji sięgnij do
Powszechnej Licencji Publicznej GNU.

Z pewnością wraz z niniejszym programem otrzymałeś też egzemplarz
Powszechnej Licencji Publicznej GNU(GNU General Public License);
jeśli nie - napisz do Free Software Foundation, Inc., 59 Temple
Place, Fifth Floor, Boston, MA  02110 - 1301  USA
*/

#define GLM_FORCE_CTOR_INIT
#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE

#include <stdlib.h>
#include <map>
#include <iostream>
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include <mciapi.h>
#include <fstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "constants.h"
#include "shaderprogram.h"
#include "Model.h"
#include "Car.h"
#include "Plane.h"
#pragma comment(lib, "winmm.lib")


std::fstream fs;

Car Player("Models/Formula.fbx");
Model Sphere("Models/Sphere.fbx");
Model Grass("Models/Grass.fbx", 200);
Model Track("Models/Track.fbx", 50);

std::vector<Plane> Planes = {
	{ Plane("Models/Plane.fbx", 25000.0f, 5000.0f, 0.0059f) },
	{ Plane("Models/Plane.fbx", 35000.0f, 4000.0f, -0.0078f) },
};

std::vector<Model> Trees = {};

std::vector<Model> Bands = {
	{ Model("Models/Grass.fbx", 0.0f, 0.0f, -7500.0f, 36000.0f, 150.0f, 5.0f, 500) },
	{ Model("Models/Grass.fbx", 0.0f, 0.0f, 7500.0f, 36000.0f, 150.0f, 5.0f, 500) },
	{ Model("Models/Grass.fbx", -18000.0f, 0.0f, 0.0f, 5.0f, 150.0f, 15000.0f, 208) },
	{ Model("Models/Grass.fbx", 18000.0f, 0.0f, 0.0f, 5.0f, 150.0f, 15000.0f, 208) }
};

glm::mat4 M_Skybox = glm::mat4(1.0f);
glm::mat4 M_Grass = glm::mat4(1.0f);
glm::mat4 M_Track = glm::mat4(1.0f);
glm::mat4 M_Plane = glm::mat4(1.0f);

std::vector<glm::mat4> Ms_Trees = {};

std::vector<glm::mat4> Ms_Bands = {
	{ glm::mat4(1.0f) },
	{ glm::mat4(1.0f) },
	{ glm::mat4(1.0f) },
	{ glm::mat4(1.0f) }
};

std::vector<std::string> music = {
	{ "Drum 'n base - PolishKiddo" },
	{ "boombap - PolishKiddo" },
	{ "Empty roads - PolishKiddo" },
};

int music_iter = 0;
bool music_key_press = false;
float aspect_ratio = 1;
int camera_control = 1;


//Procedura obsługi błędów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}


void keyCallback(GLFWwindow* window,int key,int scancode,int action,int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_DOWN) camera_control = -1;
		if (key == GLFW_KEY_M && !music_key_press) {
			music_iter = (music_iter + 1) % music.size();
			PlaySoundA(("Sounds/" + music[music_iter] + ".wav").c_str(), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
			music_key_press = true;
		}
    }
    if (action==GLFW_RELEASE) {
        if (key == GLFW_KEY_DOWN) camera_control = 1;
		if (key == GLFW_KEY_M) music_key_press = false;
    }
}

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    if (height == 0) return;
    aspect_ratio = (float)width / (float)height;
    glViewport(0, 0, width, height);
}


//Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {
	//************Tutaj umieszczaj kod, który należy wykonać raz, na początku programu************
	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
	glfwSetWindowSizeCallback(window, windowResizeCallback);
	glfwSetKeyCallback(window, keyCallback);

	// generate random trees
	for (int i = -20000; i <= 20000; i += 500) {
		float size = (float)(rand() % 100) / 100 + 1.0f;
		Trees.push_back(Model("Models/Tree.fbx", (float)i + rand() % 400 - 200, 0.0f, -9500.0f, size, size, size, 50));
		Ms_Trees.push_back(glm::mat4(1.0f));
		size = (float)(rand() % 100) / 100 + 1.0f;
		Trees.push_back(Model("Models/Tree.fbx", (float)i + rand() % 400 - 200, 0.0f, 9500.0f, size, size, size, 50));
		Ms_Trees.push_back(glm::mat4(1.0f));
	}
	for (int i = -9000; i <= 9000; i += 500) {
		float size = (float)(rand() % 100) / 100 + 1.0f;
		Trees.push_back(Model("Models/Tree.fbx", -20000.0f, 0.0f, (float)i + rand() % 400 - 200, size, size, size, 50));
		Ms_Trees.push_back(glm::mat4(1.0f));
		size = (float)(rand() % 100) / 100 + 1.0f;
		Trees.push_back(Model("Models/Tree.fbx", 20000.0f, 0.0f, (float)i + rand() % 400 - 200, size, size, size, 50));
		Ms_Trees.push_back(glm::mat4(1.0f));
	}

	sp = new ShaderProgram("v_simplest.glsl", NULL, "f_simplest.glsl");
	Player.readTexture("Textures/Formula.png");
	Sphere.readTexture("Textures/Sphere.png");
	Grass.readTexture("Textures/Grass.png");
	Track.readTexture("Textures/Track.png");

	for (int i = 0; i < Planes.size(); i++) {
		Planes[i].readTexture("Textures/Plane" + std::to_string(i + 1) + ".png");
	}

	for (int i = 0; i < Trees.size(); i++) {
		Trees[i].readTexture("Textures/Tree.png");
		Ms_Trees[i] = glm::translate(Ms_Trees[i], glm::vec3(Trees[i].x, Trees[i].y, Trees[i].z));
		Ms_Trees[i] = glm::scale(Ms_Trees[i], glm::vec3(Trees[i].size_x, Trees[i].size_y, Trees[i].size_z));
	}

	for (int i = 0; i < Bands.size(); i++) {
		Bands[i].readTexture("Textures/Band.png");
		Ms_Bands[i] = glm::translate(Ms_Bands[i], glm::vec3(Bands[i].x, Bands[i].y, Bands[i].z));
		Ms_Bands[i] = glm::scale(Ms_Bands[i], glm::vec3(Bands[i].size_x * 0.5, Bands[i].size_y, Bands[i].size_z * 0.5));
	}

	M_Skybox = glm::scale(M_Skybox, glm::vec3(2000.0f, 2000.0f, 2000.0f));
	M_Grass = glm::scale(M_Grass, glm::vec3(25000.0f, 1.0f, 25000.0f));

	M_Track = glm::scale(M_Track, glm::vec3(100.0f, 1.0f, 100.0f));
	M_Track = glm::translate(M_Track, glm::vec3(0.0f, 0.0f, -25.0f));
	M_Track = glm::rotate(M_Track, -PI / 2, glm::vec3(1.0f, 0.0f, 0.0f));

	Player.x = 8000.0f;
	Player.z = -5000.0f;

	PlaySoundA(("Sounds/" + music[music_iter] + ".wav").c_str(), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
}


//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
    //************Tutaj umieszczaj kod, który należy wykonać po zakończeniu pętli głównej************
	std::cout << std::endl << "Thank you for playing!" << std::endl;
	std::cout << "Programming: Szymon Stanislawski - github.com/Hi-Im-Simon" << std::endl;
	std::cout << "Music: Mariusz Duszczak - https://www.youtube.com/channel/UCWVcDkZZsKus6dv2nNpIEmQ" << std::endl;
    delete sp;
}


//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window) {
	//************Tutaj umieszczaj kod rysujący obraz******************l
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear buffer

	glm::mat4 V = glm::lookAt(
		glm::vec3(
			Player.x + (450.0f * glm::cos(Player.angular_displacement) * camera_control),
			Player.y + 150.0f,
			Player.z - (450.0f * glm::sin(Player.angular_displacement) * camera_control)
		), // camera position (eye)
		glm::vec3(
			Player.x,
			Player.y + 100.0f,
			Player.z
		), // camera lookat point (center)
		glm::vec3(0.0f, 1.0f, 0.0f) // where is up
	);

	/*std::cout << Car.linear_speed << Car.angular_speed << std::endl;*/
	
    glm::mat4 P = glm::perspective(
		(50.0f*PI) / 180.0f, // FoV
		aspect_ratio, // window (width/height)
		100.0f, // near clipping plane
		500000.0f // far clipping plane
	);

	sp->use();//Aktywacja programu cieniującego

	Player.drawModel(P, V);

	float reflectPow = 0.4;
	
	Sphere.drawModel(P, V, M_Skybox, 1.0, 1.0);
	Grass.drawModel(P, V, M_Grass, reflectPow, reflectPow);
	Track.drawModel(P, V, M_Track, reflectPow, reflectPow);

	for (int i = 0; i < Planes.size(); i++) {
		Planes[i].drawModel(P, V);
	}

	for (int i = 0; i < Trees.size(); i++) {
		Trees[i].drawModel(P, V, Ms_Trees[i], 0.0, 0.0);
	}
	

	for (int i = 0; i < Bands.size(); i++) {
		Bands[i].drawModel(P, V, Ms_Bands[i], reflectPow, reflectPow);
	}

    glfwSwapBuffers(window); // swap back buffer to front
}


int main(void) {
	std::cout << "Welcome!" << std::endl;
	std::cout << "- W, S, A, D - steering" << std::endl;
	std::cout << "- SPACE - hand brake" << std::endl;
	std::cout << "- ARROW DOWN - look behind" << std::endl;
	std::cout << "- M - change music" << std::endl << std::endl;

	GLFWwindow* window;

	glfwSetErrorCallback(error_callback); // init error callback
	if (!glfwInit()) { //Zainicjuj bibliotekę GLFW
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}
	window = glfwCreateWindow(500, 500, "3D Racing game", NULL, NULL);  // create window
	if (!window) {
		fprintf(stderr, "Nie można utworzyć okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje się aktywny i polecenia OpenGL będą dotyczyć właśnie jego.
	glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Can't init GLEW.\n");
		exit(EXIT_FAILURE);
	}
	initOpenGLProgram(window);
	glfwMaximizeWindow(window);

	glfwSetTime(0); // timer reset
	while (!glfwWindowShouldClose(window)) {
		Player.readInput(window);
		Player.checkCollision(Bands);

		glfwSetTime(0);
		drawScene(window);
		glfwPollEvents(); // Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
	}

	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	exit(EXIT_SUCCESS);
}
