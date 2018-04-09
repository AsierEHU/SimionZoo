#pragma once
#include <vector>
#include "../WindowsUtils/Timer.h"

class GraphicObject3D;
class GraphicObject2D;
class TextureManager;
class Binding;
class Bindable;
class Camera;
class Light;
class BoundingBox3D;
class BoundingBox2D;
using namespace std;
namespace tinyxml2 { class XMLElement; }

class Renderer
{
	Timer m_timer;
	int m_frameCount = 0;
	double m_fps = 0.0;
	unsigned int m_num3DObjectsDrawn = 0;

	TextureManager* m_pTextureManager;

	Camera* m_pActiveCamera;
	vector <Camera*> m_cameras;

	vector <Light*> m_lights;

	string m_dataFolder;

	static Renderer* m_pInstance;

	vector<GraphicObject3D*> m_3DgraphicObjects;
	vector<GraphicObject2D*> m_2DgraphicObjects;

	vector <Binding*> m_bindings;

	int m_windowWidth= 1024, m_windowHeight= 768;

	void reshapeWindow(int w, int h);
	void drawScene();
	static void _drawScene();
	static void _reshapeWindow(int width, int height);
	void loadSceneObjects(tinyxml2::XMLElement* pNode);
	Binding* getBinding(string externalName);

	void drawBoundingBox3D(BoundingBox3D& box) const;
	void drawBoundingBox2D(BoundingBox2D& box) const;

	bool m_bShowBoundingBoxes= false;
public:
	Renderer();
	virtual ~Renderer();

	void getWindowsSize(int& sizeX, int& sizeY) { sizeX = m_windowWidth; sizeY = m_windowHeight; }
	double getFPS() { return m_fps; }
	void updateFPS();
	unsigned int getNum3DObjectsDrawn() { return m_num3DObjectsDrawn; }

	void setDataFolder(string dataFolder);
	string getDataFolder();

	void loadScene(const char* file);

	void redraw();

	void init(int argc, char** argv, int sizeX, int sizeY);

	void addGraphicObject(GraphicObject3D* pObj);
	void add2DGraphicObject(GraphicObject2D* pObj);

	static Renderer* get();
	TextureManager* getTextureManager();
	Camera* getActiveCamera();

	GraphicObject3D* get3DObjectByName(string name);
	GraphicObject2D* get2DObjectByName(string name);

	void showBoundingBoxes(bool bShow) { m_bShowBoundingBoxes = bShow; }
	bool bShowBoundingBoxes() { return m_bShowBoundingBoxes; }

	//this is called by Bindable objects at initialization time
	template <typename T>
	void registerBinding(string externalName, T& obj, string internalName, double offset = 0.0)
	{
		Binding* pBinding = getBinding(externalName);
		if (pBinding == nullptr)
		{
			//No binding registered yet for the external name (i.e, the state variable's name)
			//For now, only the first bound object is allowed to set the offset
			pBinding = new Binding(externalName, obj, internalName, offset);
		}
		else
		{
			//we simply add the new bound object
			pBinding->addBoundObject(new BoundObject<T>(obj, internalName));
		}
		m_bindings.push_back(pBinding);
	}

	//These methods should be called from outside the renderer
	size_t getNumBindings();
	string getBindingExternalName(unsigned int i);
	bool updateBinding(unsigned int i, double value);
	bool updateBinding(string bindingExternalName, double value);
};

