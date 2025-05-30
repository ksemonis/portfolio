///////////////////////////////////////////////////////////////////////////////
// shadermanager.cpp
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager* pShaderManager)
{
	m_pShaderManager = pShaderManager;
	// create the shape meshes object
	m_basicMeshes = new ShapeMeshes();

	// Initialize texture collection
	for (int i = 0; i < 16; i++)
	{
		m_textureIDs[i].tag = "/0";
		m_textureIDs[i].ID = -1;
	}
	m_loadedTextures = 0;
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	// free the allocated objects
	m_pShaderManager = NULL;
	if (NULL != m_basicMeshes)
	{
		delete m_basicMeshes;
		m_basicMeshes = NULL;
	}

	// free the allocated OpenGL textures
	DestroyGLTextures();
}


/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationX * rotationY * rotationZ * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
			m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

void SceneManager::DefineObjectMaterials()
{
	OBJECT_MATERIAL plasticMaterial;
	plasticMaterial.ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);
	plasticMaterial.ambientStrength = 0.4f;
	plasticMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.1f);
	plasticMaterial.specularColor = glm::vec3(0.5f, 0.5f, 0.5f);
	plasticMaterial.shininess = 30.0;
	plasticMaterial.tag = "plastic";
	m_objectMaterials.push_back(plasticMaterial);

	OBJECT_MATERIAL cementMaterial;
	cementMaterial.ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);
	cementMaterial.ambientStrength = 0.2f;
	cementMaterial.diffuseColor = glm::vec3(0.5f, 0.5f, 0.5f);
	cementMaterial.specularColor = glm::vec3(0.4f, 0.4f, 0.4f);
	cementMaterial.shininess = 0.5;
	cementMaterial.tag = "cement";
	m_objectMaterials.push_back(cementMaterial);

	OBJECT_MATERIAL woodMaterial;
	woodMaterial.ambientColor = glm::vec3(0.4f, 0.3f, 0.1f);
	woodMaterial.ambientStrength = 0.2f;
	woodMaterial.diffuseColor = glm::vec3(0.3f, 0.2f, 0.1f);
	woodMaterial.specularColor = glm::vec3(0.1f, 0.1f, 0.1f);
	woodMaterial.shininess = 0.3;
	woodMaterial.tag = "wood";
	m_objectMaterials.push_back(woodMaterial);

	OBJECT_MATERIAL glassMaterial;
	glassMaterial.ambientColor = glm::vec3(0.4f, 0.4f, 0.4f);
	glassMaterial.ambientStrength = 0.3f;
	glassMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.3f);
	glassMaterial.specularColor = glm::vec3(0.6f, 0.6f, 0.6f);
	glassMaterial.shininess = 85.0;
	glassMaterial.tag = "glass";
	m_objectMaterials.push_back(glassMaterial);

	OBJECT_MATERIAL clayMaterial;
	clayMaterial.ambientColor = glm::vec3(0.2f, 0.2f, 0.3f);
	clayMaterial.ambientStrength = 0.3f;
	clayMaterial.diffuseColor = glm::vec3(0.4f, 0.4f, 0.5f);
	clayMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.4f);
	clayMaterial.shininess = 0.5;
	clayMaterial.tag = "clay";
	m_objectMaterials.push_back(clayMaterial);

	OBJECT_MATERIAL tileMaterial;
	tileMaterial.ambientColor = glm::vec3(0.2f, 0.3f, 0.4f);
	tileMaterial.ambientStrength = 0.3f;
	tileMaterial.diffuseColor = glm::vec3(0.3f, 0.2f, 0.1f);
	tileMaterial.specularColor = glm::vec3(0.4f, 0.5f, 0.6f);
	tileMaterial.shininess = 25.0;
	tileMaterial.tag = "tile";
	m_objectMaterials.push_back(tileMaterial);
}

void SceneManager::SetupSceneLights()
{
	m_pShaderManager->setVec3Value("lightSources[0].position", 0.0f, 5.0f, -1.0f);
	m_pShaderManager->setVec3Value("lightSources[0].ambientColor", 0.4f, 0.4f, 0.4f);
	m_pShaderManager->setVec3Value("lightSources[0].diffuseColor", 0.4f, 0.4f, 0.4f);
	m_pShaderManager->setVec3Value("lightSources[0].specularColor", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setFloatValue("lightSources[0].focalStrength", 32.0f);
	m_pShaderManager->setFloatValue("lightSources[0].specularIntensity", 0.05f);

	m_pShaderManager->setVec3Value("lightSources[1].position", -5.0f, 7.0f, 5.0f);
	m_pShaderManager->setVec3Value("lightSources[1].ambientColor", 0.1f, 0.1f, 0.4f);
	m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", 0.1f, 0.1f, 0.4f);
	m_pShaderManager->setVec3Value("lightSources[1].specularColor", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setFloatValue("lightSources[1].focalStrength", 32.0f);
	m_pShaderManager->setFloatValue("lightSources[1].specularIntensity", 0.05f);


	m_pShaderManager->setBoolValue("bUseLighting", true);
}
/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/

void SceneManager::LoadSceneTextures()
{
	bool bReturn = false;

	bReturn = CreateGLTexture(
		"../../Utilities/textures/plastic.jpg",
		"whiteplastic");

	bReturn = CreateGLTexture(
		"../../Utilities/textures/stainless.jpg",
		"stainless");

	bReturn = CreateGLTexture(
		"../../Utilities/textures/bplastic.jpg",
		"blackplastic");
	
	bReturn = CreateGLTexture(
		"../../Utilities/textures/4838099530_7777bf4981_b.jpg",
		"bluewall");

	bReturn = CreateGLTexture(
		"../../Utilities/textures/ceramic.jpg",
		"ceramic");

	bReturn = CreateGLTexture(
		"../../Utilities/textures/carpet.jpg",
		"carpet");

	bReturn = CreateGLTexture(
		"../../Utilities/textures/wood.jpg",
		"wood");

	bReturn = CreateGLTexture(
		"../../Utilities/textures/dirt.jpg",
		"dirt");

	BindGLTextures();
}
/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene
	LoadSceneTextures();
	DefineObjectMaterials();
	SetupSceneLights();

	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadConeMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadSphereMesh();
	m_basicMeshes->LoadPrismMesh();
	m_basicMeshes->LoadPyramid3Mesh();
	m_basicMeshes->LoadPyramid4Mesh();
	m_basicMeshes->LoadTaperedCylinderMesh();
	m_basicMeshes->LoadTorusMesh();
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by 
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(7.0f, 1.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 0.0f, 5.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(1, 1, 1, 1);
	SetShaderTexture("carpet");
	SetShaderMaterial("clay");

	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
	/****************************************************************/

	//Background plane
	scaleXYZ = glm::vec3(4.0f, 1.0f, 9.0f);

	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	positionXYZ = glm::vec3(0.0f, 0.0f, -0.5f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(1.0f, 0.0f, 0.0f, 1.0f);
	SetShaderTexture("bluewall");
	SetShaderMaterial("plastic");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawPlaneMesh();

	//Wall to left of bath
	scaleXYZ = glm::vec3(5.0f, 0.1f, 9.0f);

	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	positionXYZ = glm::vec3(-4.0f, 4.5f, 2.0f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(1.0f, 0.0f, 0.0f, 1.0f);
	SetShaderTexture("bluewall");
	SetShaderMaterial("plastic");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawBoxMesh();
	
	//Wall to right of bath
	scaleXYZ = glm::vec3(5.0f, 0.1f, 9.0f);

	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	positionXYZ = glm::vec3(4.0f, 4.5f, 2.0f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(1.0f, 0.0f, 0.0f, 1.0f);
	SetShaderTexture("bluewall");
	SetShaderMaterial("plastic");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawBoxMesh();

	//Front-facing wall to left of bath

	scaleXYZ = glm::vec3(3.0f, 0.1f, 9.0f);

	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	positionXYZ = glm::vec3(-5.5f, 4.5f, 4.45f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(1.0f, 0.0f, 0.0f, 1.0f);
	SetShaderTexture("bluewall");
	SetShaderMaterial("plastic");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawBoxMesh();

	//Front-facing wall to right of bath

	scaleXYZ = glm::vec3(3.0f, 0.1f, 9.0f);

	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	positionXYZ = glm::vec3(5.5f, 4.5f, 4.45f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(1.0f, 0.0f, 0.0f, 1.0f);
	SetShaderTexture("bluewall");
	SetShaderMaterial("plastic");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawBoxMesh();

	//Main wall to right
	scaleXYZ = glm::vec3(10.0f, 0.1f, 9.0f);

	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	positionXYZ = glm::vec3(7.0f, 4.5f, 9.5f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(1.0f, 0.0f, 0.0f, 1.0f);
	SetShaderTexture("bluewall");
	SetShaderMaterial("plastic");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawBoxMesh();

	//Main wall to left
	scaleXYZ = glm::vec3(10.0f, 0.1f, 9.0f);

	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	positionXYZ = glm::vec3(-7.0f, 4.5f, 9.5f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(1.0f, 0.0f, 0.0f, 1.0f);
	SetShaderTexture("bluewall");
	SetShaderMaterial("plastic");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawBoxMesh();



	//Adds bath shapes

	//Front long-side of bath
	scaleXYZ = glm::vec3(8.0f, 4.0f, 0.4f);

	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	positionXYZ = glm::vec3(0.0f, 0.0f, 3.0f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(1.0f, 0.0f, 0.0f, 1.0f);
	SetShaderTexture("whiteplastic");
	SetShaderMaterial("plastic");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawBoxMesh();

	//Back long-side of bath
	scaleXYZ = glm::vec3(8.0f, 4.0f, 0.4f);

	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.0f, 1.0f, 0.0f, 1.0f);
	SetShaderTexture("whiteplastic");
	SetShaderMaterial("plastic");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawBoxMesh();

	//Left short-side of bath
	scaleXYZ = glm::vec3(3.0f, 4.0f, 0.4f);

	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;

	positionXYZ = glm::vec3(-3.8f, 0.0f, 1.5f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.0f, 0.0f, 1.0f, 1.0f);
	SetShaderTexture("whiteplastic");
	SetShaderMaterial("plastic");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawBoxMesh();

	//Right short-side of bath
	scaleXYZ = glm::vec3(3.0f, 4.0f, 0.4f);

	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;

	positionXYZ = glm::vec3(3.8f, 0.0f, 1.5f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.0f, 0.0f, 1.0f, 1.0f);
	SetShaderTexture("whiteplastic");
	SetShaderMaterial("plastic");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawBoxMesh();

	//Curtain Rod
	scaleXYZ = glm::vec3(0.1f, 9.0f, 0.1f);

	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	positionXYZ = glm::vec3(4.5f, 8.0f, 3.5f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.0f, 0.0f, 1.0f, 1.0f);
	SetShaderTexture("blackplastic");
	SetShaderMaterial("plastic");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawCylinderMesh();

	//Showerhead base
	scaleXYZ = glm::vec3(0.1f, 0.4f, 0.1f);

	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	positionXYZ = glm::vec3(-3.5f, 7.0f, 0.6f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.0f, 0.0f, 1.0f, 1.0f);
	SetShaderTexture("stainless");
	SetShaderMaterial("glass");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawCylinderMesh();

	//Showerhead midsection
	scaleXYZ = glm::vec3(0.1f, 0.4f, 0.1f);

	XrotationDegrees = 45.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 85.0f;

	positionXYZ = glm::vec3(-3.15f, 6.9f, 0.6f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.0f, 1.0f, 1.0f, 1.0f);
	SetShaderTexture("stainless");
	SetShaderMaterial("glass");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawCylinderMesh();

	//Actual Showerhead
	scaleXYZ = glm::vec3(0.5f, 0.5f, 0.5f);

	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 60.0f;

	positionXYZ = glm::vec3(-2.92f, 6.6f, 0.6f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(1.0f, 0.0f, 1.0f, 1.0f);
	SetShaderTexture("stainless");
	SetShaderMaterial("glass");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawConeMesh();
	///bath///



	//Adds toilet shapes

	//Base of toilet
	scaleXYZ = glm::vec3(1.5f, 0.8f, 0.8f);

	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	positionXYZ = glm::vec3(5.4f, 0.0f, 8.0f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(1.0f, 0.0f, 1.0f, 1.0f);
	SetShaderTexture("ceramic");
	SetShaderMaterial("glass");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawTaperedCylinderMesh();

	//Bowl of toilet
	scaleXYZ = glm::vec3(1.5f, 1.2f, 0.8f);

	XrotationDegrees = 180.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	positionXYZ = glm::vec3(5.4f, 2.0f, 8.0f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(1.0f, 0.0f, 1.0f, 1.0f);
	SetShaderTexture("ceramic");
	SetShaderMaterial("glass");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawTaperedCylinderMesh();

	//Back of toilet
	scaleXYZ = glm::vec3(0.9f, 2.7f, 1.4f);

	XrotationDegrees = 180.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	positionXYZ = glm::vec3(6.4f, 3.0f, 8.0f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(1.0f, 0.0f, 1.0f, 1.0f);
	SetShaderTexture("ceramic");
	SetShaderMaterial("glass");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawBoxMesh();

	//Raised toilet seat
	scaleXYZ = glm::vec3(0.5f, 1.0f, 0.2f);

	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;

	positionXYZ = glm::vec3(5.9f, 3.0f, 8.0f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(1.0f, 0.0f, 1.0f, 1.0f);
	SetShaderTexture("ceramic");
	SetShaderMaterial("glass");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawTorusMesh();
	///toilet////

	//Adds sink shapes

	//Main body of sink
	scaleXYZ = glm::vec3(2.0f, 4.0f, 3.0f);

	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	positionXYZ = glm::vec3(-5.9f, 2.0f, 8.0f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(1.0f, 0.0f, 1.0f, 1.0f);
	SetShaderTexture("wood");
	SetShaderMaterial("wood");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawBoxMesh();

	//Front overhang-y part of sink
	scaleXYZ = glm::vec3(0.4f, 3.7f, 3.0f);

	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	positionXYZ = glm::vec3(-4.7f, 2.2f, 8.0f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(1.0f, 0.0f, 1.0f, 1.0f);
	SetShaderTexture("wood");
	SetShaderMaterial("wood");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawBoxMesh();

	//Top Surface of sink
	scaleXYZ = glm::vec3(2.41f, 0.2f, 3.05f);

	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	positionXYZ = glm::vec3(-5.7f, 4.0f, 8.0f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(1.0f, 0.0f, 1.0f, 1.0f);
	SetShaderTexture("whiteplastic");
	SetShaderMaterial("plastic");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawBoxMesh();

	//Faucet base
	scaleXYZ = glm::vec3(0.1f, 0.4f, 0.1f);

	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	positionXYZ = glm::vec3(-6.85f, 4.0f, 8.0f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(1.0f, 0.0f, 1.0f, 1.0f);
	SetShaderTexture("stainless");
	SetShaderMaterial("glass");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawCylinderMesh();

	//Faucet end
	scaleXYZ = glm::vec3(0.1f, 0.4f, 0.1f);

	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 60.0f;

	positionXYZ = glm::vec3(-6.55f, 4.15f, 8.0f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(1.0f, 0.0f, 1.0f, 1.0f);
	SetShaderTexture("stainless");
	SetShaderMaterial("glass");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawCylinderMesh();
	///Sink///

	//Add Plant shapes
	// 
	//Plant pot
	scaleXYZ = glm::vec3(0.7f, 1.4f, 0.7f);

	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	positionXYZ = glm::vec3(5.4f, 0.0f, 5.9f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(1.0f, 0.0f, 1.0f, 1.0f);
	SetShaderTexture("blackplastic");
	SetShaderMaterial("plastic");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawCylinderMesh();

	//Dirt in plantpot
	scaleXYZ = glm::vec3(0.65f, 0.1f, 0.65f);

	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	positionXYZ = glm::vec3(5.4f, 1.31f, 5.9f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(1.0f, 0.0f, 1.0f, 1.0f);
	SetShaderTexture("dirt");
	SetShaderMaterial("clay");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawCylinderMesh();

	//PLANT
	scaleXYZ = glm::vec3(0.05f, 2.0f, 0.05f);

	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	positionXYZ = glm::vec3(5.4f, 1.5f, 5.9f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(1.0f, 0.0f, 1.0f, 1.0f);
	SetShaderTexture("wood");
	SetShaderMaterial("wood");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawCylinderMesh();
}
